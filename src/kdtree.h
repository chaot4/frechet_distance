#pragma once

#include "defs.h"

#include <algorithm>
#include <array>
#include <functional>
#include <queue>
#include <type_traits>
#include <vector>

template <typename T, int k, typename V, typename D = T>
class KdTree
{
	static_assert(k > 0, "Template parameter k should be > 0.");
	static_assert(std::is_pod<V>::value, "The value parameter should be a POD type.");

public:
	using Point = std::array<T, k>;
	using Value = V;
	using Values = std::vector<Value>;
	using Distance = D;
	using NearChecker = std::function<bool(Point const&, Point const&, Distance distance)>;

	KdTree(NearChecker const& near_checker)
		: is_near(near_checker) {}

	void add(Point const& point, Value value);
	void build();
	void clear();

	// Fills the variable 'result' by all the points in the kdtree
	// which are <= 'distance' away from 'point'
	void search(Point const& point, Distance distance, Values& result) const;

protected:
	bool is_ready_for_search = false;
	NearChecker is_near;

	// types: empty = -1, split = 0..k-1, leaf = k
	// The split value gives the dimension which is used for the split
	using Type = int;
	using KdID = std::size_t;
	struct KdNode
	{
		KdID id = std::numeric_limits<KdID>::max();
		Type type = -1;
		Point point;
		Value value;
#ifdef CERTIFY
		//maintain deletion information for orthogonal range search
		enum class DeleteState { Present, MarkedDeleted, Deleted };
		DeleteState delete_flag = DeleteState::Present;
		bool is_deleted() { return delete_flag == DeleteState::Deleted; }
		bool holds_value() { return delete_flag == DeleteState::Present; }
#endif

		KdNode() = default;
		KdNode(Point const& point, Value value)
			: point(point), value(value) {}

		bool is_empty() const { return type == -1; }
		bool is_inner() const { return type >= 0 && type < k; }
		bool is_leaf() const { return type == k; }
	};
	using Tree = std::vector<KdNode>;

	Tree tree;

	// helper structs for the building process
	using TreeIterator = typename Tree::iterator;

	struct BuildElement
	{
		KdID id;
		TreeIterator begin;
		TreeIterator end;

		// BuildElement() = default;
		BuildElement(KdID id, TreeIterator begin, TreeIterator end)
			: id(id), begin(begin), end(end) {}
	};

	struct Comp
	{
		Comp(int dimension) : dimension(dimension) {}

		bool operator()(KdNode const& node1, KdNode const& node2) const
		{
			return node1.point[dimension] < node2.point[dimension];
		};

	private:
		int dimension;
	};

	int calcSplitDimension(TreeIterator begin, TreeIterator end) const;
};

	template <typename T, int k, typename V, typename D>
void KdTree<T, k, V, D>::add(Point const& point, Value value)
{
	tree.emplace_back(point, value);
	is_ready_for_search = false;
}

	template <typename T, int k, typename V, typename D>
void KdTree<T, k, V, D>::build()
{
	if (tree.empty()) { return; }

	std::vector<BuildElement> build_stack;
	build_stack.emplace_back(0, tree.begin(), tree.end());

	// Give the points the correct IDs (which correspond to their
	// final position in the tree vector)
	while (!build_stack.empty()) {
		auto current = build_stack.back();
		build_stack.pop_back();

		// If only a single element remains in the range
		if (std::distance(current.begin, current.end) == 1) {
			current.begin->id = current.id;
			current.begin->type = k;
			continue;
		}

		// Find median
		auto median = current.begin + std::distance(current.begin, current.end)/2;
		auto split_dimension = calcSplitDimension(current.begin, current.end);
		std::nth_element(current.begin, median, current.end, Comp(split_dimension));

		// Settle the median element
		median->id = current.id;
		median->type = split_dimension;

		// Find correct IDs of remainng elments (and check for empty range before push)
		if (current.begin < median) {
			build_stack.emplace_back(2*current.id + 1, current.begin, median);
		}
		if (median + 1 < current.end) {
			build_stack.emplace_back(2*current.id + 2, median + 1, current.end);
		}
	}

	// Build tree using the IDs
	// There are 3 steps to this:
	// 1) sort by ID (because that's the order in which the points will appear in the tree vector)
	// 2) resize to largest id (as the tree vector has some gaps)
	// 3) actually place the points at their correct id (starting from the back of the vector)

	// 1)
	auto sort_by_id = [](KdNode const& node1, KdNode const& node2) {
		assert(node1.id != node2.id);
		return node1.id < node2.id;
	};
	std::sort(tree.begin(), tree.end(), sort_by_id);
	// 2)
	auto number_of_points = tree.size();
	tree.resize(tree.back().id + 1);
	// 3)
	for(int i = number_of_points - 1; i >= 0; --i) {
		std::swap(tree[i], tree[tree[i].id]);
	}

	is_ready_for_search = true;
}

template <typename T, int k, typename V, typename D>
int KdTree<T, k, V, D>::calcSplitDimension(TreeIterator begin, TreeIterator end) const
{
	Point min;
	Point max;
	min.fill(std::numeric_limits<T>::max());
	max.fill(std::numeric_limits<T>::lowest());

	// find min and max
	auto find_min_max = [&min, &max](KdNode const& node) {
		for (std::size_t i = 0; i < k; ++i) {
			min[i] = std::min(min[i], node.point[i]);
			max[i] = std::max(max[i], node.point[i]);
		}
	};
	std::for_each(begin, end, find_min_max);

	// find dimension with largest difference
	T max_difference = -1;
	int max_dimension = -1;
	for (int i = 0; i < k; ++i) {
		if (max[i] - min[i] > max_difference) {
			max_difference = max[i] - min[i];
			max_dimension = i;
		}
	}

	assert(max_dimension != -1 && max_difference >= 0);
	return max_dimension;
}

	template <typename T, int k, typename V, typename D>
void KdTree<T, k, V, D>::clear()
{
	tree.clear();
	is_ready_for_search = false;
}

template <typename T, int k, typename V, typename D>
void KdTree<T, k, V, D>::search(Point const& query_point, Distance distance, Values& result) const
{
	assert(is_ready_for_search);

	std::queue<KdID> search_queue;
	search_queue.push(0);

	while (!search_queue.empty()) {
		auto current_index = search_queue.front();
		auto const& kd_point = tree[current_index];
		search_queue.pop();
		if (kd_point.is_empty()) { continue; }

		// Check if this point is in range
		if (is_near(kd_point.point, query_point, distance)) {
			result.push_back(kd_point.value);
		}
		if (kd_point.is_leaf()) { continue; }

		// Search in subtrees
		assert(kd_point.is_inner());

		auto dimension = kd_point.type;
		auto query_coord = query_point[dimension];
		auto split_coord = kd_point.point[dimension];
		// first child
		if (query_coord - distance <= split_coord) {
			assert(2*current_index + 1 < tree.size());
			search_queue.push(2*current_index + 1);
		}
		// second child (if it exists -- therefore we also have to check)
		if (2*current_index + 2 < tree.size() && query_coord + distance >= split_coord) {
			search_queue.push(2*current_index + 2);
		}
	}
}
