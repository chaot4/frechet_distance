#include "freespace_light_vis.h"

#include "defs.h"

FreespaceLightVis::FreespaceLightVis(FrechetLight const& frechet)
	: frechet(frechet)
{
	auto curve_pair = frechet.getCurvePair();
	int n = curve_pair[0]->size();
	int m = curve_pair[1]->size();
	auto max = std::max(n, m);

	cell_width = (max/10 + 1) * 4.;
	point_radius = (max/10 + 1) * 0.5;
	line_thickness1 = (max/10 + 1) * 1.;
	line_thickness2 = (max/10 + 1) * 0.75;
	line_thickness3 = (max/10 + 1) * 0.75;
	delta = line_thickness1;
	page_padding = 2 * cell_width;
	diagram_curves_padding = 2 * page_padding;
}

void FreespaceLightVis::exportToSvg(std::string const& filename)
{
	setWidthAndHeight();

	std::ofstream f(filename);
	if (!f.is_open()) {
		ERROR("The filename passed to exportToSvG cannot be opened.");
		return;
	}

	writeHeader(f);
	if (draw_diagram) {
		writeCells(f);
		writeEmptyIntervals(f);
		writeReachLists(f);
		writeUnknownIntervals(f);
		writeConnections(f);
		writeFreeNonReachable(f);
		writeCertificate(f);
	}
	if (draw_curves) {
		writeCurves(f);
	}
	writeFooter(f);
}

void FreespaceLightVis::exportFreespaceToSvg(std::string const& filename)
{
	setWidthAndHeight();

	std::ofstream f(filename);
	if (!f.is_open()) {
		ERROR("The filename passed to exportToSvG cannot be opened.");
		return;
	}

	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];

	auto writeFreespaceBoundary = [&](PointID min, PointID max, PointID fixed, CurveID fixed_curve) {
		// compute interval
		auto fixed_point = fixed_curve == 0 ? curve1[fixed] : curve2[fixed];
		auto min_point = fixed_curve == 0 ? curve2[min] : curve1[min];
		auto max_point = fixed_curve == 0 ? curve2[max] : curve1[max];
		auto interval = IntersectionAlgorithm::intersection_interval(fixed_point, frechet.distance, min_point, max_point);

		auto min_cpoint = CPoint{min, 0.};
		auto max_cpoint = CPoint{max, 0.};
		auto fixed_cpoint = CPoint{fixed, 0.};
		auto begin_cpoint = CPoint{min, interval.begin};
		auto end_cpoint = CPoint{min, interval.end};

		if (interval.is_empty()) {
			auto cinterval = CInterval{min_cpoint, max_cpoint, fixed_cpoint, fixed_curve};
			writeInterval(f, cinterval, Color::Nonfree);
		}
		else {
			// write first empty interval
			if (interval.begin != 0.) {
				auto cinterval =
					CInterval{min_cpoint, begin_cpoint, fixed_cpoint, fixed_curve};
				writeInterval(f, cinterval, Color::Nonfree);
			}
			// write free interval
			if (!interval.is_empty()) {
				auto cinterval =
					CInterval{begin_cpoint, end_cpoint, fixed_cpoint, fixed_curve};
				writeInterval(f, cinterval, Color::Free);
			}
			// write second empty interval
			if (interval.end < 1.) {
				auto cinterval =
					CInterval{end_cpoint, max_cpoint, fixed_cpoint, fixed_curve};
				writeInterval(f, cinterval, Color::Nonfree);
			}
		}
	};

	writeHeader(f);
	if (draw_diagram) {
		// write innner cells
		for (PointID i = 0; i < curve1.size()-1; ++i) {
			for (PointID j = 0; j < curve2.size()-1; ++j) {
				auto ellipse = segmentsToEllipse(curve1[i], curve1[i+1],
												 curve2[j], curve2[j+1],
												 frechet.distance);

				if (ellipse.is_valid()) {
					writeCell(i, j, ellipse, f);
				}
				else {
					writePolygonCell(i, j, f);
				}
			}
		}

		// write boundaries
		for (PointID i = 0; i < curve1.size()-1; ++i) {
			for (PointID j = 0; j < curve2.size()-1; ++j) {
				writeFreespaceBoundary(j, j+1, i, 0); // left
				writeFreespaceBoundary(i, i+1, j, 1); // bottom
				if (i == curve1.size()-2) {
					writeFreespaceBoundary(j, j+1, i+1, 0); // right
				}
				if (j == curve2.size()-2) {
					writeFreespaceBoundary(i, i+1, j+1, 1); // top
				}
			}
		}
	}
	if (draw_curves) {
		writeCurves(f);
	}
	writeFooter(f);
}

