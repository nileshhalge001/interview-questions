/* You have a list of N elements: a1, a2, a3, ..., aN
 *
 * Along with the elements, you are given some information about their relative order. To be
 * more specific, you are given a set of predicates where each predicate is a pair (ai, aj).
 * The pair (ai, aj) means that element ai is known to be greater than the element aj.
 *
 * You are to find whether it is possible to unequivocally sort this list. Your algorithm should
 * output a Yes/No answer. If there isn't enough information to fully sort the list, the algorithm
 * must return No as an answer. In other words, only return Yes if the given predicates lead to a
 * unique sort order.
 *
 * EXAMPLES
 *
 * Consider the list [ 2, 1, 3 ] and the predicates:
 *
 * 3 > 1
 * 2 > 1
 * 3 > 2
 *
 * Output: Yes
 *
 * Consider the list [ 2, 1, 4, 3 ] and the predicates:
 *
 * 4 > 3
 * 2 > 1
 * 3 > 1
 *
 * Output: No
 * Explanation: we know that 1 is the smallest element, so it would come first, but the relation
 * between 2 and the elements 4 and 3 is ambiguous. This set of predicates implies that we could
 * the elements in more than one way and still satisfy all the conditions, so the algorithm
 * outputs No. On the other hand, if we knew that 3 > 2, then the algorithm would output Yes
 * because there would be no more ambiguity and only one sort would be possible.
 *
 * Source: Careercup (Adobe interview)
 */
#include <cassert>
#include <map>
#include <iostream>

using namespace std;

struct graph_node {
	int id;
	bool visited;
	bool on_stack;
	map<int, graph_node*> children;
	graph_node(int key) : id(key), visited(false), on_stack(false) { }
};

class Graph {
	map<int, graph_node*> nodes;
	graph_node *topsort_last;

	bool cycle_detect(graph_node *node) {
		if (node->on_stack) {
			return false;
		}

		if (node->visited) {
			return true;
		}

		node->visited = true;
		node->on_stack = true;
		for (map<int, graph_node*>::iterator it = node->children.begin();
		     it != node->children.end();
		     it++) {
			if (!cycle_detect(it->second)) {
				return false;
			}
		}
		node->on_stack = false;

		return true;
	}

	bool no_cycles(void) {
		for (map<int, graph_node*>::iterator node_it = nodes.begin();
		     node_it != nodes.end();
		     node_it++) {
			node_it->second->visited = false;
			node_it->second->on_stack = false;
		}
		for (map<int, graph_node*>::iterator node_it = nodes.begin();
		     node_it != nodes.end();
		     node_it++) {
			if (!cycle_detect(node_it->second)) {
				return false;
			}
		}

		return true;
	}

	bool topsort_internal(graph_node *node) {
		if (node->visited) {
			return true;
		}

		node->visited = true;

		for (map<int, graph_node *>::iterator it = node->children.begin();
		     it != node->children.end();
		     it++) {
			if (!topsort_internal(it->second)) {
				return false;
			}
		}

		if (topsort_last != NULL && node->children.find(topsort_last->id) == node->children.end()) {
			return false;
		}

		topsort_last = node;
		return true;
	}

	bool unique_topsort(void) {
		topsort_last = NULL;

		for (map<int, graph_node*>::iterator node_it = nodes.begin();
		     node_it != nodes.end();
		     node_it++) {
			node_it->second->visited = false;
		}

		for (map<int, graph_node*>::iterator node_it = nodes.begin();
		     node_it != nodes.end();
		     node_it++) {
			if (!topsort_internal(node_it->second)) {
				return false;
			}
		}

		return true;
	}

public:

	void add_node(int id) {
		nodes[id] = new graph_node(id);
	}

	void add_edge(int from, int to) {
		graph_node *from_node = nodes[from];
		graph_node *to_node = nodes[to];

		assert(from_node != NULL);
		assert(to_node != NULL);

		from_node->children[to] = to_node;
	}

	bool is_sortable(void) {
		return no_cycles() && unique_topsort();
	}

	~Graph(void) {
		for (map<int, graph_node*>::iterator it = nodes.begin();
		     it != nodes.end();
		     it++) {
			delete it->second;
		}
	}
};

int main(void) {
	cout << "Enter size of list, followed by each element. Then, enter the number of relations" << endl;
	cout << "that are known, followed by each relation. A relation is a pair or whitespace separated" << endl;
	cout << "IDs where the left ID is greater than the right ID." << endl;
	cout << "> ";

	unsigned entries;
	while (cin >> entries) {
		Graph graph;

		for (unsigned i = 0; i < entries; i++) {
			int id;
			cin >> id;
			graph.add_node(id);
		}

		unsigned relations;
		cin >> relations;

		for (unsigned i = 0; i < relations; i++) {
			int left, right;
			cin >> left >> right;
			graph.add_edge(left, right);
		}

		cout << (graph.is_sortable() ? "Yes" : "No") << endl;
		cout << "> ";
	}

	return 0;
}
