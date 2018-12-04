#pragma once

#include "defs.h"
#include "geometry_basics.h"
#include "id.h"

// Represents a trajectory. Additionally to the points given in the input file,
// we also store the length of any prefix of the trajectory.
class Curve
{
public:
    Curve() = default;
    Curve(const Points& points);

    std::size_t size() const { return points.size(); }
	bool empty() const { return points.empty(); }
    Point const& operator[](PointID i) const { return points[i]; }
	Point interpolate_at(CPoint const& pt) const  {
		assert(pt.getFraction() >= 0. && pt.getFraction() <= 1.);
		assert((pt.getPoint() < points.size()-1 || (pt.getPoint() == points.size()-1 && pt.getFraction() == 0.)));
		return pt.getFraction() == 0. ? points[pt.getPoint()] : points[pt.getPoint()]*(1.-pt.getFraction()) + points[pt.getPoint()+1]*pt.getFraction();
	}
    distance_t curve_length(PointID i, PointID j) const
		{ return prefix_length[j] - prefix_length[i]; }

    Point front() const { return points.front(); }
    Point back() const { return points.back(); }

    void push_back(Point const& point);

	Points::const_iterator begin() { return points.begin(); }
	Points::const_iterator end() { return points.end(); }
	Points::const_iterator begin() const { return points.cbegin(); }
	Points::const_iterator end() const { return points.cend(); }
	
	std::string filename;

	struct ExtremePoints { distance_t min_x, min_y, max_x, max_y; };
	ExtremePoints const& getExtremePoints() const;
	distance_t getUpperBoundDistance(Curve const& other) const;

private:
    Points points;
    std::vector<distance_t> prefix_length;
	ExtremePoints extreme_points = {
		std::numeric_limits<distance_t>::max(), std::numeric_limits<distance_t>::max(),
		std::numeric_limits<distance_t>::lowest(), std::numeric_limits<distance_t>::lowest()
	};
};
using Curves = std::vector<Curve>;

std::ostream& operator<<(std::ostream& out, const Curve& curve);
