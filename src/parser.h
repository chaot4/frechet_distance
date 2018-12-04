#include "geometry_basics.h"
#include "curves.h"

#include <fstream>
#include <string>

namespace parser
{

Curve readCurve(std::string filename);
void readCurve(std::ifstream& curve_file, Curve& curve);

} // namespace parser
