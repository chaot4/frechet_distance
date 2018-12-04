#pragma once

#include "geometry_basics.h"
#include "times.h"
#include "curves.h"
#include "certificate.h"

class Filter
{
private:
	Certificate cert;
	const Curve *curve1_pt, *curve2_pt;
	distance_t distance;

public:
	Filter(const Curve& curve1, const Curve& curve2, distance_t distance) {
		this->curve1_pt = &curve1;
		this->curve2_pt = &curve2;
		this->distance = distance;
#ifdef CERTIFY
		cert.setCurves(&curve1, &curve2);
		cert.setDistance(distance);
#endif
	}

	Certificate const& getCertificate() { return cert; };

	bool bichromaticFarthestDistance();
	bool greedy();
	bool adaptiveGreedy(PointID& pos1, PointID& pos2);
	bool adaptiveSimultaneousGreedy();
	bool negative(PointID pos1, PointID pos2);

	static bool isPointTooFarFromCurve(Point fixed, const Curve& curve, distance_t distance);
	static bool isFree(Point const& fixed, Curve const& var_curve, PointID start, PointID end,
	                   distance_t distance);
	static bool isFree(Curve const& curve1, PointID start1, PointID end1, Curve const& curve2,
	                   PointID start2, PointID end2, distance_t distance);
	static void increase(size_t& step);
	static void decrease(size_t& step);
};
