#pragma once

//
// 2D static range tree
//

#include <algorithm>
#include <cmath>
#include <vector>

#include "defs.h"
#include "id.h"

namespace unit_tests { void testRangeTree(); }

template <typename T, typename V>
class RangeTree
{
	// static_assert(std::is_pod<T>::value, "The type parameter should be a POD type.");
	static_assert(std::is_pod<V>::value, "The value parameter should be a POD type.");

public:
	using Value = V;
	using Values = std::vector<Value>;
	struct Point { T x, y; };
	using Points = std::vector<Point>;
	struct Rect { T x_min, x_max, y_min, y_max; };

	void add(Point const& p, Value v);
	void build();
	void clear();

	void searchAndDelete(Point const& query, Values& result);

private:
	//
	// data
	//

	bool is_ready_for_search = false;

	struct PointValuePair;
	struct ValueEntry;
	struct Node;
	using PVPID = ID<PointValuePair>;
	using ValueEntryID = ID<ValueEntry>;
	using NodeID = ID<Node>;

	struct PointValuePair {
		Point point;
		Value value;

		PointValuePair(Point const& point, Value value)
			: point(point), value(value) {}
	};

	struct Node {
		T x;

		ValueEntryID begin;
		ValueEntryID end;
		bool is_leaf = false;

		Node() {} // id is invalid in this case

		bool is_empty() const { return !begin.valid() && !end.valid(); }
	};

	struct ValueEntry {
		T y;
		Value value;
		PVPID pvpid;

		ValueEntry(T y, Value value, PVPID pvpid) : y(y), value(value), pvpid(pvpid) {}

		bool operator<(ValueEntry const& other) const { return y < other.y; }
		bool operator<(T y) const { return this->y < y; }
		bool operator<=(T y) const { return this->y <= y; }
	};

	std::vector<PointValuePair> point_value_pairs;
	std::vector<Node> nodes;
	std::vector<ValueEntry> value_entries;
	std::vector<bool> deleted;

	//
	// helpers for building the tree
	//

	struct BuildElement {
		NodeID node_id;
		PVPID begin;
		PVPID end;

		BuildElement(NodeID node_id, PVPID begin, PVPID end)
			: node_id(node_id), begin(begin), end(end) {}
	};
	std::vector<BuildElement> todo;

	//
	// other helpers
	//

	static inline NodeID left(NodeID id) { return 2*id + 1; }
	static inline NodeID right(NodeID id) { return 2*id + 2; }
	static inline NodeID parent(NodeID id) { return (id-1)/2; }

	template <typename TT, typename VV>
		friend std::ostream& operator<<(std::ostream& os, RangeTree<TT, VV> const& tree);
};

	template <typename T, typename V>
void RangeTree<T,V>::add(Point const& point, Value value)
{
	point_value_pairs.emplace_back(point, value);
	is_ready_for_search = false;
}

	template <typename T, typename V>
