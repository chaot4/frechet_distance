#include "filter.h"

bool Filter::isPointTooFarFromCurve(Point fixed, const Curve& curve, distance_t distance)
{
	auto dist_sqr = distance * distance;
	if (fixed.dist_sqr(curve.front()) <= dist_sqr || fixed.dist_sqr(curve.back()) <= dist_sqr) { return false; }
	std::size_t stepsize = 1;
	for (PointID pt = 0; pt < curve.size()-1; ) {
		stepsize = std::min<std::size_t>(stepsize, curve.size() - 1 - pt);
		auto mid = pt + (stepsize+1)/2; 
		auto mid_dist_sqr = fixed.dist_sqr(curve[mid]);
		auto maxdist = std::max(curve.curve_length(pt, mid), curve.curve_length(mid, pt + stepsize));
		auto comp_dist = distance + maxdist;

		if (mid_dist_sqr > std::pow(comp_dist, 2)) {
			pt += stepsize;
			stepsize *= 2;
		}
		else if (stepsize > 1) {
			stepsize /= 2;
		}
		else { return false; }
	}
	return true;
}

bool Filter::isFree(Point const& fixed, Curve const& var_curve, PointID start, PointID end,
            distance_t distance)
{
	auto mid = (start + end + 1) / 2;
	auto max = std::max(var_curve.curve_length(start+1, mid), var_curve.curve_length(mid, end));
	auto mid_dist_sqr = fixed.dist_sqr(var_curve[mid]);

	auto comp_dist = distance - max;
	if (comp_dist > 0 && mid_dist_sqr <= std::pow(comp_dist, 2)) {
		return true;
	}
	else {
		return false;
	}
}

bool Filter::isFree(Curve const& curve1, PointID start1, PointID end1, Curve const& curve2, PointID start2, PointID end2, distance_t distance)
{
	auto mid1 = (start1 + end1 + 1) / 2;
	auto mid2 = (start2 + end2 + 1) / 2;
	auto max1 = std::max(curve1.curve_length(start1 + 1, mid1), curve1.curve_length(mid1, end1));
	auto max2 = std::max(curve2.curve_length(start2 + 1, mid2), curve2.curve_length(mid2, end2));
	auto mid_dist_sqr = curve1[mid1].dist_sqr(curve2[mid2]);

	auto comp_dist = distance - max1 - max2;
	return comp_dist >= 0 && mid_dist_sqr <= std::pow(comp_dist, 2);
}

void Filter::increase(size_t& step)
{
	step = std::ceil(1.5*step);
}

void Filter::decrease(size_t& step)
{
	step /= 2;
}


//NOTE: all calls to cert.XXX() do nothing if CERTIFY is not defined
//TODO: is it better to use #ifdef CERTIFY blocks here to avoid constructing CPosition objects?

bool Filter::bichromaticFarthestDistance() 
{
  	cert.reset();

	auto& curve1 = *curve1_pt;
	auto& curve2 = *curve2_pt;
	auto const& extreme1 = curve1.getExtremePoints();
	auto const& extreme2 = curve2.getExtremePoints();

	distance_t distance_sqr = distance*distance;
	distance_t d;
;
	d = Point{extreme1.min_x, extreme1.min_y}.dist_sqr(Point{extreme2.max_x, extreme2.max_y});
	if (d > distance_sqr) { return false; }
	d = Point{extreme1.min_x, extreme1.max_y}.dist_sqr(Point{extreme2.max_x, extreme2.min_y});
	if (d > distance_sqr) { return false; }
	d = Point{extreme1.max_x, extreme1.min_y}.dist_sqr(Point{extreme2.min_x, extreme2.max_y});
	if (d > distance_sqr) { return false; }
	d = Point{extreme1.max_x, extreme1.max_y}.dist_sqr(Point{extreme2.min_x, extreme2.min_y});
	if (d > distance_sqr) { return false; }

	cert.setAnswer(true);
	cert.addPoint( { CPoint(0, 0.), CPoint(0,0.)});
	if (curve2.size() > 1) {
		cert.addPoint( { CPoint(curve1.size()-1, 0.), CPoint(0,0.)});
	}
	cert.addPoint( { CPoint(curve1.size()-1, 0.), CPoint(curve2.size()-1,0.)});
	cert.validate();

	return true;
}

bool Filter::greedy() 
{
	cert.reset();
	auto& curve1 = *curve1_pt;
	auto& curve2 = *curve2_pt;
	auto distance_sqr = distance*distance;
	auto d_sqr = curve1.back().dist_sqr(curve2.back());
	PointID pos1 = 0;
	PointID pos2 = 0;

	while (pos1 + pos2 < curve1.size() + curve2.size() - 2) {
		d_sqr = std::max(d_sqr, curve1[pos1].dist_sqr(curve2[pos2]));

		if (d_sqr > distance_sqr) { return false; }

		if (curve1.size() - 1 == pos1) {
			++pos2;
		}
		else if (curve2.size() - 1 == pos2) {
			++pos1;
		}
		else {
			distance_t dist1 = curve1[pos1 + 1].dist_sqr(curve2[pos2]);
			distance_t dist2 = curve1[pos1].dist_sqr(curve2[pos2 + 1]);
			distance_t dist12 = curve1[pos1 + 1].dist_sqr(curve2[pos2 + 1]);

			if (dist1 < dist2 && dist1 < dist12) {
				++pos1;
			} else if (dist2 < dist12) {
				++pos2;
			} else {
				++pos1;
				++pos2;
			}
		}
	}

	return true;
}

