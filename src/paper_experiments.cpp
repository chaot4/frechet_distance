#include "defs.h"
#include "frechet_light.h"
#include "query.h"
#include "times.h"
#include "parser.h"
#include "range.h"

#include <array>
#include <chrono>
#include <string>
#include <vector>

using hrc = std::chrono::high_resolution_clock;
using time_point = hrc::time_point;
using ns = std::chrono::nanoseconds;
using TimesRow = std::vector<double>;
using TimesRows = std::vector<TimesRow>;

void comparisonExp();
void partsMeasurementExp();
void datasetStatsExp();
void boxesVsRuntimeExp();
void certificatesRuntime();
void certificatesYesNoRuntime();
void omitRulesExp();
void deciderComparisonExp();
void deciderCountFiltered();

namespace
{

using Strings = std::vector<std::string>;
Strings const curve_directories = {"../../benchmark/sigspatial/",
	"../../benchmark/characters/data/", "../../benchmark/Geolife Trajectories 1.3/data/"};
Strings const curve_data_files = {"../../benchmark/sigspatial/dataset.txt",
	"../../benchmark/characters/data/dataset.txt",
	"../../benchmark/Geolife Trajectories 1.3/data/dataset.txt"};

} // end anonymous namespace

int main()
{
	bool comparison = false;
	bool parts_measurement = false;
	bool dataset_stats = false;
	bool boxes_vs_runtime = false;
	bool certificates_runtime = false;
	bool certificates_yes_no_runtime = true;
	bool omit_rules = false;
	bool decider_comparison = false;
	bool decider_count_filtered = false;

	if (comparison) { comparisonExp(); }
	if (parts_measurement) { partsMeasurementExp(); }
	if (dataset_stats) { datasetStatsExp(); }
	if (boxes_vs_runtime) { boxesVsRuntimeExp(); }
	if (certificates_runtime) { certificatesRuntime(); }
	if (certificates_yes_no_runtime) { certificatesYesNoRuntime(); }
	if (omit_rules) { omitRulesExp(); }
	if (decider_comparison) { deciderComparisonExp(); }
	if (decider_count_filtered) { deciderCountFiltered(); }
}

void printRow(std::vector<double> const& row, int precision = 3)
{
	std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(precision);
	for (std::size_t i = 0; i < row.size()-1; ++i) {
		std::cout << row[i] << " & ";
	}
	std::cout << row.back() << " \\\\\n";
}

template <typename A, typename B>
void printPairs(std::vector<std::pair<A,B>> const& data)
{
	std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(3);
	for (auto const& data_point: data) {
		std::cout << data_point.first << " " << data_point.second << "\n";
	}
}

struct DeciderQuery
{
	Curve curve1;
	Curve curve2;
	distance_t distance;
};
using DeciderQueries = std::vector<DeciderQuery>;

DeciderQueries loadQueries(std::string const& filename, std::string const& curve_directory,
	std::size_t max_queries = std::numeric_limits<std::size_t>::max())
{
	DeciderQueries decider_queries;
	std::ifstream f(filename);

	if (!f.is_open()) {
		std::cerr << "ERROR: Could not open query file: " << filename << "\n";
		std::exit(1);
	}

	std::string curve1_file;
	std::string curve2_file;
	distance_t distance;

	std::size_t query_count = 0;
	while (query_count < max_queries && f >> curve1_file >> curve2_file >> distance) {
		decider_queries.emplace_back();
		decider_queries.back().curve1 = parser::readCurve(curve_directory + curve1_file);
		decider_queries.back().curve2 = parser::readCurve(curve_directory + curve2_file);
		decider_queries.back().distance = distance;
		++query_count;
	}

	return decider_queries;
}

void comparisonExp()
{
	std::cout << "Starting comparison experiment." << std::endl;

	std::size_t num_data_sets = 3;
	Strings query_file_prefixes = {"../test_data/benchmark_queries/sigspatial_query",
		"../test_data/benchmark_queries/characters_query",
		"../test_data/benchmark_queries/geolife_query"};

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	std::vector<double> times;
	for (std::size_t d = 0; d < num_data_sets; ++d) {
		global::times.startPreprocessing();
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.setAlgorithm("light");
		query.getReady();
		global::times.stopPreprocessing();

		for (std::size_t i = 0; i < ks.size(); ++i) {
			auto k = ks[i];

			global::times.startReadingQueryCurve();
			query.readQueryCurves(query_file_prefixes[d] + std::to_string(k) + ".txt");
			global::times.stopReadingQueryCurve();

			query.run_parallel();

			times.push_back(global::times.frechet_query_sum/1000000000.);
			global::times.reset();
		}
	}

	printRow(times);
}

