// Make sure to always print in the unit tests
#ifdef NVERBOSE
#	undef NVERBOSE
#endif
#include "unit_tests.h"

#include <cmath>
#include <random>
#include <unordered_set>

#include "defs.h"
#include "frechet_light.h"
#include "parser.h"
#include "priority_search_tree.h"
#include "range_tree.h"
#include "curves.h"

#ifdef CERTIFY
#include "freespace_light_vis.h"
#endif

//
// Define some helpers
//

#define TEST(x)                                                                \
	do {                                                                       \
		if (!(x)) {                                                            \
			std::cout << "\n";                                                 \
			std::cout << "TEST_FAILED!\n";                                     \
			std::cout << "File: " << __FILE__ << "\n";                         \
			std::cout << "Line: " << __LINE__ << "\n";                         \
			std::cout << "Function: " << __func__ << "\n";                     \
			std::cout << "Test: " << #x << "\n";                               \
			std::cout << "\n";                                                 \
			std::cout << std::flush;                                           \
			std::abort();                                                      \
		}                                                                      \
	} while (0)

namespace
{

Curve getCurve1() {
	Curve curve;
	curve.push_back({0., 0.});
	curve.push_back({2., 0.});

	return curve;
}

Curve getCurve2() {
	Curve curve;
	curve.push_back({0., 1.});
	curve.push_back({1., 1.5});
	curve.push_back({2., 1.});

	return curve;
}

Curve getCurve3() {
	Curve curve;
	curve.push_back({0., 0.});
	curve.push_back({1., 0.});
	curve.push_back({2., 0.});
	curve.push_back({3., 1.});
	curve.push_back({2., 2.});
	curve.push_back({1., 2});
	curve.push_back({1., 1.9});
	curve.push_back({1., 1.8});
	curve.push_back({1., 1.7});

	return curve;
}

// bool roughlyEqual(distance_t a, distance_t b)
// {
//     return std::abs(a-b) < 0.001;
// }

} // end anonymous

//
// Unit Tests
//

void unit_tests::testAll()
{
	unit_tests::testPrioritySearchTree();
	unit_tests::testGeometricBasics();
#ifdef CERTIFY
	unit_tests::testFreespaceLightVis();
#endif
	unit_tests::testLightCertificate();
	unit_tests::testRangeTree();
}

void unit_tests::testGeometricBasics()
{
	// Test Points
	Point p1{0., 0.};
	Point p2{2., 0.};
	Point p3{3., 4.};

	TEST(p1.dist(p2) == 2);
	TEST(p1.dist(p3) == 5);

	// Test Curves
	auto curve1 = getCurve1();
	auto curve2 = getCurve2();

	TEST(curve1.size() == 2 && curve2.size() == 3);
	TEST(curve1.curve_length(0, 1) == 2);
}

#ifdef CERTIFY
void unit_tests::testFreespaceLightVis()
{
	auto curve2 = getCurve2();
	auto curve3 = getCurve3();
	distance_t distance = 1.5;

	FrechetLight frechet;
	frechet.lessThan(distance, curve2, curve3);

	FreespaceLightVis vis(frechet);
	vis.exportToSvg("freespace_light_vis.svg");
}
#endif

void unit_tests::testLightCertificate() {
	//unit_tests::testLightCertificate("../../testdaten/simple-curve1.txt", "../../testdaten/simple-curve2.txt", 5);
	//unit_tests::testLightCertificate("../../testdaten/simple-curve1.txt", "../../testdaten/simple-curve2.txt", 2);

	// unit_tests::testLightCertificate("../../testdaten/simple-curve3.txt", "../../testdaten/simple-curve4.txt", 0.75);

	unit_tests::testLightCertificate("../../testdaten/file-012086.dat", "../../testdaten/file-019953.dat", 1272.84);
}


void unit_tests::testLightCertificate(std::string curve1file, std::string curve2file, distance_t distance) {
	auto curve1 = parser::readCurve(curve1file);
	auto curve2 = parser::readCurve(curve2file);

	FrechetLight frechet;
	bool output = frechet.lessThan(distance, curve1, curve2);
	Certificate& c = frechet.computeCertificate();
	TEST(c.isValid()); 
	TEST(c.isYes() == output);
	TEST(c.check());

	c.dump_certificate();

}