bool Filter::adaptiveGreedy(PointID& pos1, PointID& pos2)
{
	cert.reset();
	auto& curve1 = *curve1_pt;
	auto& curve2 = *curve2_pt;
	auto const distance_sqr = distance*distance;
	//PointID pos1 = 0;
	//PointID pos2 = 0;
	pos1 = 0;
	pos2 = 0;
	cert.addPoint({ CPoint(pos1, 0.), CPoint(pos2, 0.) });
	
	if (curve1[0].dist_sqr(curve2[0]) > distance_sqr || curve1.back().dist_sqr(curve2.back()) > distance_sqr) { return false; }
	
	std::size_t step = std::max(curve1.size(), curve2.size());
	while (pos1 + pos2 < curve1.size() + curve2.size() - 2) {
		//++numSteps;
		// if we have to do the step on curve 2
		if (curve1.size() - 1 == pos1) {
			auto new_pos2 = std::min<PointID::IDType>(pos2 + step, curve2.size() - 1);
			if (isFree(curve1[pos1], curve2, pos2, new_pos2, distance)) {
				global::times.incrementGreedySteps(new_pos2 - pos2);
				pos2 = new_pos2;
				cert.addPoint({ CPoint(pos1, 0.), CPoint(pos2, 0.) });
				//step *= 2;
				increase(step);
			}
			else if (step == 1) {
				return false;
			}
			else {
				global::times.incrementGreedySteps(0);
				decrease(step);
			}
		}
		// if we have to do the step on curve 1
		else if (curve2.size() - 1 == pos2) {
			auto new_pos1 = std::min<PointID::IDType>(pos1 + step, curve1.size() - 1);
			if (isFree(curve2[pos2], curve1, pos1, new_pos1, distance)) {
				global::times.incrementGreedySteps(new_pos1 - pos1);
				pos1 = new_pos1;
				cert.addPoint({ CPoint(pos1, 0.), CPoint(pos2, 0.) });
				//step *= 2;
				increase(step);
			}
			else if (step == 1) {
				return false;
			}
			else {
				global::times.incrementGreedySteps(0);
				decrease(step);
			}
		}
		// if we cannot reduce the step size
		else if (step == 1) {
			auto dist1 = curve1[pos1 + 1].dist_sqr(curve2[pos2]);
			auto dist2 = curve1[pos1].dist_sqr(curve2[pos2 + 1]);
			auto dist12 = curve1[pos1 + 1].dist_sqr(curve2[pos2 + 1]);

			if (dist1 <= distance_sqr && dist1 < dist2 && dist1 < dist12) { ++pos1; }
		       	else if (dist2 <= distance_sqr && dist2 < dist12) { ++pos2; }
		       	else if (dist12 <= distance_sqr) {  ++pos1; ++pos2; }
		       	else { return false; }

			cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );

			global::times.incrementGreedySteps(1);

			step = 2;
		}
		else {
			auto new_pos1 = std::min<PointID::IDType>(pos1 + step, curve1.size() - 1);
			auto new_pos2 = std::min<PointID::IDType>(pos2 + step, curve2.size() - 1);
			bool step1_possible = isFree(curve2[pos2], curve1, pos1, new_pos1, distance);
			bool step2_possible = isFree(curve1[pos1], curve2, pos2, new_pos2, distance);

			if (step1_possible && step2_possible) {
				auto dist_after_step1 = curve1[new_pos1].dist_sqr(curve2[pos2]);
				auto dist_after_step2 = curve1[pos1].dist_sqr(curve2[new_pos2]);
				if (dist_after_step1 <= dist_after_step2) {
					global::times.incrementGreedySteps(new_pos1 - pos1);
					pos1 = new_pos1;
				}
				else {
					global::times.incrementGreedySteps(new_pos2 - pos2);
					pos2 = new_pos2;
				}
				cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
				//step *= 2;
				increase(step);
			}
			else if (step1_possible) {
				global::times.incrementGreedySteps(new_pos1 - pos1);
				pos1 = new_pos1;
				// for some reason, not increasing the stepsize here is slightly faster
				// step *= 2;
				// increase(step);
				cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
			}
			else if (step2_possible) {
				global::times.incrementGreedySteps(new_pos2 - pos2);
				pos2 = new_pos2;
				// for some reason, not increasing the stepsize here is slightly faster
				// step *= 2;
				// increase(step);
				cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
			}
			else {
				global::times.incrementGreedySteps(0);
				decrease(step);
			}
		}
	}

	cert.setAnswer(true);
	cert.validate();


	return true;
}

