#pragma once

#include "geometry_basics.h"
#include "times.h"

#include <algorithm>

namespace unit_tests { void testPrioritySearchTree(); }

template <typename T, typename V>
class PrioritySearchTree
{
public:
	struct Point { T x, y; };
	using Points = std::vector<Point>;
	using Value = V;
	using Values = std::vector<Value>;

	void add(Point const& point, Value value);
	void build();
	void clear();

	PrioritySearchTree() = default; 

	// report and delete everything to the lower right of "corner"
	void searchAndDelete(const Point& corner, Values& result);

	// checks consistency of tree
	bool everythingfine();

private:
	bool is_ready_for_search = false;

	struct Node;
	using NodeID = ID<Node>;
	using NodeIDs = std::vector<NodeID>;

	struct Node
	{
		ID<Node> id;
		Point point;
		Value value;

		T x_split;

		Node() = default;
		Node(Point const& point, Value value)
			: point(point), value(value) {}

		bool is_empty() const { return !id.valid(); }
	};
	using Nodes = std::vector<Node>;

	Nodes nodes;

	// helper structs for the building process
	using NodeIterator = typename Nodes::iterator;

	struct BuildElement
	{
		NodeID id;
		NodeIterator begin;
		NodeIterator end;

		// BuildElement() = default;
		BuildElement(NodeID id, NodeIterator begin, NodeIterator end)
			: id(id), begin(begin), end(end) {}
	};

	struct CompX
	{
		bool operator()(Node const& node1, Node const& node2) const
		{
			return node1.point.x < node2.point.x;
		};
	};

	struct CompY
	{
		bool operator()(Node const& node1, Node const& node2) const
		{
			return node1.point.y < node2.point.y;
		};
	};

	// helper functions
	bool toLowerRight(const Point& p, const Point& corner) const;

	static inline NodeID left(NodeID id) { return 2*id + 1; }
	static inline NodeID right(NodeID id) { return 2*id + 2; }
	static inline NodeID parent(NodeID id) { return (id-1)/2; }

	template <typename TT, typename VV>
		friend std::ostream& operator<<(std::ostream& os, PrioritySearchTree<TT, VV> const& tree);

	// search data structures
	NodeIDs search_stack;
	NodeIDs roots;
	NodeIDs to_delete;

	void deleteNodes();
	void rotate(NodeID parent_id, NodeID child_id);
};

	template <typename T, typename V>
void PrioritySearchTree<T,V>::add(Point const& point, Value value)
{
	nodes.emplace_back(point, value);
	is_ready_for_search = false;
}

	template <typename T, typename V>
void PrioritySearchTree<T,V>::build()
{
	if (nodes.empty()) {
		is_ready_for_search = true;
		return;
	}

	std::vector<BuildElement> build_stack;
	build_stack.emplace_back(0, nodes.begin(), nodes.end());

	// give the points the correct IDs (which correspond to their
	// final position in the tree vector)
	while (!build_stack.empty()) {
		auto current = build_stack.back();
		build_stack.pop_back();

		// if only a single element remains in the range
		if (std::distance(current.begin, current.end) == 1) {
			current.begin->id = current.id;
			continue;
		}

		// find element with min y value
		auto min_it = std::min_element(current.begin, current.end, CompY());
		std::swap(*current.begin, *min_it);

		// set min_it to first element (where the minimum now is) and
		// shrink range to all other elements
		min_it = current.begin;
		++(current.begin);

		// find median
		auto median = current.begin + std::distance(current.begin, current.end)/2;
		std::nth_element(current.begin, median, current.end, CompX());

		// settle min element
		min_it->id = current.id;
		min_it->x_split = median->point.x;

		// build subranges resulting from median computation (and check for empty range before push)
		if (current.begin < median) {
			build_stack.emplace_back(left(current.id), current.begin, median);
		}
		if (median < current.end) {
			build_stack.emplace_back(right(current.id), median, current.end);
		}
	}

	// Build tree using the IDs
	// There are 3 steps to this:
	// 1) sort by ID (because that's the order in which the points will appear in the tree vector)
	// 2) resize to largest id (as the tree vector has some gaps)
	// 3) actually place the points at their correct id (starting from the back of the vector)

	// 1)
	auto sort_by_id = [](Node const& node1, Node const& node2) {
		assert(node1.id != node2.id);
		return node1.id < node2.id;
	};
	std::sort(nodes.begin(), nodes.end(), sort_by_id);
	// 2)
	auto number_of_points = nodes.size();
	nodes.resize(nodes.back().id + 1);
	// 3)
	for(int i = number_of_points - 1; i >= 0; --i) {
		std::swap(nodes[i], nodes[nodes[i].id]);
	}

	is_ready_for_search = true;
	global::times.recordOrthRangeTreeSize(nodes.size());
}

	template <typename T, typename V>
