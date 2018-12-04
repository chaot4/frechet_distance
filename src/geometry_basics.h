#pragma once

#include "defs.h"
#include "id.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
#include <sstream>

namespace unit_tests { void testGeometricBasics(); }

//
// distance_t
//

using distance_t = double;

//
// Point
//

struct Point {
    distance_t x;
    distance_t y;

	Point& operator-=(const Point& point);
	Point operator-(const Point& point) const;
	Point& operator+=(const Point& point);
	Point operator+(const Point& point) const;
	Point operator*(const distance_t mult) const;
	Point& operator/=(distance_t distance);

	distance_t dist_sqr(const Point& point) const;
	distance_t dist(const Point& point) const;
};
using Points = std::vector<Point>;
using PointID = ID<Point>;

std::ostream& operator<<(std::ostream& out, const Point& p);

struct PointRange {
	PointID begin;
	PointID end;
};

struct ContinuousPoint {
	PointID point;
	distance_t fraction;

	bool operator<(ContinuousPoint const& other) const {
		return point < other.point || (point == other.point && fraction < other.fraction);
	}
	bool operator<(PointID point_id) const { return point < point_id; }
	bool operator>(PointID point_id) const { return point >= point_id; }

	bool valid() const { return point.valid(); }
	void invalidate() { point.invalidate(); }
};

struct FSCoordinate {
	PointID x;
	PointID y;
};

// Orientation and Direction etc.

enum class Direction : uint8_t {
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3
};
const std::array<Direction, 4> Directions = {{
	Direction::Up,
	Direction::Down,
	Direction::Left,
	Direction::Right
}};

enum class Orientation {
	Horizontal = 0,
	Vertical = 1
};
Orientation operator!(Orientation orientation);

// short for: backward-forward direction
enum class BFDirection {
	Backward = 0,
	Forward = 1
};
BFDirection operator!(BFDirection direction);

// some helper functions for Orientation and Direction
Orientation getOrientation(Direction direction);
Direction getForwardDirection(Orientation orientation);
Direction getBackwardDirection(Orientation orientation);
bool isForward(Direction direction);
bool isBackward(Direction direction);
std::array<Direction, 2> getDirections(Orientation orientation);
BFDirection toBFDirection(Direction direction);

//
// Interval
//

struct Interval
{
	distance_t begin;
	distance_t end;

	Interval()
		: begin(1.),
		  end(0.) {}

	Interval(distance_t begin, distance_t end)
		: begin(begin),
		  end(end) {}

	bool operator<(Interval const& other) const {
		return begin < other.begin || (begin == other.begin && end < other.end);
	}

	bool is_empty() const { return begin > end; }
	bool intersects(Interval const& other) const
	{
		if (is_empty() || other.is_empty()) { return false; }

		return (other.begin >= begin && other.begin <= end) ||
			(other.end >= begin && other.end <= end) ||
			(other.begin <= begin && other.end >= end);
	}
};
using Intervals = std::vector<Interval>;

std::ostream& operator<<(std::ostream& out, const Interval& interval);


// Data Types for FrechetLight:


class CPoint {
private:
	PointID point;
	distance_t fraction;

	void normalize() {
		assert(fraction >= 0. && fraction <= 1.);
		if (fraction == 1.) {
			fraction = 0.;
			++point;
		}
	}
public:
	CPoint(PointID point, distance_t fraction)
		: point(point), fraction(fraction)
	{
			normalize();
	}
	CPoint() : point(std::numeric_limits<PointID::IDType>::max()), fraction(0.) {}
	
	PointID getPoint() const { return point; } 
	distance_t getFraction() const { return fraction; } 
	distance_t convert() const { return (distance_t) point + fraction; }
	void setPoint(PointID point) { this->point = point; }
	void setFraction(distance_t frac) { fraction = frac; normalize(); }
		
	bool operator<(CPoint const& other) const {
		return point < other.point || (point == other.point && fraction < other.fraction);
	}
	bool operator<=(CPoint const& other) const {
		return point < other.point || (point == other.point && fraction <= other.fraction);
	}
	bool operator>(CPoint const& other) const {
		return point > other.point || (point == other.point && fraction > other.fraction);
	}
	bool operator>=(CPoint const& other) const {
		return point > other.point || (point == other.point && fraction >= other.fraction);
	}
	bool operator==(CPoint const& other) const {
		return point == other.point && fraction == other.fraction;
	}
	bool operator!=(CPoint const& other) const {
		return point != other.point or fraction != other.fraction;
	}
	bool operator<(PointID other) const {
		return point < other;
	}
	bool operator>(PointID other) const {
		return point > other || (point == other && fraction > 0.);
	}
	bool operator<=(PointID other) const {
		return point < other || (point == other && fraction == 0.);
	}
	bool operator>=(PointID other) const {
		return point >= other;
	}
	bool operator==(PointID other) const {
		return point == other && fraction == 0.;
	}
	bool operator!=(size_t other) const {
		return !(point == other);
	}
	CPoint operator+(distance_t other) const { 
	  assert(other <= 1.);
	  PointID p = point; distance_t f = fraction + other; 
	  if (f > 1.) {
	    ++p; f -= 1.;
	  } 
	  return CPoint(p, f); 
	}
	CPoint operator-(distance_t other) const { 
	  assert(other <= 1.);
	  PointID p = point; distance_t f = fraction - other; 
	  if (f < 0.) {
	    --p; f += 1.;
	  } 
	  return CPoint(p, f); 
	}
	CPoint ceil() const {
		return fraction > 0 ? CPoint(point + 1, 0.) : CPoint(point, 0.);
	}
	CPoint floor() const {
		return CPoint(point, 0.);
	}
	std::string to_string() const { 
	  //return std::to_string( (double) point + fraction); 
	  std::stringstream stream;
	  stream << std::fixed << std::setprecision(10) << (double) point + fraction;
	  return stream.str();
	}