void unit_tests::testRangeTree()
{
	using Tree = RangeTree<double, int>;

	bool manual_test = false;
	bool randomized_test = true;

	auto matches_naive = [](Tree::Point query, Tree::Points const& points,
	                        std::vector<bool> const& deleted, Tree::Values& result) {
		Tree::Values naive_result;

		for (std::size_t i = 0; i < points.size(); ++i) {
			auto const& point = points[i];
			if (point.x >= query.x && point.y <= query.y && !deleted[i]) {
				naive_result.push_back(i);
			}
		}

		std::sort(result.begin(), result.end());
		std::sort(naive_result.begin(), naive_result.end());

		return result.size() == naive_result.size() &&
			std::equal(result.begin(), result.end(), naive_result.begin());
	};

	Tree range_tree;

	if (manual_test) {
		range_tree.add({0., 0.}, 0);
		range_tree.add({1., 0.}, 1);
		range_tree.add({2., 0.}, 2);
		range_tree.add({0., 1.}, 3);
		range_tree.add({1., 2.}, 4);
		range_tree.add({2., 3.}, 5);

		range_tree.build();
		std::cout << range_tree << "\n";

		std::vector<int> result;
		range_tree.searchAndDelete({.5, 2.5}, result);

		// should be: 1, 2, 4
		for (auto value: result) { std::cout << value << ", "; } std::cout << "\n";
	}

	if (randomized_test) {
		int number_of_points = 10000;
		int number_of_queries = 100;
		int number_of_runs = 1000;
	
		for (int run = 0; run < number_of_runs; ++run) {
			range_tree.clear();

			Tree::Points points;
		
			std::random_device r;
			std::default_random_engine e(r());
			std::uniform_real_distribution<double> rand(-100., 100.);
		
			for (int i = 0; i < number_of_points; ++i) {
				Tree::Point random_point = {rand(e), rand(e)};
				points.push_back(random_point);
		
				range_tree.add(random_point, i);
			}
		
			range_tree.build();
		
			std::vector<int> result;
			std::vector<bool> deleted(number_of_points, false);
			for (int i = 0; i < number_of_queries; ++i) {
				Tree::Point random_query = {rand(e), rand(e)};
		
				result.clear();
				range_tree.searchAndDelete(random_query, result);
		
				TEST(matches_naive(random_query, points, deleted, result));
				for (auto id: result) { deleted[id] = true; }
			}
		}
	}
}

void unit_tests::testPrioritySearchTree()
{
	using Tree = PrioritySearchTree<double, int>;

	bool manual_test = false;
	bool randomized_test = true;

	auto matches_naive = [](Tree::Point query, Tree::Points const& points,
	                        std::vector<bool> const& deleted, Tree::Values& result) {
		Tree::Values naive_result;

		for (std::size_t i = 0; i < points.size(); ++i) {
			auto const& point = points[i];
			if (point.x >= query.x && point.y <= query.y && !deleted[i]) {
				naive_result.push_back(i);
			}
		}

		std::sort(result.begin(), result.end());
		std::sort(naive_result.begin(), naive_result.end());

		return result.size() == naive_result.size() &&
			std::equal(result.begin(), result.end(), naive_result.begin());
	};

	Tree pst;

	if (manual_test) {
		pst.add({0., 0.}, 0);
		pst.add({1., 0.}, 1);
		pst.add({2., 0.}, 2);
		pst.add({0., 1.}, 3);
		pst.add({1., 2.}, 4);
		pst.add({2., 3.}, 5);

		pst.build();
		std::cout << pst << "\n";

		std::vector<int> result;
		pst.searchAndDelete({.5, 2.5}, result);

		// should be: 1, 2, 4
		for (auto value: result) { std::cout << value << ", "; } std::cout << "\n";
	}

	if (randomized_test) {
		int number_of_points = 10000;
		int number_of_queries = 100;
		int number_of_runs = 1000;
	
		for (int run = 0; run < number_of_runs; ++run) {
			pst.clear();

			Tree::Points points;
		
			std::random_device r;
			std::default_random_engine e(r());
			std::uniform_real_distribution<double> rand(-100., 100.);
		
			for (int i = 0; i < number_of_points; ++i) {
				Tree::Point random_point = {rand(e), rand(e)};
				points.push_back(random_point);
		
				pst.add(random_point, i);
			}
		
			pst.build();
		
			std::vector<int> result;
			std::vector<bool> deleted(number_of_points, false);
			for (int i = 0; i < number_of_queries; ++i) {
				Tree::Point random_query = {rand(e), rand(e)};
		
				result.clear();
				pst.searchAndDelete(random_query, result);
		
				TEST(matches_naive(random_query, points, deleted, result));
				for (auto id: result) { deleted[id] = true; }
			}
		}
	}
}

// just in case anyone does anything stupid with this file...
#undef TEST
