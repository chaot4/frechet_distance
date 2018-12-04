#include "defs.h"
#include "query.h"

#include <fstream>
#include <limits>
#include <random>

void printUsage()
{
	std::cout <<
		"Usage: ./create_benchmark <curve_data_file> <curve_directory> <out_prefix> <epsilon>\n"
		"\n"
		"The epsilon gives the size of the ball around the query distance where the"
		"result size remains the same. This is to avoid issues with rounding errors"
		"which might be different in different implementations. (default: 0.0000001)"
		"\n";
}

Curve const& getRandomCurve(Query const& query)
{
	// FIXME: This initializes with 0. Use better seed after debugging.
	static std::default_random_engine gen;

	auto const& curves = query.getCurves();
	std::uniform_int_distribution<std::size_t> distribution(0, curves.size()-1);
	return curves[distribution(gen)];
}

int main(int argc, char* argv[])
{
	using QueryPair = std::pair<Curve const&, distance_t>;

	if (argc != 4 && argc != 5) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_data_file(argv[1]);
	std::string curve_directory(argv[2]);
	std::string out_prefix = argv[3];
	distance_t epsilon = (argc == 5 ? std::stod(argv[4]) : 0.0000001);

	std::vector<std::size_t> ks = {0, 1, 10, 100, 1000};

	Query query(curve_directory);
	query.readCurveData(curve_data_file);
	query.getReady();

	for (std::size_t i = 0; i < ks.size(); ++i) {
		auto k = ks[i];

		std::cout << "Creating benchmark for k=" << k << ".\n";
		query.setAlgorithm("light");
		std::vector<QueryPair> query_pairs;
		unsigned int epsilon_ball_rejects = 0;
		while (query_pairs.size() < 1000) {

			Curve const& query_curve = getRandomCurve(query);
			distance_t lower_bound = 0.;
			distance_t upper_bound = query.getUpperBoundDistance();
			distance_t query_distance;

			bool distance_found = false;
			int rounds = 0;
			int const rounds_max = 100;
			while (!distance_found && rounds < rounds_max) {
				query_distance = lower_bound + (upper_bound - lower_bound)/2.;
				query.run(query_curve, query_distance);

				auto result_size = query.getResults()[0].curve_ids.size();
				if (result_size > k+1) {
					upper_bound = query_distance;
				}
				else if (result_size < k+1) {
					lower_bound = query_distance;
				}
				else {
					// Check that there is an epsilon ball around this distance
					// which also has the same result size. If this is not the
					// case then we just drop this random curve.
					query.run(query_curve, query_distance - epsilon);
					auto result_size_minus = query.getResults()[0].curve_ids.size();
					query.run(query_curve, query_distance + epsilon);
					auto result_size_plus = query.getResults()[0].curve_ids.size();

					if (result_size_minus == result_size_plus) {
						distance_found = true;
					}
					else {
						++epsilon_ball_rejects;
						break;
					}
				}

				++rounds;
			}

			if (distance_found) {
				query_pairs.emplace_back(query_curve, query_distance);
				std::cout << "." << std::flush;
			}

			// XXX: avoids allocating huge amounts of memory just for the timing data
			global::times.reset();
		}
		std::cout << "\n";

		std::cout << "There were " << epsilon_ball_rejects << " rejects.\n";

		std::cout << "Verifying..." << "\n";
		query.setAlgorithm("naive");
		for (auto const& query_pair: query_pairs) {
			query.run(query_pair.first, query_pair.second);
			assert(query.getResults()[0].curve_ids.size() == k+1);
		}

		std::cout << "Export to file..." << "\n";
		std::ofstream file(out_prefix + std::to_string(k) + ".txt");
		if (file.is_open()) {
			file << std::setprecision(20);
			for (auto const& query_pair: query_pairs) {
				file << query_pair.first.filename << " " << query_pair.second << "\n";
			}
		}
	}
}
