#pragma once

#include "geometry_basics.h"

#include <queue>

using CIntervalIDs = std::vector<CIntervalID>;

class OrthRangeSearch
{
public: 
	static const int k = 2;

	using Point = std::array<CPoint, k>;
	using Value = CIntervalID;
	using Values = CIntervalIDs;

	void add(Point const& point, Value value);
	void build();
	void clear();


	//returns and deletes(!) all points (x,y) to the lower right of corner (cx,cy), i.e.,
	//where x >= cx and y <= cy	
	void reportAndDeleteToLowerRight(const Point& corner, Values& result);
	void reportAndDeleteOR(const Point& topleft, const Point& bottomright, Values& result);

	OrthRangeSearch() = default; 

	bool everythingfine();

private:
	bool is_ready_for_search = false;

	// types: empty = -1, split = 0..k-1, leaf = k
	// The split value gives the dimension which is used for the split
	using Type = int;
	struct KdNode
	{
		ID<KdNode> id;
		Type type = -1;
		Point point;
		Value value;

		enum class DeleteState { Present, MarkedDeleted, Deleted };
		DeleteState delete_flag = DeleteState::Present;
		bool is_deleted() { return delete_flag == DeleteState::Deleted; }
		bool holds_value() { return delete_flag == DeleteState::Present; }

		KdNode() = default;
		KdNode(Point const& point, Value value)
			: point(point), value(value) {}

		bool is_empty() const { return type == -1; }
		bool is_inner() const { return type >= 0 && type < k; }
		bool is_leaf() const { return type == k; }
	};
	using KdID = ID<KdNode>;
	using Tree = std::vector<KdNode>;

	Tree tree;
	std::queue<KdID> search_queue;

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
	void deletePoint(KdID kd_id);
};
