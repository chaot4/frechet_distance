#pragma once

#include <fstream>

#include "frechet_light.h"

namespace unit_tests { void testFreespaceLightVis(); }

class FreespaceLightVis
{
	using Length = double;

	// Those sizes are set in the constructor depending on curve length
	Length cell_width;
	Length point_radius;
	Length line_thickness1, line_thickness2, line_thickness3;
	Length delta;
	Length page_padding;
	Length diagram_curves_padding;

	Length width, height;
	Length diagram_width, diagram_height;
	Length curve_width, curve_height;

	bool draw_diagram = true;
	bool draw_curves = true;

	enum class Color { Default, Active, Inactive, Free, Nonfree, Undecided, Curve1, Curve2, Reachable, EmptyCell, FreeCell};
	struct SvgCoordinate {
		Length x;
		Length y;

		SvgCoordinate operator+(SvgCoordinate const& other) { return {x + other.x, y + other.y}; }
		SvgCoordinate operator-(SvgCoordinate const& other) { return {x - other.x, y - other.y}; }
		SvgCoordinate operator*(Length divisor) { return {x*divisor, y*divisor}; }
		SvgCoordinate operator/(Length divisor) { return {x/divisor, y/divisor}; }
	};
	using SvgCoordinates = std::vector<SvgCoordinate>;
	struct SvgPoint {
		SvgCoordinate coord;
		Length radius;
		Color color;

		SvgPoint(SvgCoordinate coord, Length radius, Color color = Color::Default)
			: coord(coord), radius(radius), color(color) {}
	};
	struct SvgLine {
		SvgCoordinate p1;
		SvgCoordinate p2;
		Length thickness;
		Color color;
		bool dashed;

		SvgLine(SvgCoordinate p1, SvgCoordinate p2, Length thickness, Color color = Color::Default, bool dashed = false)
			: p1(p1), p2(p2), thickness(thickness), color(color), dashed(dashed) {}
	};
	struct SvgRect {
		SvgCoordinate p;
		Length width;
		Length height;
		Length thickness;
		Color color;

		SvgRect(SvgCoordinate p, Length width, Length height, Length thickness,
				Color color = Color::Default)
			: p(p), width(width), height(height), thickness(thickness), color(color) {}
	};

public:
	FreespaceLightVis(FrechetLight const& frechet);

	void exportToSvg(std::string const& filename);
	void exportFreespaceToSvg(std::string const& filename);
	void onlyDrawDiagram() { draw_diagram = true; draw_curves = false; };
	void onlyDrawCurves() { draw_diagram = false; draw_curves = true; };

private:
	FrechetLight const& frechet;

	void setWidthAndHeight();
	void writeHeader(std::ofstream& f);
	void writeEmptyIntervals(std::ofstream& f);
	void writeReachLists(std::ofstream& f);
	void writeUnknownIntervals(std::ofstream& f);
	void writeConnections(std::ofstream& f);
	void writeFreeNonReachable(std::ofstream& f);
	void writeCells(std::ofstream& f);
	void writeCertificate(std::ofstream& f);
	void writeYesCertificate(std::ofstream& f);
	void writeNoCertificate(std::ofstream& f);
	void writeCurves(std::ofstream& f);
	void writeFooter(std::ofstream& f);

	void writeInterval(std::ofstream& f, CInterval cinterval, Color color, bool dashed = false);


	SvgCoordinate toSvgCoordinate(CPosition pt);
	SvgCoordinate toSvgCoordinate(CPoint point1, CPoint point2, Orientation orientation);
	SvgCoordinate toSvgCurvePoint(Point const& point);
	void writePoint(std::ofstream& f, SvgPoint point);
	void writeLine(std::ofstream& f, SvgLine line);
	void writePolyline(std::ofstream& f, SvgCoordinates const& svg_points, Color color);
	void writeRectangle(std::ofstream& f, SvgRect rect);
	void writeCell(PointID i, PointID j, Ellipse const& e, std::ofstream& f);
	void writePolygonCell(PointID i, PointID j, std::ofstream& f);

	std::string toColorString(Color color) const;
};
