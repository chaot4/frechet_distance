#pragma once

#include "geometry_basics.h"
#include "kdtree.h"
#include "curves.h"

#include <vector>
#include <iostream>


//
// Tree
//

using Tree = KdTree<distance_t, 8, CurveID>;

inline Tree::Point toKdPoint(Curve const& curve)
{
	auto const& extreme_points = curve.getExtremePoints();

	return {{
		curve.front().x,
		curve.front().y,
		curve.back().x,
		curve.back().y,
		extreme_points.min_x,
		extreme_points.min_y,
		extreme_points.max_x,
		extreme_points.max_y
	}};
}

//
// QueryElement
//

struct QueryElement
{
	Curve curve;
	distance_t distance;

	// This is rvalue ref only on purpose.
	QueryElement(Curve&& curve, distance_t distance)
		: curve(curve), distance(distance) {}
};
using QueryElements = std::vector<QueryElement>;

//
// Result(s)
//

struct Result
{
	CurveIDs curve_ids;

	void addCurve(CurveID curve_id)
	{
		curve_ids.push_back(curve_id);
	}
};
using Results = std::vector<Result>;

inline std::ostream& operator<<(std::ostream& out, const Result& result)
{
	for (auto curve_id: result.curve_ids) {
		out << curve_id << " ";
	}
	out << "\n";

    return out;
}
