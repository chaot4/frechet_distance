#include "certificate.h"

#define CHECK(correct, error)                                                                	\
	do {                                                                       		\
		if (! (correct)) {                                                            	\
			std::cerr << "CERTIFICATE CHECK (claim:" << (lessThan ? "LESS THAN" : "GREATER") << ") FAILED: " << error << std::endl; \
			std::cerr << "INSTANCE:" << std::endl;											\
			std::cerr << curve_pair[0]->filename << std::endl;									\
			std::cerr << curve_pair[1]->filename << std::endl;									\
			std::cerr << dist << std::endl;												\
			std::cerr << std::flush;												\
			return false;														\
			dump_certificate();													\
			std::abort();														\
		}																\
	} while (0)

void Certificate::dump_certificate() const {
	std::cout << "CERTIFICATE DUMP:" << std::endl;
	for (size_t t = 0; t < traversal.size(); t++) {
		std::cout << t << ": (" << traversal[t][0].to_string() << ", " << traversal[t][1].to_string() << ")" << std::endl;
	}
}

bool Certificate::feasible(const CPosition & pt) const {
	return feasible(pt[0], pt[1]);
}

bool Certificate::feasible(const CPoint & pt1, const CPoint & pt2) const {
	return curve_pair[0]->interpolate_at(pt1).dist_sqr(curve_pair[1]->interpolate_at(pt2)) <= dist_sqr;
}	

//checks whether segment [start_pt, end_pt] on variable_curve, is non_empty at fixed_point on fixed_curve
bool Certificate::nonEmpty(CurveID fixed_curve, const CPoint& fixed_point, const CPoint& start_pt, const CPoint& end_point) const {
	Point fixed = curve_pair[fixed_curve]->interpolate_at(fixed_point);
	Point start = curve_pair[1-fixed_curve]->interpolate_at(start_pt);
	Point end = curve_pair[1-fixed_curve]->interpolate_at(end_point);
	Interval interval = IntersectionAlgorithm::intersection_interval(fixed, dist, start, end);
	if (!interval.is_empty()) {
		std::cout << "free is [" << interval.begin << ", " << interval.end << "]" << std::endl;
		std::cout << "fixed: " << fixed << std::endl;
		std::cout << "start: " << start << std::endl;
		std::cout << "end: " << end << std::endl;
		std::cout << "dist: " << dist << std::endl;
	}
	return interval.is_empty();
}

CPoint nextIntegralPoint(const CPoint& point) {
	return CPoint(point.getPoint()+1, 0.); 
}

CPoint prevIntegralPoint(const CPoint& point) {
	if (point.getFraction() > 0) {
		return CPoint(point.getPoint(), 0.);
	} else {
		return CPoint(point.getPoint()-1, 0.); 
	}
}


