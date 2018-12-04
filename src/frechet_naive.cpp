#include "frechet_naive.h"

#include "defs.h"
#include "filter.h"

#include <vector>
#include <limits>

bool FrechetNaive::lessThan(distance_t distance, Curve const& curve1, Curve const& curve2)
{
	assert(curve1.size() >= 2);
	assert(curve2.size() >= 2);
	distance_t dist_sqr = distance * distance;

	if (curve1[0].dist_sqr(curve2[0]) > dist_sqr || curve1.back().dist_sqr(curve2.back()) > dist_sqr) { return false; }

	distance_t infty = std::numeric_limits<distance_t>::max();

	std::vector<std::vector<distance_t>> reachable1(curve1.size()-1, std::vector<distance_t>(curve2.size(), infty));
	std::vector<std::vector<distance_t>> reachable2(curve1.size(), std::vector<distance_t>(curve2.size()-1, infty));
	for (size_t i = 0; i < curve1.size() - 1; ++i) {
		reachable1[i][0] = 0.;
		if (curve2[0].dist_sqr(curve1[i+1]) > dist_sqr) { break; }
	}
	for (size_t j = 0; j < curve2.size() - 1; ++j) {
		reachable2[0][j] = 0.;
		if (curve1[0].dist_sqr(curve2[j+1]) > dist_sqr) { break; }
	}

	for (size_t i = 0; i < curve1.size(); ++i) {
		for (size_t j = 0; j < curve2.size(); ++j) {
			if (i < curve1.size() - 1 && j > 0) {
				Interval free_int = IntersectionAlgorithm::intersection_interval(curve2[j], distance, curve1[i], curve1[i+1]);
				if (!free_int.is_empty()) {
					if (reachable2[i][j-1] != infty) {
						reachable1[i][j] = free_int.begin;
					}
					else if (reachable1[i][j-1] <= free_int.end) {
						reachable1[i][j] = std::max(free_int.begin, reachable1[i][j-1]);
					}
				}
			}
			if (j < curve2.size() - 1 && i > 0) {
				Interval free_int = IntersectionAlgorithm::intersection_interval(curve1[i], distance, curve2[j], curve2[j+1]);
				if (!free_int.is_empty()) {
					if (reachable1[i-1][j] != infty) {
						reachable2[i][j] = free_int.begin;
					}
					else if (reachable2[i-1][j] <= free_int.end) {
						reachable2[i][j] = std::max(free_int.begin, reachable2[i-1][j]);
					}
				}
			}
		}
	}

	assert((reachable1.back().back() < infty) == (reachable2.back().back() < infty));

	return reachable1.back().back() < infty;
}

bool FrechetNaive::lessThanWithFilters(distance_t distance, Curve const& curve1, Curve const& curve2)
{
	assert(curve1.size());
	assert(curve2.size());

	distance_t dist_sqr = distance * distance;
	if (curve1[0].dist_sqr(curve2[0]) > dist_sqr ||
		curve1.back().dist_sqr(curve2.back()) > dist_sqr) {
		return false;
	}
	if (curve1.size() == 1 && curve2.size() == 1) {
		return true;
	}

	Filter filter(curve1, curve2, distance);


	if (filter.bichromaticFarthestDistance()) {
		return true;
	}

	PointID pos1;
	PointID pos2;
	if (filter.adaptiveGreedy(pos1, pos2)) {
		return true;
	}
	if (filter.negative(pos1, pos2)) {
		return false;
	}
	if (filter.adaptiveSimultaneousGreedy()) {
		return true;
	}

	return lessThan(distance, curve1, curve2);
}
