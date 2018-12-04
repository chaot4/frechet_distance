#include "parser.h"

#include "defs.h"

#include <sstream>

namespace parser
{

Curve readCurve(std::string filename)
{
	std::ifstream file(filename);
	if (!file.is_open()) {
		ERROR("Could not open curve file " << filename);
	}

	Curve curve;
	readCurve(file, curve);

	return curve;
}

void readCurve(std::ifstream& curve_file, Curve& curve)
{
	// Read everything into a stringstream.
	std::stringstream ss;
	ss << curve_file.rdbuf();

	auto ignore_count = std::numeric_limits<std::streamsize>::max();

	std::string x_str, y_str;
	while (ss >> x_str >> y_str) {
		distance_t x, y;
		x = std::stod(x_str);
		y = std::stod(y_str);

		ss.ignore(ignore_count, '\n');
		// ignore duplicate rows
		if (curve.size() && curve.back().x == x && curve.back().y == y) {
			continue;
		}
		curve.push_back({x, y});
	}
}

} // namespace parser