void partsMeasurementExp()
{
	std::cout << "Starting parts measurement experiment." << std::endl;

	std::size_t num_data_sets = 3;
	Strings query_file_prefixes = {"../test_data/benchmark_queries/sigspatial_query",
		"../test_data/benchmark_queries/characters_query",
		"../test_data/benchmark_queries/geolife_query"};

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	// std::vector<double> preprocessing_times;
	std::vector<double> kd_times;
	std::vector<double> greedy_times;
	std::vector<double> simgreedy_times;
	std::vector<double> negative_times;
	std::vector<double> freespace_times;

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		global::times.startPreprocessing();
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.setAlgorithm("light");
		query.getReady();
		global::times.stopPreprocessing();

		for (std::size_t i = 0; i < ks.size(); ++i) {
			auto k = ks[i];

			global::times.startReadingQueryCurve();
			query.readQueryCurves(query_file_prefixes[d] + std::to_string(k) + ".txt");
			global::times.stopReadingQueryCurve();

			query.run();

			// preprocessing_times.push_back(global::times.preprocessing_sum/1000000000.);
			kd_times.push_back(global::times.kd_search_sum/1000000000.);
			greedy_times.push_back(global::times.greedy_sum/1000000000.);
			simgreedy_times.push_back(global::times.simultaneous_greedy_sum/1000000000.);
			negative_times.push_back(global::times.negative_sum/1000000000.);
			freespace_times.push_back(global::times.lessthan_sum/1000000000.);
			global::times.reset();
		}
	}

	// printRow(preprocessing_times);
	printRow(kd_times);
	printRow(greedy_times);
	printRow(simgreedy_times);
	printRow(negative_times);
	printRow(freespace_times);
}

void datasetStatsExp()
{
	std::cout << "Starting dataset stats experiment." << std::endl;

	std::size_t num_data_sets = 3;

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.printDataStats(true);
	}
}

// XXX: we can also do some nice statistics over number of boxes
void boxesVsRuntimeExp()
{
	std::cout << "Starting boxes vs runtime experiment." << std::endl;

	std::size_t num_data_sets = 1;
	Strings query_file_prefixes = {"../test_data/benchmark_queries/sigspatial_query",
		"../test_data/benchmark_queries/characters_query",
		"../test_data/benchmark_queries/geolife_query"};

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	std::vector<std::pair<int, double>> data;
	for (std::size_t d = 0; d < num_data_sets; ++d) {
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.setAlgorithm("light");
		query.getReady();

		for (std::size_t i = 0; i < ks.size(); ++i) {
			auto k = ks[i];

			query.readQueryCurves(query_file_prefixes[d] + std::to_string(k) + ".txt");
			auto hard_instances = query.getHardInstances();

			FrechetLight frechet;
			for (auto const& hard_instance: hard_instances) {
				frechet.clear();
				auto start = hrc::now();
				frechet.lessThan(hard_instance.distance, hard_instance.curve1, hard_instance.curve2);
				auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();

				auto num_boxes = frechet.getNumberOfBoxes();

				data.emplace_back(num_boxes, time/1000.);
			}
		}
	}

	printPairs(data);
}

// FIXME: Make sure that times is used when executing this.
void certificatesRuntime()
{
	std::cout << "Starting certificates runtime experiment." << std::endl;

	std::size_t num_data_sets = 3;
	Strings query_file_prefixes = {"../test_data/benchmark_queries/sigspatial_query",
		"../test_data/benchmark_queries/characters_query",
		"../test_data/benchmark_queries/geolife_query"};

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	std::vector<double> cert_overall_times;
	std::vector<double> cert_creation_times;
	std::vector<double> cert_check_times;

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.setAlgorithm("light");
		query.getReady();

		for (std::size_t i = 0; i < ks.size(); ++i) {
			auto k = ks[i];

			query.readQueryCurves(query_file_prefixes[d] + std::to_string(k) + ".txt");

			auto start = hrc::now();
			query.run();
			auto cert_overall_time = std::chrono::duration_cast<ns>(hrc::now()-start).count();

			auto cert_creation_time = (double)global::times.certcomp_sum;
			auto cert_check_time = (double)global::times.certcheck_sum;
			cert_overall_times.push_back(cert_overall_time/1000000.);
			cert_creation_times.push_back(cert_creation_time/1000000.);
			cert_check_times.push_back(cert_check_time/1000000.);
			global::times.reset();
		}
	}

	printRow(cert_overall_times, 1);
	printRow(cert_creation_times, 1);
	printRow(cert_check_times, 1);
}