void PrioritySearchTree<T,V>::clear()
{
	nodes.clear();
	is_ready_for_search = false;
}

	template <typename T, typename V>
void PrioritySearchTree<T,V>::searchAndDelete(const Point& query, Values& result)
{
	assert(is_ready_for_search);

	if (nodes.empty()) { return; }
	roots.clear();
	search_stack.clear();
	to_delete.clear();

	NodeID current_id = 0;

	// search along the lower x path and collect all the inner nodes
	// already reporting the nodes on the path if they fit
	while (current_id < nodes.size()) {
		auto const& current = nodes[current_id];

		if (current.is_empty()) { break; }
		if (toLowerRight(current.point, query)) {
			result.push_back(current.value);
			to_delete.push_back(current_id);
		}

		if (query.x > current.x_split) {
			current_id = right(current_id);
		}
		else {
			roots.push_back(right(current_id));
			current_id = left(current_id);
		}
	}

	// now search roots for min elements
	for (auto root: roots) {

		search_stack.push_back(root);

		while (!search_stack.empty()) {
			if (search_stack.back() >= nodes.size()) {
				search_stack.pop_back();
				continue;
			}

			auto current_id = search_stack.back();
			auto const& current = nodes[current_id];
			search_stack.pop_back();

			if (current.is_empty()) { continue; }

			if (current.point.y <= query.y) {
				result.push_back(current.value);
				to_delete.push_back(current_id);

				search_stack.push_back(left(current_id));
				search_stack.push_back(right(current_id));
			}
		}
	}

	deleteNodes();
}

	template <typename T, typename V>
void PrioritySearchTree<T,V>::deleteNodes()
{
	while (!to_delete.empty()) {
		auto current_id = to_delete.back();
		to_delete.pop_back();

		nodes[current_id].id.invalidate();

		bool done = false;
		while (!done) {
			auto left_id = left(current_id);
			auto right_id = right(current_id);

			bool has_left = left_id < nodes.size() && !nodes[left_id].is_empty();
			bool has_right = right_id < nodes.size() && !nodes[right_id].is_empty();

			if (!has_left && !has_right) {
				done = true;
			}
			else if (has_left && has_right) {
				if (nodes[left_id].point.y < nodes[right_id].point.y) {
					rotate(current_id, left_id);
					current_id = left_id;
				}
				else {
					rotate(current_id, right_id);
					current_id = right_id;
				}
			}
			else if (has_left) {
				rotate(current_id, left_id);
				current_id = left_id;
			}
			else { // has_right
				rotate(current_id, right_id);
				current_id = right_id;
			}
		}
	}
}

	template <typename T, typename V>
void PrioritySearchTree<T,V>::rotate(NodeID parent_id, NodeID child_id)
{
	assert(parent_id < nodes.size());
	assert(child_id < nodes.size());
	assert(child_id >= 2*parent_id + 1 && child_id <= 2*parent_id + 2);

	nodes[parent_id].id = parent_id;
	nodes[parent_id].point = nodes[child_id].point;
	nodes[parent_id].value = nodes[child_id].value;
	nodes[child_id].id.invalidate();
}

template <typename T, typename V>
bool PrioritySearchTree<T,V>::toLowerRight(const Point& p, const Point& corner) const
{
	return p.x >= corner.x and p.y <= corner.y;
}

	template <typename T, typename V>
std::ostream& operator<<(std::ostream& os, PrioritySearchTree<T, V> const& tree)
{
	os << "number of nodes: " << tree.nodes.size() << "\n";
	os << "\n";

	for (std::size_t node_id = 0; node_id < tree.nodes.size(); ++node_id) {
		auto const& node = tree.nodes[node_id];
		if (node.is_empty()) { continue; }

		os << "Node: " << node_id << " " << node.point.x << " " << node.point.y
			<< " " << node.value << " " << node.x_split << "\n";
	}

	return os;
}
