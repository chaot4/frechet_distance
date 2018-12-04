#include "defs.h"
#include "query.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void printUsage()
{
	std::cout <<
		"Usage: ./frechet <curve_directory> <curve_data_file> <query_file_prefix> <result_file_prefix> <alg_string> <?performance_test>\n"
		"With <alg_string> you choose the algorithm to be used (normal, light, naive)."
		"\n";
}

Results readResults(std::string const& results_file)
{
	Results results;

	std::ifstream file(results_file);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			std::stringstream ss(line);

			results.emplace_back();
			auto& result = results.back();

			CurveID curve_id;
			while (ss >> curve_id) {
				result.curve_ids.push_back(curve_id);
			}
		}
	}
	else {
		ERROR("The results file could not be opened: " << results_file);
	}

	return results;
}

void compareResults(Results const& query_results, Results const& known_results,
                    Query const& query)
{
	if (query_results.size() != known_results.size()) {
		ERROR("Number of results does not match. This shouldn't happen...");
	}

	for (std::size_t i = 0; i < query_results.size(); ++i) {
		auto query_result = query_results[i].curve_ids;
		auto known_result = known_results[i].curve_ids;

		std::sort(query_result.begin(), query_result.end());
		std::sort(known_result.begin(), known_result.end());

		CurveIDs missing;
		std::set_difference(known_result.begin(), known_result.end(),
							query_result.begin(), query_result.end(),
							std::inserter(missing, missing.begin()));

		CurveIDs additional;
		std::set_difference(query_result.begin(), query_result.end(),
							known_result.begin(), known_result.end(),
							std::inserter(additional, additional.begin()));

		if (!missing.empty() || !additional.empty()) {
			std::cout << "================================\n";
			std::cout << "Non-matching query result found!\n";
			query.printQueryInformation(i);

			std::cout << "Missing curves: ";
			for (auto m: missing) { std::cout << query.getCurve(m).filename << " "; }
			std::cout << "\n";

			std::cout << "Additional curves: ";
			for (auto a: additional) { std::cout << query.getCurve(a).filename << " "; }
			std::cout << "\n";

			std::cout << "================================\n";
			ERROR("exiting...");
		}
	}
}

int main(int argc, char* argv[])
{
	if (argc != 6 and argc != 7) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}
	bool output_performance = (argc != 6);

	std::string curve_directory(argv[1]);
	std::string curve_data_file(argv[2]);
	std::string query_file_prefix(argv[3]);
	std::string result_file_prefix(argv[4]);
	std::string frechet_version = argv[5];

	std::vector<int> ks = {0, 1, 10, 100, 1000};

	Query query(curve_directory);
	query.readCurveData(curve_data_file);
	query.setAlgorithm(frechet_version);
	query.getReady();

	for (std::size_t i = 0; i < ks.size(); ++i) {
		auto k = ks[i];

		std::cout << "Starting queries for k=" << k << ".\n";

		global::times.startReadingQueryCurve();
		query.readQueryCurves(query_file_prefix + std::to_string(k) + ".txt");
		global::times.stopReadingQueryCurve();
		query.run();
		// query.run_parallel();

		auto results = readResults(result_file_prefix + std::to_string(k) + ".txt");
		compareResults(query.getResults(), results, query);

		if (output_performance) {
			std::cout << "\nTime measurements:\n";
			std::cout << "==================\n";
			std::cout << global::times;
			global::times.reset();
		}
	}
	if (global::times.numInvalid[Times::FILTER] + global::times.numInvalid[Times::COMPLETE] > 0) {
		std::cout << "There were " << global::times.numInvalid[Times::FILTER] + global::times.numInvalid[Times::COMPLETE] << " many invalid certificates!" << std::endl;
	}

	std::cout << "All hashes match!\n";

}