// FIXME: Make sure that times is used when executing this.
void certificatesYesNoRuntime()
{
	std::cout << "Starting certificates runtime yes/no experiment." << std::endl;

	std::size_t num_data_sets = 3;
	Strings query_file_prefixes = {"../test_data/benchmark_queries/sigspatial_query",
		"../test_data/benchmark_queries/characters_query",
		"../test_data/benchmark_queries/geolife_query"};

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	std::vector<double> yes_times;
	std::vector<double> no_times;

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		Query query(curve_directories[d]);
		query.readCurveData(curve_data_files[d]);
		query.setAlgorithm("light");
		query.getReady();

		for (std::size_t i = 0; i < ks.size(); ++i) {
			auto k = ks[i];

			query.readQueryCurves(query_file_prefixes[d] + std::to_string(k) + ".txt");
			query.run();

			auto time_per_yes = (double)global::times.certcompyes_sum/global::times.numYesChecked[Times::COMPLETE];
			auto time_per_no = (double)global::times.certcompno_sum/global::times.numNoChecked[Times::COMPLETE];
			yes_times.push_back(time_per_yes/1000.);
			no_times.push_back(time_per_no/1000.);
			global::times.reset();
		}
	}

	printRow(yes_times, 1);
	printRow(no_times, 1);
}

void omitRulesExp()
{
	std::cout << "Starting omit rules experiment." << std::endl;

	FrechetLight frechet;

	std::size_t num_data_sets = 3;
	std::size_t num_queries = 100;
	std::size_t number_of_runs = 5;

	Strings query_file_prefixes = {"../test_data/decider_benchmark_queries/sigspatial_query_decider",
		"../test_data/decider_benchmark_queries/characters_query_decider",
		"../test_data/decider_benchmark_queries/geolife_query_decider"};
	std::vector<ValueRange<int>> k_ranges = { {1, 15}, {1, 12}, {1, 15} };
	std::vector<ValueRange<int>> l_plus_ranges = { {-10, 3}, {-10, 3}, {-10, 3} };
	std::vector<ValueRange<int>> l_minus_ranges = { {-10, 0}, {-10, 0}, {-10, 0} };

	std::cout << "Leave out one experiment:" << "\n";
	// First do a normal (sequential) run
	std::vector<double> times;
	for (std::size_t d = 0; d < num_data_sets; ++d) {
		double time_sum = 0.;

		for (std::size_t run = 0; run < number_of_runs; ++run) {
			for (auto k: k_ranges[d]) {
				for (auto l: l_plus_ranges[d]) {
					std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
						+ "_" + std::to_string(l) + "_plus.txt";
					auto queries = loadQueries(filename, curve_directories[d], num_queries);

					auto start = hrc::now();
					for (auto const& query: queries) {
						frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
					}
					auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
					time_sum += time/1000000.;
				}
				for (auto l: l_minus_ranges[d]) {
					std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
						+ "_" + std::to_string(l) + "_minus.txt";
					auto queries = loadQueries(filename, curve_directories[d], num_queries);

					auto start = hrc::now();
					for (auto const& query: queries) {
						frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
					}
					auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
					time_sum += time/1000000.;
				}
			}
		}

		times.push_back(time_sum/(double)number_of_runs);
	}

	printRow(times);

	// Then leave out each of the rules once
	for (std::size_t leave_out_index = 0; leave_out_index < 5; ++leave_out_index) {
		times.clear();
		std::array<bool,5> enable = {true, true, true, true, true};
		enable[leave_out_index] = false;

		for (std::size_t d = 0; d < num_data_sets; ++d) {
			double time_sum = 0.;

			frechet.setRules(enable);
			for (std::size_t run = 0; run < number_of_runs; ++run) {
				for (auto k: k_ranges[d]) {
					for (auto l: l_plus_ranges[d]) {
						std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
							+ "_" + std::to_string(l) + "_plus.txt";
						auto queries = loadQueries(filename, curve_directories[d], num_queries);

						auto start = hrc::now();
						for (auto const& query: queries) {
							frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
						}
						auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
						time_sum += time/1000000.;
					}
					for (auto l: l_minus_ranges[d]) {
						std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
							+ "_" + std::to_string(l) + "_minus.txt";
						auto queries = loadQueries(filename, curve_directories[d], num_queries);

						auto start = hrc::now();
						for (auto const& query: queries) {
							frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
						}
						auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
						time_sum += time/1000000.;
					}
				}
			}

			times.push_back(time_sum/(double)number_of_runs);
		}

		printRow(times);
	}

}

template <typename Vec>
void printDeciderTable(Vec const& minus, Vec const& plus)
{
	assert(minus.size() == plus.size());

	for (std::size_t k_index = 0; k_index < minus.size(); ++k_index) {
		auto const& minus_row = minus[k_index];
		auto const& plus_row = plus[k_index];

		for (int i = (int)minus_row.size()-1; i >= 0; --i) {
			std::cout << minus_row[i] << " ";
		}
		for (int i = 0; i < (int)plus_row.size(); ++i) {
			std::cout << plus_row[i] << " ";
		}
		std::cout << "\n";
	}
	std::cout << "\n";
}