	friend std::ostream& operator<<(std::ostream& out, const CPoint& p);
};

struct CInterval;
using CIntervals = std::vector<CInterval>;
using CIntervalsID = ID<CIntervals>;
using CIntervalID = std::size_t;
using CIntervalIDs = std::vector<CIntervalID>;

using CPoints = std::vector<CPoint>;

using CPosition = std::array<CPoint, 2>;
using CPositions = std::vector<CPosition>;

using CurveID = std::size_t;
using CurveIDs = std::vector<CurveID>;

struct CInterval
{
	CPoint begin;
	CPoint end;

#ifdef CERTIFY
	const CInterval* reach_parent = nullptr; 
	CPoint fixed = CPoint(std::numeric_limits<PointID::IDType>::max(),0.);
	CurveID fixed_curve = -1;

	CPosition getLowerRightPos() const { 
	  if (fixed_curve == 0) {
	    CPosition ret = {{fixed, begin}}; 
	    return ret;
	  } else {
	    CPosition ret = {{end, fixed}};
	    return ret;
	  }
	}
	CPosition getUpperLeftPos() const { 
	  if (fixed_curve == 0) {
	    CPosition ret = {{fixed, end}}; 
	    return ret;
	  } else {
	    CPosition ret = {{begin, fixed}};
	    return ret;
	  }
	}

	CInterval(CPoint begin, CPoint end, CPoint fixed, CurveID fixed_curve)
		: begin(begin), end(end), fixed(fixed), fixed_curve(fixed_curve) {}
#endif

	CInterval()
		: begin(std::numeric_limits<PointID::IDType>::max(), 0.),
		  end(std::numeric_limits<PointID::IDType>::lowest(), 0.) {}

	CInterval(CInterval const& other) = default;

	CInterval(CPoint const& begin, CPoint const& end)
                : begin(begin), end(end) {}

	CInterval(PointID point1, distance_t fraction1, PointID point2, distance_t fraction2)
		: begin(point1, fraction1), end(point2, fraction2) {}

	CInterval(PointID begin, PointID end)
		: begin(begin, 0.), end(end, 0.) {}

	
	bool operator<(CInterval const& other) const {
		return begin < other.begin || (begin == other.begin && end < other.end);
	}

	bool is_empty() const { return end < begin; }
	void make_empty() {
		begin = {std::numeric_limits<PointID::IDType>::max(), 0.};
		end = {std::numeric_limits<PointID::IDType>::lowest(), 0.};
	}
	void clamp(CPoint const& min, CPoint const& max) {
		begin = std::max(min, begin);
		end = std::min(max, end);
	}
};

std::ostream& operator<<(std::ostream& out, const CInterval& interval);

class IntersectionAlgorithm
{
public:
	static constexpr distance_t eps = 1e-8;
	
   /*
    * Returns which section of the line segment from line_start to line_end is inside the circle given by circle_center and radius.
    * If the circle and line segment do not intersect, the result is the empty Interval (and outer is the empty Interval, too).
	* Otherwise the result is an interval [x,y] such that the distance at x and at y is at most the radius, i.e., [x,y] is a subset of the free interval. 
	* The optional output "outer" is an interval strictly containing the free interval.
	* In other words, "outer" is an interval [x',y'] containing [x,y] such that x-x', y'-y <= eps and:
	* If x = 0 then x' = -eps, while if x > 0 then the distance at x' is more than the radius.
	* If y = 1 then y' = 1+eps, while if y < 1 then the distance at y' is more than the radius.
    */
	static Interval intersection_interval(Point circle_center, distance_t radius, Point line_start, Point line_end, Interval * outer = nullptr);
private:
	IntersectionAlgorithm() {} // Make class static-only
	static inline bool smallDistanceAt(distance_t interpolate, Point line_start, Point line_end, Point circle_center, distance_t radius_sqr);
	static inline distance_t distanceAt(distance_t interpolate, Point line_start, Point line_end, Point circle_center);

	static constexpr distance_t save_eps = 0.5 * eps;
	static constexpr distance_t save_eps_half = 0.25 * eps;
};

// Ellipse
struct Ellipse
{
	Point center;
	distance_t width;
	distance_t height;
	double alpha;

	void invalidate() { width = -1.; height = -1.; }
	bool is_valid() { return width >= 0 && height >= 0; }
};

Ellipse segmentsToEllipse(Point const& a1, Point const& b1, Point const& a2, Point const& b2, distance_t distance);
