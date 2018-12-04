#include "defs.h"
#include "query.h"
#include "times.h"

#include <string>
#include <vector>

void printUsage()
{
	std::cout <<
		"Usage: ./frechet <curve_directory> <curve_data_file> <query_file_prefix> <alg_string>\n"
		"With <alg_string> you choose the algorithm to be used (light, naive)."
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc != 5) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_directory(argv[1]);
	std::string curve_data_file(argv[2]);
	std::string query_file_prefix(argv[3]);
	std::string frechet_version = argv[4];

	// The vector their_hashes contains modified hashes of the old implementation
	// due to numerical instabilities in the old implementation.
	std::vector<int> ks = {0, 1, 10, 100, 1000};

	//global::times.startPreprocessing();
	Query query(curve_directory);
	query.readCurveData(curve_data_file);
	query.setAlgorithm(frechet_version);
	query.getReady();
	//global::times.stopPreprocessing();

	for (std::size_t i = 0; i < ks.size(); ++i) {
		auto k = ks[i];

		std::cout << "\nStarting queries for k=" << k << ".\n";

		global::times.startReadingQueryCurve();
		query.readQueryCurves(query_file_prefix + std::to_string(k) + ".txt");
		global::times.stopReadingQueryCurve();

		query.run();
		// query.run_parallel();

		std::cout << "\nTime measurements:\n";
		std::cout << "==================\n";
		std::cout << global::times;
		global::times.reset();
	}
}
