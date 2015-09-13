/* TODO
 * - Add comments to explain error handling with realloc
 * - Rewrite top-level code to use only stack allocated variables and integers
 *   (rather than char buffers)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

struct channel {
	sem_t reader_to_writer;
	sem_t writer_to_reader;
	sem_t data_writer;
	sem_t data_reader;
	struct {
		char *data;
		size_t len;
		size_t cap;
	} data;
};

typedef struct channel channel_t;

channel_t *make_channel(void) {
	channel_t *ch = malloc(sizeof(*ch));
	if (ch == NULL)
		goto freem;
	if (sem_init(&ch->reader_to_writer, 0, 0) == -1)
		goto freem;
	if (sem_init(&ch->writer_to_reader, 0, 0) == -1)
		goto freer;
	if (sem_init(&ch->data_writer, 0, 1) == -1)
		goto freew;
	if (sem_init(&ch->data_reader, 0, 0) == -1)
		goto freedw;

	ch->data.data = NULL;
	ch->data.len = 0;
	ch->data.cap = 0;

	return ch;

freedw:
	sem_destroy(&ch->data_writer);
freew:
	sem_destroy(&ch->writer_to_reader);
freer:
	sem_destroy(&ch->reader_to_writer);
freem:
	free(ch);
	return NULL;
}

void destroy_channel(channel_t *ch) {
	sem_destroy(&ch->reader_to_writer);
	sem_destroy(&ch->writer_to_reader);
	free(ch);
}

int channel_write(channel_t *ch, char *data, size_t len) {

	if (len == 0) {
		errno = EINVAL;
		return -1;
	}

	if (sem_post(&ch->reader_to_writer) == -1)
		return -1;
	sem_wait(&ch->writer_to_reader);

	sem_wait(&ch->data_writer);
	if (ch->data.cap < len) {
		char *new_buf = realloc(ch->data.data, len*2);
		if (new_buf == NULL) {
			ch->data.len = 0;
			sem_post(&ch->data_reader);
			errno = ENOMEM;
			return -1;
		}
		ch->data.data = new_buf;
		ch->data.cap = len*2;
	}

	memcpy(ch->data.data, data, len);
	ch->data.len = len;
	sem_post(&ch->data_reader);

	return 0;
}

int channel_read(channel_t *ch, char *data, size_t len) {

	if (len == 0) {
		errno = EINVAL;
		return -1;
	}

	if (sem_post(&ch->writer_to_reader) == -1)
		return -1;
	sem_wait(&ch->reader_to_writer);

	sem_wait(&ch->data_reader);
	size_t copy_len = len;
	if (ch->data.len < copy_len)
		copy_len = ch->data.len;

	if (copy_len == 0) {
		sem_post(&ch->data_writer);
		errno = ENOMEM;
		return -1;
	}

	memcpy(data, ch->data.data, copy_len);
	sem_post(&ch->data_writer);

	return 0;
}

#define THREADS_N 10
#define COUNT 100

channel_t *chan;

static pthread_t threads[THREADS_N];

void *thr_fn(void *id) {
	int i;
	//int *buf = malloc(sizeof(*buf));
	char *buf = malloc(32);

	if (buf == NULL) {
		fprintf(stderr, "malloc(3) returned NULL\n");
		return NULL;
	}

	for (i = (uintptr_t) id; i < COUNT; i += THREADS_N) {
		int n = sprintf(buf, "%d", i);
		if (channel_write(chan, buf, n+1) == -1) {
			perror("channel_write() failed");
			//free(buf);
			return NULL;
		}
	}

	//free(buf);
	return NULL;
}

int main(void) {

	if ((chan = make_channel()) == NULL) {
		perror("make_channel() error");
		exit(EXIT_FAILURE);
	}

	size_t i;
	for (i = 0; i < THREADS_N; i++) {
		int thr_res = pthread_create(&threads[i], NULL, thr_fn, (void *) (uintptr_t) i);
		if (thr_res != 0) {
			fprintf(stderr, "pthread_create(3) error: %s\n", strerror(thr_res));
			exit(EXIT_FAILURE);
		}
	}

	for (i = 0; i < COUNT; i++) {
		//int j;
		char j[32];
		if (channel_read(chan, j, sizeof(j)) == -1) {
			perror("channel_read() error");
			exit(EXIT_FAILURE);
		}
		printf("%s\n", j);
	}

	return 0;
}