void FreespaceLightVis::setWidthAndHeight()
{
	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];
	auto n = curve1.size();
	auto m = curve2.size();

	width = 0;
	height = 0;
	diagram_width = 0;
	diagram_height = 0;
	curve_width = 0;
	curve_height = 0;

	if (draw_diagram) {
		diagram_width = n*line_thickness1 + (n-1)*cell_width;
		diagram_height = m*line_thickness1 + (m-1)*cell_width;
	}
	
	if (diagram_height == 0) { diagram_height = 10*page_padding; }

	if (draw_curves) {
		Length curve1_width = curve1.getExtremePoints().max_x - curve1.getExtremePoints().min_x;
		Length curve1_height = curve1.getExtremePoints().max_y - curve1.getExtremePoints().min_y;
		Length curve2_width = curve2.getExtremePoints().max_x - curve2.getExtremePoints().min_x;
		Length curve2_height = curve2.getExtremePoints().max_y - curve2.getExtremePoints().min_y;
		Length max_width = std::max(curve1_width, curve2_width);
		Length max_height = std::max(curve1_height, curve2_height);

		curve_width = diagram_height * max_width/max_height;
		curve_height = diagram_height;
	}

	width = diagram_width + (draw_curves && draw_diagram)*diagram_curves_padding + curve_width;
	height = diagram_height;
}

void FreespaceLightVis::writeHeader(std::ofstream& f)
{
	f << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
	  << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\""
	  << -page_padding << " " << -page_padding << " "
	  << width + 2*page_padding << " " << height + 2*page_padding << "\">\n"
	  << "<rect x=\"" << -page_padding << "\" y=\"" << -page_padding << "\" "
	  << "width=\"" << width + 2*page_padding << "\" height=\"" << height + 2*page_padding
	  << "\" fill=\"white\"/>\n";
}

void FreespaceLightVis::writeEmptyIntervals(std::ofstream& f)
{
	for (auto const& cinterval : frechet.empty_intervals) {
		writeInterval(f, cinterval, Color::Nonfree);
	}
}

void FreespaceLightVis::writeReachLists(std::ofstream& f)
{
	for (auto const& cinterval : frechet.reachable_intervals) {
		writeInterval(f, cinterval, Color::Reachable);
	}
}

void FreespaceLightVis::writeUnknownIntervals(std::ofstream& f)
{
	for (auto const& cinterval : frechet.unknown_intervals) {
		writeInterval(f, cinterval, Color::Undecided);
	}
}

void FreespaceLightVis::writeConnections(std::ofstream& f)
{
	for (auto const& cinterval : frechet.connections) {
		writeInterval(f, cinterval, Color::Reachable, true);
	}
}

void FreespaceLightVis::writeFreeNonReachable(std::ofstream& f)
{
	for (auto const& cinterval : frechet.free_non_reachable) {
		writeInterval(f, cinterval, Color::Free);
	}
}

void FreespaceLightVis::writeCells(std::ofstream& f)
{
	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];

	for (auto const& cell: frechet.getCells()) {
		// calculate ellipse
		auto ellipse = segmentsToEllipse(curve1[cell.i], curve1[cell.i+1],
		                                 curve2[cell.j], curve2[cell.j+1],
		                                 frechet.distance);

		if (ellipse.is_valid()) {
			writeCell(cell.i, cell.j, ellipse, f);
		}
		else {
			writePolygonCell(cell.i, cell.j, f);
		}
	}
}

void FreespaceLightVis::writeCell(PointID i, PointID j, Ellipse const& e, std::ofstream& f)
{
	Length x = line_thickness1 + i*(cell_width + line_thickness1);
	Length y = (j+1)*(cell_width + line_thickness1);
	Length y_e = line_thickness1 + j*(cell_width + line_thickness1);

	auto clip_name = "clip-" + std::to_string(i) + "-" + std::to_string(j);

	auto cx = x + (1 - e.center.x) * cell_width;
	auto cy = y_e + (1 - e.center.y) * cell_width;
	auto rx = e.width * cell_width;
	auto ry = e.height * cell_width;

	// compensate for svg coordinates
	y = height - y;
	cy = height - cy;

	f <<
	"<defs>\n"
	"<clipPath id=\"" << clip_name << "\">\n"
	"<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << cell_width << "\" height=\"" << cell_width << "\"/>\n"
	"</clipPath>\n"
	"</defs>\n"
	"<g id=\"sites\" clip-path=\"url(#" << clip_name << ")\">\n"
	"<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << cell_width << "\" height=\"" << cell_width << "\" fill=\"" << toColorString(Color::EmptyCell) << "\"/>\n"
	"<ellipse cx=\"" << cx << "\" cy=\"" << cy << "\" rx=\"" << rx << "\" ry=\"" << ry << "\" transform=\"rotate(" << -e.alpha << " " << cx << " " << cy <<")\" fill=\"" << toColorString(Color::FreeCell) << "\"/>\n"
	"</g>\n";
}


