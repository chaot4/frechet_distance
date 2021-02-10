#include "defs.h"
#include "frechet_light.h"
#include "frechet_naive.h"
#include "freespace_light_vis.h"
#include "parser.h"
#include "filter.h"

#include <string>

void printUsage()
{
	std::cout <<
		"Usage: ./test_curves <curve_file1> <curve_file2> <distance> <alg_string> [<out_svg_file>]\n"
		"\n"
		"The fourth argument is optional. If only three arguments are passed, then\n"
		"the svg visualization is written to compare_curves_result.svg. More information\n"
		"regarding the format of the curve files can be found in README.\n"
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc <= 3 || argc >= 7) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_file1(argv[1]);
	std::string curve_file2(argv[2]);
	distance_t distance(std::stod(argv[3]));
	std::string frechet_version = (argc >= 5 ? argv[4] : "light");
	std::string vis_file = (argc == 6 ? argv[5] : "compare_curves_result.svg");

	auto curve1 = parser::readCurve(curve_file1);
	auto curve2 = parser::readCurve(curve_file2);
	
	// std::cout << std::setprecision(20) << distance*distance << "\n";
	// std::cout << std::setprecision(20) << curve1[0].dist_sqr(curve2[0]) << "\n";
	// std::cout << std::setprecision(20) << curve1[0] << " " << curve2[0] << "\n";
	// std::cout << std::setprecision(20) << curve1[0].x - curve2[0].x << " " << curve1[0].y - curve2[0].y << "\n";
	// std::cout << std::setprecision(20) << std::pow(curve1[0].x - curve2[0].x,2) << " " << std::pow(curve1[0].y - curve2[0].y,2) << "\n";
	

	if (frechet_version == "light") {
		FrechetLight frechet;
		std::cout << (frechet.lessThan(distance, curve1, curve2) ? "LESS" : "GREATER") << "\n";
		Certificate c = frechet.computeCertificate();
		assert(c.check());
		FreespaceLightVis vis(frechet);
		vis.exportToSvg(vis_file);
	}
	else if (frechet_version == "naive") {
		FrechetNaive frechet;
		std::cout << (frechet.lessThan(distance, curve1, curve2) ? "LESS" : "GREATER") << "\n";
	}
	else if (frechet_version == "greedy") {
		Filter filter(curve1, curve2, distance);
		std::cout << (filter.greedy() ? "LESS" : "NOT CLEAR") << "\n";
	}
	else if (frechet_version == "adaptiveGreedy") {
		Filter filter(curve1, curve2, distance);
		PointID pos1;
		PointID pos2;
		std::cout << (filter.adaptiveGreedy(pos1, pos2) ? "LESS" : "NOT CLEAR") << "\n";
	}
	else if (frechet_version == "adaptiveSimultaneousGreedy") {
		Filter filter(curve1, curve2, distance);
		std::cout << (filter.adaptiveSimultaneousGreedy() ? "LESS" : "NOT CLEAR") << "\n";
	}
	else if (frechet_version == "negative") {
		Filter filter(curve1, curve2, distance);
		PointID pos1;
		PointID pos2;
		filter.adaptiveGreedy(pos1, pos2);
		std::cout << (filter.negative(pos1, pos2) ? "GREATER" : "NOT CLEAR") << "\n";
	}
	else {
		ERROR("Unknown Frechet version: " << frechet_version << "\n"
		      "Known Frechet versions: light, naive");
	}
}