bool Filter::adaptiveSimultaneousGreedy()
{
	cert.reset();
	auto& curve1 = *curve1_pt;
	auto& curve2 = *curve2_pt;

	auto distance_sqr = distance*distance;
	PointID pos1 = 0;
	PointID pos2 = 0;
	cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
	
	if (curve1[0].dist_sqr(curve2[0]) > distance_sqr || curve1.back().dist_sqr(curve2.back()) > distance_sqr) { return false; }
	
	std::size_t step = std::max(curve1.size(), curve2.size());
	while (pos1 + pos2 < curve1.size() + curve2.size() - 2) {
		// if we have to do the step on curve 2
		if (curve1.size() - 1 == pos1) {
			auto new_pos2 = std::min<PointID::IDType>(pos2 + step, curve2.size() - 1);
			if (isFree(curve1[pos1], curve2, pos2, new_pos2, distance)) {
				pos2 = new_pos2;
				cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
				//step *= 2;
				increase(step);
			}
			else if (step == 1) {
				return false;
			}
			else {
				decrease(step);
			}
		}
		// if we have to do the step on curve 1
		else if (curve2.size() - 1 == pos2) {
			auto new_pos1 = std::min<PointID::IDType>(pos1 + step, curve1.size() - 1);
			if (isFree(curve2[pos2], curve1, pos1, new_pos1, distance)) {
				pos1 = new_pos1;
				cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );
				//step *= 2;
				increase(step);
			}
			else if (step == 1) {
				return false;
			}
			else {
				decrease(step);
			}
		}
		// if we cannot reduce the step size
		else if (step == 1) {
			auto dist1 = curve1[pos1 + 1].dist_sqr(curve2[pos2]);
			auto dist2 = curve1[pos1].dist_sqr(curve2[pos2 + 1]);
			auto dist12 = curve1[pos1 + 1].dist_sqr(curve2[pos2 + 1]);

			if (dist1 <= distance_sqr && dist1 < dist2 && dist1 < dist12) { ++pos1; }
			else if (dist2 <= distance_sqr && dist2 < dist12) { ++pos2; }
			else if (dist12 <= distance_sqr) { ++pos1; ++pos2; }
			else { return false; }
			cert.addPoint( {CPoint(pos1, 0.), CPoint(pos2, 0.)} );

			step = 2;
		}
		else {
			PointID new_pos1 = std::min<PointID::IDType>(pos1 + step, curve1.size() - 1);
			size_t step2 = (step * (curve2.size() - pos2)) / (curve1.size() - pos1);
			if (step2 < 1) { step2 = 1; }
			PointID new_pos2 = std::min<PointID::IDType>(pos2 + step2, curve2.size() - 1);
			if (isFree(curve1, pos1, new_pos1, curve2, pos2, new_pos2, distance)) {
				if (pos1 != new_pos1 && pos2 != new_pos2) {
					cert.addPoint( {CPoint(pos1+1, 0.), CPoint(pos2+1, 0.)} );
				}
				if (new_pos1 > pos1+1) {
					cert.addPoint( {CPoint(new_pos1, 0.), CPoint(pos2+1, 0.)} );
				}
				if (new_pos2 > pos2+1) {
					cert.addPoint( {CPoint(new_pos1, 0.), CPoint(new_pos2, 0.)} );
				}
				pos1 = new_pos1;
				pos2 = new_pos2;
				increase(step);
			}
			else {
				decrease(step);
			}
		}
	}

	cert.setAnswer(true);
	cert.validate();

	return true;
}

bool Filter::negative(PointID position1, PointID position2)
{
	cert.reset();
	auto& curve1 = *curve1_pt;
	auto& curve2 = *curve2_pt;

	distance_t distance_sqr = distance * distance;
	if (curve1[0].dist_sqr(curve2[0]) > distance_sqr || curve1.back().dist_sqr(curve2.back()) > distance_sqr) { return true; }
	
	size_t pos1 = position1;
	size_t pos2 = position2;
	for (size_t step = 1; pos1 + step <= curve1.size(); increase(step)) {
		size_t cur_pos1 = pos1 + step - 1;
		if (isPointTooFarFromCurve(curve1[cur_pos1], curve2, distance)) {
		       	cert.setAnswer(false);	
			cert.addPoint({CPoint(cur_pos1, 0.), CPoint(0, 0.)});
			cert.addPoint({CPoint(cur_pos1, 0.), CPoint(curve2.size()-1, 0.)});
			cert.validate();
			return true;
		}
	}
	for (size_t step = 1; pos2 + step <= curve2.size(); increase(step)) {
		size_t cur_pos2 = pos2 + step - 1;
		if (isPointTooFarFromCurve(curve2[cur_pos2], curve1, distance)) { 
		       	cert.setAnswer(false);	
			cert.addPoint({CPoint(curve1.size()-1, 0.), CPoint(cur_pos2, 0.)});
			cert.addPoint({CPoint(0, 0.), CPoint(cur_pos2, 0.)});
			cert.validate();
			return true;
		}
	}

	return false;
}
