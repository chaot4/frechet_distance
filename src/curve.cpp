#include "curve.h"

Curve::Curve(const Points& points)
	: points(points), prefix_length(points.size())
{
	if (points.empty()) { return; }

	auto const& front = points.front();
	extreme_points = { front.x, front.y, front.x, front.y };
	prefix_length[0] = 0;

	for (PointID i = 1; i < points.size(); ++i)
	{
		auto segment_distance = points[i - 1].dist(points[i]);
		prefix_length[i] = prefix_length[i - 1] + segment_distance;

		extreme_points.min_x = std::min(extreme_points.min_x, points[i].x);
		extreme_points.min_y = std::min(extreme_points.min_y, points[i].y);
		extreme_points.max_x = std::max(extreme_points.max_x, points[i].x);
		extreme_points.max_y = std::max(extreme_points.max_y, points[i].y);
	}
}

void Curve::push_back(Point const& point)
{
	if (prefix_length.size()) {
		auto segment_distance = points.back().dist(point);
		prefix_length.push_back(prefix_length.back() + segment_distance);
	}
	else {
		prefix_length.push_back(0);
	}

	extreme_points.min_x = std::min(extreme_points.min_x, point.x);
	extreme_points.min_y = std::min(extreme_points.min_y, point.y);
	extreme_points.max_x = std::max(extreme_points.max_x, point.x);
	extreme_points.max_y = std::max(extreme_points.max_y, point.y);

	points.push_back(point);
}

auto Curve::getExtremePoints() const -> ExtremePoints const&
{
	return extreme_points;
}

distance_t Curve::getUpperBoundDistance(Curve const& other) const
{
	auto const& extreme1 = this->getExtremePoints();
	auto const& extreme2 = other.getExtremePoints();

	Point min_point{ std::min(extreme1.min_x, extreme2.min_x),
		std::min(extreme1.min_y, extreme2.min_y) };
	Point max_point = { std::max(extreme1.max_x, extreme2.max_x),
		std::max(extreme1.max_y, extreme2.max_y) };

	return min_point.dist(max_point);
}

std::ostream& operator<<(std::ostream& out, const Curve& curve)
{
    out << "[";
	for (auto const& point: curve) {
		out << point << ", ";
	}
    out << "]";

    return out;
}