void deciderComparisonExp()
{
	std::cout << "Starting decider comparison experiment." << std::endl;

	FrechetLight frechet;

	std::size_t num_data_sets = 3;
	std::size_t number_of_runs = 5;

	Strings query_file_prefixes = {"../test_data/decider_benchmark_queries/sigspatial_query_decider",
		"../test_data/decider_benchmark_queries/characters_query_decider",
		"../test_data/decider_benchmark_queries/geolife_query_decider"};
	std::vector<ValueRange<int>> k_ranges = { {1, 15}, {1, 12}, {1, 15} };
	std::vector<ValueRange<int>> l_plus_ranges = { {-10, 3}, {-10, 3}, {-10, 3} };
	std::vector<ValueRange<int>> l_minus_ranges = { {-10, 0}, {-10, 0}, {-10, 0} };

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		TimesRows times_plus;
		TimesRows times_minus;

		for (auto k: k_ranges[d]) {
			TimesRow row_plus(l_plus_ranges[d].size(), 0.);
			TimesRow row_minus(l_minus_ranges[d].size(), 0.);

			for (std::size_t run = 0; run < number_of_runs; ++run) {
				std::size_t plus_index = 0;
				for (auto l: l_plus_ranges[d]) {
					std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
						+ "_" + std::to_string(l) + "_plus.txt";
					auto queries = loadQueries(filename, curve_directories[d]);

					auto start = hrc::now();
					for (auto const& query: queries) {
						frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
					}
					auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
					row_plus[plus_index] += time/1000000.;
					++plus_index;
				}
				std::size_t minus_index = 0;
				for (auto l: l_minus_ranges[d]) {
					std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
						+ "_" + std::to_string(l) + "_minus.txt";
					auto queries = loadQueries(filename, curve_directories[d]);

					auto start = hrc::now();
					for (auto const& query: queries) {
						frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
					}
					auto time = std::chrono::duration_cast<ns>(hrc::now()-start).count();
					row_minus[minus_index] += time/1000000.;
					++minus_index;
				}
			}

			for (auto& element: row_plus) { element /= (double)number_of_runs; }
			for (auto& element: row_minus) { element /= (double)number_of_runs; }

			times_plus.push_back(std::move(row_plus));
			times_minus.push_back(std::move(row_minus));
		}

		printDeciderTable(times_minus, times_plus);
	}
}

void deciderCountFiltered()
{
	std::cout << "Starting counting filtered queries." << std::endl;

	FrechetLight frechet;

	std::size_t num_data_sets = 3;

	Strings query_file_prefixes = {"../test_data/decider_benchmark_queries/sigspatial_query_decider",
		"../test_data/decider_benchmark_queries/characters_query_decider",
		"../test_data/decider_benchmark_queries/geolife_query_decider"};
	std::vector<ValueRange<int>> k_ranges = { {1, 15}, {1, 12}, {1, 15} };
	std::vector<ValueRange<int>> l_plus_ranges = { {-10, 3}, {-10, 3}, {-10, 3} };
	std::vector<ValueRange<int>> l_minus_ranges = { {-10, 0}, {-10, 0}, {-10, 0} };

	for (std::size_t d = 0; d < num_data_sets; ++d) {
		TimesRows counts_plus;
		TimesRows counts_minus;

		for (auto k: k_ranges[d]) {
			TimesRow row_plus(l_plus_ranges[d].size(), 0.);
			TimesRow row_minus(l_minus_ranges[d].size(), 0.);

			std::size_t plus_index = 0;
			for (auto l: l_plus_ranges[d]) {
				std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
					+ "_" + std::to_string(l) + "_plus.txt";
				auto queries = loadQueries(filename, curve_directories[d]);

				frechet.non_filtered = 0;
				for (auto const& query: queries) {
					frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
				}
				row_plus[plus_index] = (double)(queries.size() - frechet.non_filtered)/queries.size();
				++plus_index;
			}

			std::size_t minus_index = 0;
			for (auto l: l_minus_ranges[d]) {
				std::string filename = query_file_prefixes[d] + "_" + std::to_string(k)
					+ "_" + std::to_string(l) + "_minus.txt";
				auto queries = loadQueries(filename, curve_directories[d]);

				frechet.non_filtered = 0;
				for (auto const& query: queries) {
					frechet.lessThanWithFilters(query.distance, query.curve1, query.curve2);
				}
				row_minus[minus_index] = (double)(queries.size() - frechet.non_filtered)/queries.size();
				++minus_index;
			}

			counts_plus.push_back(std::move(row_plus));
			counts_minus.push_back(std::move(row_minus));
		}

		printDeciderTable(counts_minus, counts_plus);
	}
}
