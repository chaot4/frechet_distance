#include "defs.h"
#include "frechet_light.h"
#include "freespace_light_vis.h"
#include "parser.h"

#include <string>

void printUsage()
{
	std::cout <<
		"Usage: ./test_curves <curve_file1> <curve_file2> [<out_svg_file>]\n"
		"\n"
		"The third is optional. If only two arguments are passed, then no svg file\n"
		"is exported. More information regarding the format of the curve files can\n"
		"be found in README.\n"
		"\n";
}

int main(int argc, char* argv[])
{
	if (argc <= 2 || argc >= 5) {
		printUsage();
		ERROR("Wrong number of arguments passed.");
	}

	std::string curve_file1(argv[1]);
	std::string curve_file2(argv[2]);
	std::string vis_file = (argc == 4 ? argv[3] : "");

	auto curve1 = parser::readCurve(curve_file1);
	auto curve2 = parser::readCurve(curve_file2);

	FrechetLight frechet;
	auto distance = frechet.calcDistance(curve1, curve2);
	std::cout << "The Fréchet distance is: " << std::setprecision(20) << distance << "\n";

	if (!vis_file.empty()) {
		FreespaceLightVis vis(frechet);
		vis.exportToSvg(vis_file);
	}
}
