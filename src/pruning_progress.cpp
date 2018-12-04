#include "defs.h"
#include "frechet_light.h"
#include "freespace_light_vis.h"
#include "parser.h"
#include "filter.h"

#include <string>

void printUsage()
{
	std::cout <<
		"Usage: ./pruning_progress <curve_file1> <curve_file2> <distance> [<out_svg_file_prefix>]\n"
		"\n"
		"The fourth argument is optional. If only three arguments are passed, then\n"
		"the svg visualization is written to pruning_progress<i>.svg. More information\n"
		"regarding the format of the curve files can be found in README.\n"
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc <= 3 || argc >= 6) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_file1(argv[1]);
	std::string curve_file2(argv[2]);
	distance_t distance(std::stod(argv[3]));
	std::string vis_file_prefix = (argc == 5 ? argv[4] : "pruning_progress");

	auto curve1 = parser::readCurve(curve_file1);
	auto curve2 = parser::readCurve(curve_file2);
	
	FrechetLight frechet;
	for (int pruning_level = 0; pruning_level < 7; ++pruning_level) {
		frechet.setPruningLevel(pruning_level);
		frechet.lessThan(distance, curve1, curve2);
		FreespaceLightVis vis(frechet);
		vis.exportToSvg(vis_file_prefix + std::to_string(pruning_level) + ".svg");

		// Write free-space diagram. This is sort of a dumb hack...
		if (pruning_level == 0) {
			vis.exportFreespaceToSvg(vis_file_prefix + "_freespace.svg");
		}
	}
}