void RangeTree<T,V>::build()
{
	if (point_value_pairs.empty()) { return; }

	deleted.assign(point_value_pairs.size(), false);

	nodes.reserve(point_value_pairs.size()*2);
	auto add_node = [this](NodeID node_id, ValueEntryID begin, ValueEntryID end) {
		if (node_id >= nodes.size()) {
			nodes.resize(node_id+1);
		}
		nodes[node_id].begin = begin;
		nodes[node_id].end = end;
	};

	// sort point_value_pairs by x (ascending)...
	auto compare_x = [](PointValuePair const& pvp1, PointValuePair const& pvp2) {
		return pvp1.point.x < pvp2.point.x;
	};
	std::sort(point_value_pairs.begin(), point_value_pairs.end(), compare_x);

	// set value entries for first node
	value_entries.reserve(point_value_pairs.size()*log2(point_value_pairs.size()));
	for (PVPID id = 0; id < point_value_pairs.size(); ++id) {
		auto const& pvp = point_value_pairs[id];
		value_entries.emplace_back(pvp.point.y, pvp.value, id);
	}
	std::sort(value_entries.begin(), value_entries.end());
	add_node(0, 0, value_entries.size());

	// now build the tree recursively
	todo.emplace_back(0, 0, point_value_pairs.size());

	while (!todo.empty()) {
		// get top
		NodeID node_id = todo.back().node_id;
		PVPID pvp_begin = todo.back().begin;
		PVPID pvp_end = todo.back().end;
		todo.pop_back();

		// set x value of node
		PVPID pvp_middle = pvp_begin + PVPID((pvp_end - pvp_begin + 1)/2);
		auto& node = nodes[node_id];
		node.x = point_value_pairs[pvp_middle-1].point.x;
		// if it is a leaf, we're done.
		if (pvp_end - pvp_begin == 1) { node.is_leaf = true; continue; }

		// push left child to todo and add value_entries incl. links
		auto value_entries_begin_left = value_entries.size();
		for (auto value_entry_id = node.begin; value_entry_id < node.end; ++value_entry_id) {
			auto const& value_entry = value_entries[value_entry_id];
			if (value_entry.pvpid < pvp_middle) {
				value_entries.emplace_back(value_entry.y, value_entry.value, value_entry.pvpid);
			}
		}
		auto value_entries_end_left = value_entries.size();
		add_node(left(node_id), value_entries_begin_left, value_entries_end_left);
		// add x value to leaves and push non-leaves to todo
		if (pvp_middle - pvp_begin == 1) {
			nodes[left(node_id)].x = point_value_pairs[pvp_begin].point.x;
			nodes[left(node_id)].is_leaf = true;
		}
		else {
			todo.emplace_back(left(node_id), pvp_begin, pvp_middle);
		}

		// same as above for right
		auto value_entries_begin_right = value_entries.size();
		for (auto value_entry_id = node.begin; value_entry_id < node.end; ++value_entry_id) {
			auto const& value_entry = value_entries[value_entry_id];
			if (value_entry.pvpid >= pvp_middle) {
				value_entries.emplace_back(value_entry.y, value_entry.value, value_entry.pvpid);
			}
		}
		auto value_entries_end_right = value_entries.size();
		add_node(right(node_id), value_entries_begin_right, value_entries_end_right);
		// add x value to leaves and push non-leaves to todo
		if (pvp_end - pvp_middle == 1) {
			nodes[right(node_id)].x = point_value_pairs[pvp_middle].point.x;
			nodes[right(node_id)].is_leaf = true;
		}
		else {
			todo.emplace_back(right(node_id), pvp_middle, pvp_end);
		}
	}

	is_ready_for_search = true;
}

	template <typename T, typename V>
void RangeTree<T,V>::clear()
{
	value_entries.clear();
	nodes.clear();
	point_value_pairs.clear();
	deleted.clear();
	todo.clear();

	is_ready_for_search = false;
}

	template <typename T, typename V>
void RangeTree<T,V>::searchAndDelete(Point const& query, Values& result)
{
	assert(is_ready_for_search);

	// Search along left boundary
	NodeID current_id = 0;
	while (current_id < nodes.size()) {
		auto& current = nodes[current_id];

		if (current.is_empty() || current.begin >= current.end) { return; }

		if (current.is_leaf) {
			auto const& value_entry = value_entries[current.begin];
			if (current.x >= query.x && value_entry.y <= query.y) {
				if (!deleted[value_entry.pvpid]) {
					deleted[value_entry.pvpid] = true;
					result.push_back(value_entry.value);
				}
				++current.begin;
			}
			return;
		}

		if (query.x <= current.x) { // go left and return right child
			if (right(current_id) < nodes.size()) {
				auto& right_node = nodes[right(current_id)];
				if (!right_node.is_empty()) {
					while (right_node.begin < right_node.end &&
						   value_entries[right_node.begin].y <= query.y) {
						auto const& value_entry = value_entries[right_node.begin];
						if (!deleted[value_entry.pvpid]) {
							deleted[value_entry.pvpid] = true;
							result.push_back(value_entry.value);
						}
						++right_node.begin;
					}
				}
			}

			current_id = left(current_id);
		}
		else { // go right
			current_id = right(current_id);
		}
	}
}

	template <typename T, typename V>
std::ostream& operator<<(std::ostream& os, RangeTree<T, V> const& tree)
{
	os << "number of nodes: " << tree.nodes.size() << "\n";
	os << "value entries size: " << tree.value_entries.size() << "\n";
	os << "\n";

	for (std::size_t node_id = 0; node_id < tree.nodes.size(); ++node_id) {
		auto const& node = tree.nodes[node_id];
		if (node.is_empty()) { continue; }

		os << "Node: " << node_id << " " << node.x << " " << node.begin << " " << node.end << "\n";

		auto begin = tree.value_entries.begin() + node.begin;
		auto end = tree.value_entries.begin() + node.end;
		os << "Value Entries:\n";
		for (auto it = begin; it != end; ++it) {
			os << it->y << " " << it->value << " " << it->pvpid << "\n";
		}
		os << "\n";
	}

	for (std::size_t id = 0; id < tree.value_entries.size(); ++id) {
		auto const value_entry = tree.value_entries[id];
		os << id << " " << value_entry.y << " " << value_entry.value
			<< " " << value_entry.pvpid << "\n";
	}

	return os;
}