void FreespaceLightVis::writePolygonCell(PointID i, PointID j, std::ofstream& f)
{
	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];

	Length x = line_thickness1 + i*(cell_width + line_thickness1);
	Length y = height - (j+1)*(cell_width + line_thickness1);

	// head
	f << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << cell_width << "\" height=\"" << cell_width << "\" fill=\"" << toColorString(Color::EmptyCell) << "\"/>\n";
	f << "<polygon points=\"";

	// points of the polygon
	auto a1 = curve1[i];
	auto b1 = curve1[i+1];
	auto a2 = curve2[j];
	auto b2 = curve2[j+1];

	auto i_l = IntersectionAlgorithm::intersection_interval(a1, frechet.distance, a2, b2, nullptr);
	auto i_r = IntersectionAlgorithm::intersection_interval(b1, frechet.distance, a2, b2, nullptr);
	auto i_b = IntersectionAlgorithm::intersection_interval(a2, frechet.distance, a1, b1, nullptr);
	auto i_t = IntersectionAlgorithm::intersection_interval(b2, frechet.distance, a1, b1, nullptr);

	if (!i_b.is_empty()) {
		f << x + cell_width*i_b.end << "," << y + cell_width<< " ";
		f << x + cell_width*i_b.begin << "," << y + cell_width << " ";
	}
	if (!i_l.is_empty()) {
		f << x << "," << y + cell_width*(1.-i_l.begin) << " ";
		f << x << "," << y + cell_width*(1.-i_l.end) << " ";
	}
	if (!i_t.is_empty()) {
		f << x + cell_width*i_t.begin << "," << y << " ";
		f << x + cell_width*i_t.end << "," << y << " ";
	}
	if (!i_r.is_empty()) {
		f << x + cell_width << "," << y + cell_width*(1.-i_r.end) << " ";
		f << x + cell_width << "," << y + cell_width*(1.-i_r.begin) << " ";
	}

	// tail
	f << "\" fill=\"" << toColorString(Color::FreeCell) << "\"/>";
}

void FreespaceLightVis::writeCertificate(std::ofstream& f)
{
	const Certificate& cert = frechet.getCertificate();
	if (!cert.isValid()) {
	  return;
	}
	if (cert.isYes()) {
		writeYesCertificate(f);
	} else {
		writeNoCertificate(f);
	}
}

void FreespaceLightVis::writeYesCertificate(std::ofstream& f)
{
	Certificate const& cert = frechet.getCertificate();

	CPositions traversal = cert.getTraversal();
	size_t T = traversal.size();
	for (size_t t = 0; t < T-1; t++) {
		SvgCoordinate start;
		SvgCoordinate end;
		if (traversal[t][0] == traversal[t+1][0]) {
			start = toSvgCoordinate(traversal[t][0], traversal[t][1], Orientation::Vertical);
			end = toSvgCoordinate(traversal[t][0], traversal[t+1][1], Orientation::Vertical);

			if (traversal[t+1][1].getFraction() == 0) { end.y += line_thickness1; }
		} else if (traversal[t][1] == traversal[t+1][1]) {
			start = toSvgCoordinate(traversal[t][0], traversal[t][1], Orientation::Horizontal);
			end = toSvgCoordinate(traversal[t+1][0], traversal[t][1], Orientation::Horizontal);

			if (traversal[t+1][0].getFraction() == 0) { end.x -= line_thickness1; }
		} else {
			start = toSvgCoordinate(traversal[t]);
			end = toSvgCoordinate(traversal[t+1]);
		}
		writeLine(f, {start, end, line_thickness1/2, Color::Default, false});
	}
}

