#include "defs.h"
#include "query.h"
#include "frechet_light.h"

#include <fstream>
#include <limits>
#include <random>
#include <cmath>
#include <chrono>

void printUsage()
{
	std::cout <<
		"Usage: ./create_benchmark <curve_data_file> <curve_directory> <out_prefix> <number_of_queries>\n"
		"\n";
}

struct BenchmarkQuery {
	Curve const& curve1;
	Curve const& curve2;
	distance_t distance;

	BenchmarkQuery(Curve const& curve1, Curve const& curve2, distance_t distance)
		: curve1(curve1), curve2(curve2), distance(distance) {}
};
using BenchmarkQueries = std::vector<BenchmarkQuery>;
using BenchmarkQueriesVec = std::vector<std::vector<BenchmarkQueries>>;

CurveID getRandomCurveID(Query const& query)
{
	static const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	static std::default_random_engine gen(seed);
	auto const& curves = query.getCurves();

	std::uniform_int_distribution<std::size_t> distribution(0, curves.size()-1);
	return distribution(gen);
}

std::size_t getIndexInRange(Query const& query, int k)
{
	static const auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	static std::default_random_engine gen(seed+13);
	auto const& curves = query.getCurves();

	auto lower_bound = pow(2, k)-1;
	auto upper_bound = pow(2, k+1)-2;
	if (upper_bound > curves.size()-1) { upper_bound = curves.size()-1; }

	std::uniform_int_distribution<std::size_t> distribution(lower_bound, upper_bound);
	return distribution(gen);
}

int main(int argc, char* argv[])
{
	if (argc != 5) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_data_file(argv[1]);
	std::string curve_directory(argv[2]);
	std::string out_prefix = argv[3];
	std::size_t number_of_queries = std::stoi(argv[4]);

	Query query(curve_directory);
	query.readCurveData(curve_data_file);
	query.getReady();
	FrechetLight frechet;

	std::size_t log_n = log2(query.getCurves().size());
	std::vector<std::size_t> ks(log_n);
	std::iota(ks.begin(), ks.end(), 1);

	std::vector<int> ls_plus(13);
	std::iota(ls_plus.begin(), ls_plus.end(), -10);
	std::vector<int> ls_minus(10);
	std::iota(ls_minus.begin(), ls_minus.end(), -10);

	BenchmarkQueriesVec benchmark_queries_vec_plus(ks.size());
	BenchmarkQueriesVec benchmark_queries_vec_minus(ks.size());
	for (auto& vec: benchmark_queries_vec_plus) { vec.resize(ls_plus.size()); }
	for (auto& vec: benchmark_queries_vec_minus) { vec.resize(ls_minus.size()); }

	//
	// calculate benchmark
	//

	for (std::size_t i = 0; i < number_of_queries; ++i) {
		auto curve1_id = getRandomCurveID(query);
		auto const& curve1 = query.getCurves()[curve1_id];

		std::vector<std::pair<double, CurveID>> distance_curve_pairs;
		for (std::size_t curve_id = 0; curve_id < query.getCurves().size(); ++curve_id) {
			auto curve = query.getCurves()[curve_id];
			auto distance = frechet.calcDistance(curve1, curve);
			distance_curve_pairs.emplace_back(distance, curve_id);
		}
		std::sort(distance_curve_pairs.begin(), distance_curve_pairs.end());

		for (std::size_t k_index = 0; k_index < ks.size(); ++k_index) {
			auto k = ks[k_index];
			auto curve2_index = getIndexInRange(query, k);
			auto const& curve2 = query.getCurves()[distance_curve_pairs[curve2_index].second];
			auto delta_star = frechet.calcDistance(curve1, curve2);

			for (std::size_t l_index = 0; l_index < ls_minus.size(); ++l_index) {
				auto l = ls_minus[l_index];
				auto distance = delta_star*(1. - pow(2,l));

				BenchmarkQuery benchmark_query(curve1, curve2, distance);
				benchmark_queries_vec_minus[k_index][l_index].push_back(benchmark_query);
			}

			for (std::size_t l_index = 0; l_index < ls_plus.size(); ++l_index) {
				auto l = ls_plus[l_index];
				auto distance = delta_star*(1. + pow(2,l));

				BenchmarkQuery benchmark_query(curve1, curve2, distance);
				benchmark_queries_vec_plus[k_index][l_index].push_back(benchmark_query);
			}
		}
	}

	//
	// export
	//

	for (std::size_t k_index = 0; k_index < ks.size(); ++k_index) {
		for (std::size_t l_index = 0; l_index < ls_plus.size(); ++l_index) {
			std::string filename = out_prefix + "_" + std::to_string(ks[k_index]) + "_" +
				std::to_string(ls_plus[l_index]) + "_" + "plus.txt";
			std::ofstream f(filename);
			if (!f.is_open()) { std::cerr << "Error opening file.\n"; std::exit(1); }

			f << std::setprecision(20);
			auto const& benchmark_queries = benchmark_queries_vec_plus[k_index][l_index];
			for (auto const& q: benchmark_queries) {
				f << q.curve1.filename << " " << q.curve2.filename << " " << q.distance << "\n";
			}

			f.close();
		}
	}

	for (std::size_t k_index = 0; k_index < ks.size(); ++k_index) {
		for (std::size_t l_index = 0; l_index < ls_minus.size(); ++l_index) {
			std::string filename = out_prefix + "_" + std::to_string(ks[k_index]) + "_" +
				std::to_string(ls_minus[l_index]) + "_" + "minus.txt";
			std::ofstream f(filename);
			if (!f.is_open()) { std::cerr << "Error opening file.\n"; std::exit(1); }

			f << std::setprecision(20);
			auto const& benchmark_queries = benchmark_queries_vec_minus[k_index][l_index];
			for (auto const& q: benchmark_queries) {
				f << q.curve1.filename << " " << q.curve2.filename << " " << q.distance << "\n";
			}

			f.close();
		}
	}
}
