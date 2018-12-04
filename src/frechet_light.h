#pragma once

#include "defs.h"
#include "filter.h"
#include "frechet_abstract.h"
#include "frechet_light_types.h"
#include "geometry_basics.h"
#include "id.h"
#include "certificate.h"
#include "curves.h"
#ifdef CERTIFY
#include "priority_search_tree.h"
#endif

#include <array>
#include <vector>

class FrechetLight final : public FrechetAbstract
{
	using CurvePair = std::array<Curve const*, 2>;

public:
	static constexpr distance_t eps = 1e-10;
	
	FrechetLight() = default;
	void buildFreespaceDiagram(distance_t distance, Curve const& curve1, Curve const& curve2);
	bool lessThan(distance_t distance, Curve const& curve1, Curve const& curve2);
	bool lessThanWithFilters(distance_t distance, Curve const& curve1, Curve const& curve2);
	distance_t calcDistance(Curve const& curve1, Curve const& curve2);
	void clear();

	CurvePair getCurvePair() const;
	Certificate& computeCertificate();
	const Certificate& getCertificate() const { return cert; } 

	void setPruningLevel(int pruning_level);
	void setRules(std::array<bool,5> const& enable) override;

	std::size_t getNumberOfBoxes() const;

	std::size_t non_filtered = 0;

private:
	CurvePair curve_pair;

	distance_t distance;
	distance_t dist_sqr;

	std::vector<CIntervals> reachable_intervals_vec;
	QSimpleIntervals qsimple_intervals;
	std::size_t num_boxes;

	// 0 = no pruning ... 6 = full pruning
	int pruning_level = 6;
	// ... and additionally bools to enable/disable rules
	bool enable_box_shrinking = true;
	bool enable_empty_outputs = true;
	bool enable_propagation1 = true;
	bool enable_propagation2 = true;
	bool enable_boundary_rule = true;

#ifdef VIS
	CIntervals unknown_intervals;
	CIntervals connections;
	CIntervals free_non_reachable;
	CIntervals reachable_intervals;

	struct Cell {
		PointID i, j;
		Cell(PointID i, PointID j) : i(i), j(j) {}
	};

	std::vector<Cell> cells;
	std::vector<Cell> const& getCells() const { return cells; }
#endif

	Certificate cert;
#ifdef CERTIFY
	CIntervals empty_intervals;

	using RangeSearch = PrioritySearchTree<CPoint, CIntervalID>;
	RangeSearch intervals_remaining;
#endif

	CInterval getInterval(Point const& point, Curve const& curve, PointID i) const;
	CInterval getInterval(Point const& point, Curve const& curve, PointID i, CInterval* ) const;
	void merge(CIntervals& v, CInterval const& i) const;

	Outputs createFinalOutputs();
	Inputs computeInitialInputs();
	// XXX: consistency of arguments in following functions!
	distance_t getDistToPointSqr(const Curve& curve, Point const& point) const;
	bool isClose(Point const& point, Curve const& curve) const;
	CPoint getLastReachablePoint(Point const& point, Curve const& curve) const;
	bool isTopRightReachable(Outputs const& outputs) const;
	void computeOutputs(Box const& initial_box, Inputs const& initial_inputs, Outputs& final_outputs);

	void getReachableIntervals(BoxData& data);

	// subfunctions of getReachableIntervals
	bool emptyInputsRule(BoxData& data);
	void boxShrinkingRule(BoxData& data);
	void handleCellCase(BoxData& data);
	void getQSimpleIntervals(BoxData& data);
	void calculateQSimple1(BoxData& data);
	void calculateQSimple2(BoxData& data);
	bool boundaryPruningRule(BoxData& data);
	void splitAndRecurse(BoxData& data);

	// intervals used in getReachableIntervals and subfunctions
	CInterval const empty;
	CInterval const* firstinterval1;
	CInterval const* firstinterval2;
	distance_t min1_frac, min2_frac;
	QSimpleInterval qsimple1, qsimple2;
	CInterval out1, out2;
	// TODO: can those be made members of out1, out2?
	bool out1_valid = false, out2_valid = false;

	// qsimple interval calculation functions
	QSimpleInterval getFreshQSimpleInterval(const Point& fixed_point, PointID min1, PointID max1, const Curve& curve) const;
	bool updateQSimpleInterval(QSimpleInterval& qsimple, const Point& fixed_point, PointID min1, PointID max1, const Curve& curve) const;
	void continueQSimpleSearch(QSimpleInterval& qsimple, const Point& fixed_point, PointID min1, PointID max1, const Curve& curve) const;

	bool isOnLowerRight(const CPosition& pt) const;
	bool isOnUpperLeft(const CPosition& pt) const;

	void initCertificate(Inputs const& initial_inputs);
	void certSetValues(CInterval& interval, CInterval const& parent, PointID point_id, CurveID curve_id);
	void certAddEmpty(CPoint begin, CPoint end, CPoint fixed_point, CurveID fixed_curve);
	void certAddNonfreeParts(const CInterval& outer, PointID min, PointID max, PointID fixed_point, CurveID fixed_curve);

	// Those are empty function if VIS is not defined
	void visAddReachable(CInterval const& cinterval);
	void visAddUnknown(CPoint begin, CPoint end, CPoint fixed_point, CurveID fixed_curve);
	void visAddConnection(CPoint begin, CPoint end, CPoint fixed_point, CurveID fixed_curve);
	void visAddFreeNonReachable(CPoint begin, CPoint end, CPoint fixed_point, CurveID fixed_curve);
	void visAddCell(Box const& box);

	// Could also be done via getter member functions, but vis is a special
	// case in needing access to the internal structures.
	friend class FreespaceLightVis;
};