void FreespaceLightVis::writeNoCertificate(std::ofstream& f)
{
	Certificate const& cert = frechet.getCertificate();

	CPositions traversal = cert.getTraversal();
	size_t T = traversal.size();
	for (size_t t = 0; t < T-1; t++) {
		SvgCoordinate start;
		SvgCoordinate end;
		bool dashed = false;
		if (traversal[t+1][0] >= traversal[t][0] and traversal[t+1][1] <= traversal[t][1]) {
			start = toSvgCoordinate(traversal[t]);
			end = toSvgCoordinate(traversal[t+1]);
			dashed = true;
		} else if (traversal[t][0] == traversal[t+1][0]) {
			start = toSvgCoordinate(traversal[t][0], traversal[t][1], Orientation::Vertical);
			end = toSvgCoordinate(traversal[t][0], traversal[t+1][1], Orientation::Vertical);

			if (traversal[t+1][1].getFraction() == 0) { end.y += line_thickness1; }
		} else if (traversal[t][1] == traversal[t+1][1]) {
			start = toSvgCoordinate(traversal[t][0], traversal[t][1], Orientation::Horizontal);
			end = toSvgCoordinate(traversal[t+1][0], traversal[t][1], Orientation::Horizontal);

			if (traversal[t+1][0].getFraction() == 0) { end.x -= line_thickness1; }
		} else {
			ERROR("None of the expected cases was true in writeNoCertificate.");
		}
		writeLine(f, {start, end, line_thickness1/2, Color::Default, dashed});
	}
}

void FreespaceLightVis::writeInterval(std::ofstream& f, CInterval cinterval, Color color, bool dashed)
{
	SvgCoordinate start;
	SvgCoordinate end;

	if (cinterval.fixed_curve == 0) {
		start = toSvgCoordinate(cinterval.fixed, cinterval.begin, Orientation::Vertical);
		end = toSvgCoordinate(cinterval.fixed, cinterval.end, Orientation::Vertical);

		if (cinterval.end.getFraction() == 0) { end.y += line_thickness1; }
	}
	else if (cinterval.fixed_curve == 1) {
		start = toSvgCoordinate(cinterval.begin, cinterval.fixed, Orientation::Horizontal);
		end = toSvgCoordinate(cinterval.end, cinterval.fixed, Orientation::Horizontal);

		if (cinterval.end.getFraction() == 0) { end.x -= line_thickness1; }
	}
	else {
		ERROR("Invalid fixed curve!");
	}

	auto line_thickness = dashed ? line_thickness2 : line_thickness1;
	writeLine(f, {start, end, line_thickness, color, dashed});
}

void FreespaceLightVis::writeCurves(std::ofstream& f)
{
	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];

	// write first curve
	SvgCoordinates svg_coordinates;
	for (std::size_t i = 0; i < curve1.size(); ++i) {
		auto const& point = curve1[i];
		writePoint(f, {toSvgCurvePoint(point), 1.5*point_radius, Color::Curve1});
		svg_coordinates.push_back(toSvgCurvePoint(point));
	}
	writePolyline(f, svg_coordinates, Color::Curve1);
	writePoint(f, {toSvgCurvePoint(curve1[0]), 3.*point_radius, Color::Curve1});

	// write second curve
	svg_coordinates.clear();
	for (std::size_t i = 0; i < curve2.size(); ++i) {
		auto const& point = curve2[i];
		writePoint(f, {toSvgCurvePoint(point), 1.5*point_radius, Color::Curve2});
		svg_coordinates.push_back(toSvgCurvePoint(point));
	}
	writePolyline(f, svg_coordinates, Color::Curve2);
	writePoint(f, {toSvgCurvePoint(curve2[0]), 3.*point_radius, Color::Curve2});
}

void FreespaceLightVis::writeFooter(std::ofstream& f)
{
	f << "\n</svg>";
}

auto FreespaceLightVis::toSvgCurvePoint(Point const& point) -> SvgCoordinate
{
	auto const& curve1 = *frechet.getCurvePair()[0];
	auto const& curve2 = *frechet.getCurvePair()[1];

	auto min_x = std::min(curve1.getExtremePoints().min_x, curve2.getExtremePoints().min_x);
	auto max_x = std::max(curve1.getExtremePoints().max_x, curve2.getExtremePoints().max_x);
	auto min_y = std::min(curve1.getExtremePoints().min_y, curve2.getExtremePoints().min_y);
	auto max_y = std::max(curve1.getExtremePoints().max_y, curve2.getExtremePoints().max_y);

	Length x = (point.x - min_x)/(max_x - min_x)*curve_width;
	Length y = (point.y - min_y)/(max_y - min_y)*curve_height;

	// adjust for origin being in the upper-left corner
	y = curve_height - y;

	if (draw_diagram) {
		x += diagram_width + diagram_curves_padding;
	}

	return {x, y};
}