bool Certificate::check() const {

	auto& curve1 = *curve_pair[0]; 
	auto& curve2 = *curve_pair[1]; 

	if (!isValid()) {
		std::cerr << "Invalid certificate" << std::endl;
		return false;
	}

	if (lessThan) {
		size_t T = traversal.size();

		CHECK(traversal[0][0] == 0 and traversal[0][1] == 0, "start point incorrect");
		CHECK(feasible(traversal[0]), "start point not feasible");
		CHECK(traversal[T-1][0] == curve1.size()-1 and traversal[T-1][1] == curve2.size()-1, "end point incorrect");
		CHECK(feasible(traversal[T-1]), "end point not feasible");

		for (size_t t = 1; t < traversal.size(); t++) {
			CHECK(feasible(traversal[t]), "Start point of " + std::to_string(t) + "-th segement is non-feasible"); 
			if (traversal[t][0] == traversal[t-1][0]) { //staying in curve 1, advancing in curve 2
				CHECK(traversal[t-1][1] < traversal[t][1], "Monotonicity violated at segment " + std::to_string(t));
				for (size_t i2 = traversal[t-1][1].ceil().getPoint(); i2 <= traversal[t][1].floor().getPoint(); i2++) {
					CHECK(feasible(traversal[t][0], CPoint(i2,0.)), std::to_string(t)+"-th segment passes through non-feasible point (" + traversal[t][0].to_string() + ", " + std::to_string(i2)+")");
				}
			} else if (traversal[t][1] == traversal[t-1][1]) { //staying in curve 2, advancing in curve 1
				CHECK(traversal[t-1][0] < traversal[t][0], "Monotonicity violated at segment " + std::to_string(t));
				for (size_t i1 = traversal[t-1][0].ceil().getPoint(); i1 <= traversal[t][0].floor().getPoint(); i1++) {
					CHECK(feasible(CPoint(i1, 0.), traversal[t][1]), std::to_string(t)+"-th segment passes through non-feasible point (" + std::to_string(i1)  + ", " + traversal[t][1].to_string() +")");
				}
			} else {//advancing in both at cellular (!) level
				CHECK(traversal[t-1][0] < traversal[t][0], "Monotonicity violated at segment " + std::to_string(t));
				CHECK(traversal[t-1][1] < traversal[t][1], "Monotonicity violated at segment " + std::to_string(t));
				CPoint nextintegral1 = CPoint(traversal[t-1][0].getPoint()+1, 0.);
				CPoint nextintegral2 = CPoint(traversal[t-1][1].getPoint()+1, 0.);
				CHECK(traversal[t][0] <= nextintegral1 and traversal[t][1] <= nextintegral2, "Traversal at segment " + std::to_string(t) + " not at cellular level");
			}
		}	
		return true;
	} else {
		size_t T = traversal.size();

		CHECK(traversal[0][0] == curve1.size() -1  or traversal[0][1] == 0, "start point does not lie on the lower or right boundary");
		CHECK(not feasible(traversal[0]), "start point is free");
		CHECK(traversal[T-1][0] == 0 or traversal[T-1][1] == curve2.size()-1, "end point does not lie on the upper or left boundary");
		CHECK(not feasible(traversal[T-1]), "end point is free");

		for (size_t t = 1; t < T; t++) {
			//std::cout << "part " << t << std::endl;
			if (traversal[t][0] >= traversal[t-1][0] and traversal[t][1] <= traversal[t-1][1]) {
				continue;
			} else if (traversal[t][0] == traversal[t-1][0]) {
				CPoint cur  = traversal[t-1][1];
				CPoint next = nextIntegralPoint(traversal[t-1][1]);
				while (next < traversal[t][1]) {
					//std::cout << "\tchecking [" << cur.to_string() << ", " << next.to_string() << "] in curve2 from " << traversal1[t].to_string() << std::endl;
					CHECK(nonEmpty(0, traversal[t][0], cur, next), std::to_string(t) + "-th part has free points");
					cur = next;
					next = nextIntegralPoint(cur);
				}
				//std::cout << "\tchecking [" << cur.to_string() << ", " << traversal2[t].to_string() << "] in curve2 from " << traversal1[t].to_string() << std::endl;
				CHECK(nonEmpty(0, traversal[t][0], cur, traversal[t][1]), std::to_string(t) + "-th part has free points");
			} else if (traversal[t][1] == traversal[t-1][1]) {
				CPoint cur  = traversal[t-1][0];
				CPoint prev = prevIntegralPoint(traversal[t-1][0]);
				while (prev > traversal[t][0]) {
					//std::cout << "\tchecking [" << cur.to_string() << ", " << prev.to_string() << "] in curve1 from " << traversal2[t].to_string() << std::endl;
					CHECK(nonEmpty(1, traversal[t][1], cur, prev), std::to_string(t) + "-th part has free points");
					cur = prev;
					prev = prevIntegralPoint(cur);
				}
				//std::cout << "\tchecking [" << cur.to_string() << ", " << traversal1[t].to_string() << "] in curve1 from " << traversal1[t].to_string() << std::endl;
				CHECK(nonEmpty(1, traversal[t][1], cur, traversal[t][0]), std::to_string(t) + "-th part has free points");
			} else {
				CHECK(false, "invalid move at " + std::to_string(t) + "-th part");
			}
		}

		return true;
	}
}
