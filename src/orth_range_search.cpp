#include "orth_range_search.h"

#include "times.h"

bool toLowerRight(const OrthRangeSearch::Point& p, const OrthRangeSearch::Point& corner)
{
	return p[0] >= corner[0] and p[1] <= corner[1];
}

bool OrthRangeSearch::everythingfine() {
	if (not is_ready_for_search) {
		return true;
	}
	for (size_t i = 1; i < tree.size(); i++) {
		if ((not tree[i].is_deleted()) and tree[(i-1)/2].is_deleted()) {
			return false;
		}
	}
	return true;
}

void OrthRangeSearch::deletePoint(KdID kd_id)
{
	while (kd_id.valid()) {
		auto& kd_point = tree[kd_id];
		kd_id.invalidate();

		assert(not kd_point.is_deleted());
		bool children_deleted = true;

		if (kd_point.is_inner()) {
			auto child1 = 2*kd_point.id + 1;
			assert(child1 < tree.size());
			if (not tree[child1].is_deleted()) {
				children_deleted = false;
			} else {
				auto child2 = child1+1;
				if (child2 < tree.size() and not tree[child2].is_deleted()) {
					children_deleted = false;
				}
			}
		}

		if (children_deleted) {
			kd_point.delete_flag = KdNode::DeleteState::Deleted;
			//propagate deletion to parent (if not root)
			if (kd_point.id != KdID(0)) {
				auto parent = (kd_point.id-1)/2 ;
				assert(not tree[parent].is_deleted());
				if (not tree[parent].holds_value()) {
					kd_id = tree[parent].id;
				}
			}
		} else {
			kd_point.delete_flag = KdNode::DeleteState::MarkedDeleted;
		}
	}
}

//Unbounded Queries

inline static bool recurse_smaller(int dimension, CPoint query_coord, CPoint split_coord) {
	if (dimension == 0) {
		return query_coord <= split_coord;
	} else {
		return true;
	}
}

inline static bool recurse_larger(int dimension, CPoint query_coord, CPoint split_coord) {
	if (dimension == 1) {
		return query_coord >= split_coord;
	} else {
		return true;
	}
}


void OrthRangeSearch::reportAndDeleteToLowerRight(const OrthRangeSearch::Point& corner, CIntervalIDs& result)
{
	assert(is_ready_for_search);

	search_queue = std::queue<KdID>();
	search_queue.push(0);

	while (!search_queue.empty()) {
		global::times.incrementCertOrthRangeNodeVisit();
		auto current_index = search_queue.front();
		auto& kd_point = tree[current_index];
		search_queue.pop();
		if (kd_point.is_empty()) { continue; }

		// Check if this point is in range
		if (kd_point.holds_value() and toLowerRight(kd_point.point, corner)) {
			result.push_back(kd_point.value);

			assert(not kd_point.is_deleted());
			deletePoint(kd_point.id);
		}
		if (kd_point.is_deleted()) { continue; }
		if (kd_point.is_leaf()) { continue; }

		// Search in subtrees
		assert(kd_point.is_inner());

		auto dimension = kd_point.type;
		auto query_coord = corner[dimension];
		auto split_coord = kd_point.point[dimension];
		// first child
		if (recurse_smaller(dimension, query_coord, split_coord)) {
			assert(2*current_index + 1 < tree.size());
			if (!tree[2*current_index + 1].is_deleted()) {
				search_queue.push(2*current_index + 1);
			}
		}
		// second child (if it exists -- therefore we also have to check)
		if (2*current_index + 2 < tree.size() && recurse_larger(dimension, query_coord, split_coord)) {
			if (!tree[2*current_index + 2].is_deleted()) {
				search_queue.push(2*current_index + 2);
			}
		}
	}
}

//Bounded Queries

inline static bool recurse_smaller_bounded(int dimension, CPoint query_topleft, CPoint query_bottomright, CPoint split_coord) {
	if (dimension == 0) {
		return query_topleft <= split_coord;
	} else {
		return query_bottomright <= split_coord;
	}
}

inline static bool recurse_larger_bounded(int dimension, CPoint query_topleft, CPoint query_bottomright, CPoint split_coord) {
	if (dimension == 0) {
		return query_bottomright >= split_coord;
	} else {
		return query_topleft >= split_coord;
	}
}
inline static bool inRange(const OrthRangeSearch::Point& pt, const OrthRangeSearch::Point& topleft, const OrthRangeSearch::Point& bottomright) {
	return topleft[0] <= pt[0] and pt[0] <= bottomright[0] and topleft[1] >= pt[1] and pt[1] >= bottomright[1];
}


void OrthRangeSearch::reportAndDeleteOR(const OrthRangeSearch::Point& topleft, const OrthRangeSearch::Point& bottomright, CIntervalIDs& result)
{
	assert(is_ready_for_search);

	search_queue = std::queue<KdID>();
	search_queue.push(0);

	while (!search_queue.empty()) {
		global::times.incrementCertOrthRangeNodeVisit();
		auto current_index = search_queue.front();
		auto& kd_point = tree[current_index];
		search_queue.pop();
		if (kd_point.is_empty()) { continue; }

		// Check if this point is in range
		if (kd_point.holds_value() and inRange(kd_point.point, topleft, bottomright)) {
			result.push_back(kd_point.value);

			assert(not kd_point.is_deleted());
			deletePoint(kd_point.id);
		}
		if (kd_point.is_deleted()) { continue; }
		if (kd_point.is_leaf()) { continue; }

		// Search in subtrees
		assert(kd_point.is_inner());

		auto dimension = kd_point.type;
		auto query_topleft = topleft[dimension];
		auto query_bottomright = bottomright[dimension];
		auto split_coord = kd_point.point[dimension];
		// first child
		if (recurse_smaller_bounded(dimension, query_topleft, query_bottomright, split_coord)) {
			assert(2*current_index + 1 < tree.size());
			if (!tree[2*current_index + 1].is_deleted()) {
				search_queue.push(2*current_index + 1);
			}
		}
		// second child (if it exists -- therefore we also have to check)
		if (2*current_index + 2 < tree.size() && recurse_larger_bounded(dimension, query_topleft, query_bottomright, split_coord)) {
			if (!tree[2*current_index + 2].is_deleted()) {
				search_queue.push(2*current_index + 2);
			}
		}
	}
}

void OrthRangeSearch::add(Point const& point, Value value)
{
	tree.emplace_back(point, value);
	is_ready_for_search = false;
}

void OrthRangeSearch::build()
{
	if (tree.empty()) {
		Point dummy = {{ CPoint(0,0.), CPoint(0,0.) }};
		tree.emplace_back(dummy, 0);
		tree[0].delete_flag = KdNode::DeleteState::Deleted;
		is_ready_for_search = true;
		return;
	}

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
	global::times.recordOrthRangeTreeSize(tree.size());
}

int OrthRangeSearch::calcSplitDimension(TreeIterator begin, TreeIterator end) const
{
	std::array<distance_t, 2> min;
	std::array<distance_t, 2> max;
	min.fill(std::numeric_limits<distance_t>::max());
	max.fill(std::numeric_limits<distance_t>::lowest());

	// find min and max
	auto find_min_max = [&min, &max](KdNode const& node) {
		for (std::size_t i = 0; i < k; ++i) {
			min[i] = std::min(min[i], node.point[i].convert());
			max[i] = std::max(max[i], node.point[i].convert());
		}
	};
	std::for_each(begin, end, find_min_max);

	// find dimension with largest difference
	distance_t max_difference = -1;
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

void OrthRangeSearch::clear()
{
	tree.clear();
	is_ready_for_search = false;
}
