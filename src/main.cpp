#include "defs.h"
#include "query.h"

#include <string>

void printUsage()
{
	std::cout <<
		"Usage: ./frechet <curve_directory> <curve_data_file> <query_curves_file> [<results_file>]\n"
		"\n"
		"The fourth argument is optional. If only three arguments are passed, then\n"
		"the results are written to results.txt. More information regarding the\n"
		"format of the curve and query files can be found in README.\n"
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc <= 3 || argc >= 6) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_directory(argv[1]);
	std::string curve_data_file(argv[2]);
	std::string query_curves_file(argv[3]);
	std::string results_file = (argc == 5 ? argv[4] : "results.txt");

	// make everything ready for query
	Query query(curve_directory);
	query.readCurveData(curve_data_file);
	query.readQueryCurves(query_curves_file);
	query.setAlgorithm("light");
	query.getReady();

	// run and save result
	query.run();
	query.saveResults(results_file);
}