auto FreespaceLightVis::toSvgCoordinate(CPosition pt)
	-> SvgCoordinate
{
	Length x, y;

	bool not_integral1 = pt[0].getFraction() != 0. && pt[0].getFraction() != 1.;
	bool not_integral2 = pt[1].getFraction() != 0. && pt[1].getFraction() != 1.;

	if (not_integral1) {
		Length x_point = line_thickness1 + pt[0].getPoint()*(cell_width + line_thickness1);
		Length x_frac = pt[0].getFraction()*cell_width;
		x = x_point + x_frac;
	}
	else {
		x = line_thickness1/2 + pt[0].convert()*(cell_width + line_thickness1);
	}

	if (not_integral2) {
		Length y_point = line_thickness1 + pt[1].getPoint()*(cell_width + line_thickness1);
		Length y_frac = pt[1].getFraction()*cell_width;
		y = y_point + y_frac;
	}
	else {
		y = line_thickness1/2 + pt[1].convert()*(cell_width + line_thickness1);
	}

	// adjust for origin being in the upper-left corner
	y = height - y;

	return {x, y};
}



auto FreespaceLightVis::toSvgCoordinate(CPoint point1, CPoint point2, Orientation orientation)
	-> SvgCoordinate
{
	Length x, y;

	bool not_integral1 = point1.getFraction() != 0. && point1.getFraction() != 1.;
	bool not_integral2 = point2.getFraction() != 0. && point2.getFraction() != 1.;

	// y
	if (orientation == Orientation::Vertical || not_integral2) {
		Length y_point = line_thickness1 + point2.getPoint()*(cell_width + line_thickness1);
		Length y_frac = point2.getFraction()*cell_width;
		y = y_point + y_frac;
	}
	else {
		y = line_thickness1/2 + point2.convert()*(cell_width + line_thickness1);
	}

	// x
	if (orientation == Orientation::Horizontal || not_integral1) {
		Length x_point = line_thickness1 + point1.getPoint()*(cell_width + line_thickness1);
		Length x_frac = point1.getFraction()*cell_width;
		x = x_point + x_frac;
	}
	else {
		x = line_thickness1/2 + point1.convert()*(cell_width + line_thickness1);
	}

	// adjust for origin being in the upper-left corner
	y = height - y;

	return {x, y};
}

void FreespaceLightVis::writePoint(std::ofstream& f, SvgPoint point)
{
	f << "<circle cx=\"" << point.coord.x << "\" cy=\"" << point.coord.y
	  << "\" r=\"" << point.radius << "\" stroke=\"" << toColorString(point.color)
	  << "\" fill-opacity=\"0.0\"/>\n" ;
}

void FreespaceLightVis::writeLine(std::ofstream& f, SvgLine line)
{
	auto stroke_string = line.dashed ? " stroke-dasharray=\"2,5\"" : "";

	f << "<line x1=\"" << line.p1.x << "\" y1=\"" << line.p1.y
	  << "\" x2=\"" << line.p2.x << "\" y2=\"" << line.p2.y
	  << "\" stroke=\"" << toColorString(line.color) << "\" stroke-width=\""
	  << line.thickness << "\"" << stroke_string << "/>\n";
}

void FreespaceLightVis::writePolyline(std::ofstream& f, SvgCoordinates const& svg_coordinates, Color color)
{
	f << "<polyline points=\"";
	for (auto const& svg_coordinate: svg_coordinates) {
		f << svg_coordinate.x << "," << svg_coordinate.y << " ";
	}
	f << "\" style=\"fill:none;stroke:" << toColorString(color)
	  << ";stroke-width:" << line_thickness3 << "\"/>\n";
}

void FreespaceLightVis::writeRectangle(std::ofstream& f, SvgRect rect)
{
	f << "<rect x=\"" << rect.p.x << "\" y=\"" << rect.p.y << "\" width=\"" << rect.width
	  << "\" height=\"" << rect.height << "\" stroke-width=\"" << rect.thickness
	  << "\" stroke=\"" << toColorString(rect.color) << "\" fill=\""
	  << toColorString(rect.color) << "\"/>\n";
}

std::string FreespaceLightVis::toColorString(Color color) const
{
	switch (color) {
		case Color::Nonfree:
		  	return "red";
		case Color::Free:
			return "green";
		case Color::Inactive:
			return "snow";
		case Color::Undecided:
			return "grey";
		case Color::Curve1:
			return "black";
		case Color::EmptyCell:
			return "#FF7F7F";
		case Color::Curve2:
			return "red";
		case Color::Reachable:
			return "blue";
		case Color::FreeCell:
			return "#7FBF3F";
		case Color::Active:
		case Color::Default:
		default:
			return "black";
	}
}
