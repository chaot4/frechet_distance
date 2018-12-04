#include "defs.h"
#include "frechet_light.h"
#include "freespace_light_vis.h"
#include "parser.h"
#include "filter.h"

#include <string>

void printUsage()
{
	std::cout <<
		"Usage: ./export_freespace_diagram <curve_file1> <curve_file2> <distance> [<out_file>]\n"
		"\n"
		"The fourth argument is optional. If only three arguments are passed, then\n"
		"the svg visualization is written to freespace_diagram.svg. More information\n"
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
	std::string vis_file_prefix = (argc == 5 ? argv[4] : "freespace_diagram");

	auto curve1 = parser::readCurve(curve_file1);
	auto curve2 = parser::readCurve(curve_file2);
	
	FrechetLight frechet;
	frechet.lessThan(distance, curve1, curve2); // this is still a hack...
	FreespaceLightVis vis(frechet);
	vis.exportFreespaceToSvg(vis_file_prefix + ".svg");
}
