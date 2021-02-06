#include "query.h"

#include "defs.h"
#include "filter.h"
#include "frechet_light.h"
#include "frechet_naive.h"
#include "parser.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

#ifdef WITH_OPENMP
#include <omp.h>
#endif

namespace
{

inline static bool isNear(Tree::Point const& a, Tree::Point const& b, distance_t distance)
{
	for (size_t i = 0; i < 4; i += 2) {
		auto d = (a[i] - b[i])*(a[i] - b[i]) + (a[i + 1] - b[i + 1])*(a[i + 1] - b[i + 1]);
		if (d > distance*distance) { return false; }
	}
	for (size_t i = 4; i < 8; ++i) {
		auto d = std::abs(a[i] - b[i]);
		if (d > distance) { return false; }
	}

	return true;
}

} // end anonymous namespace

Query::Query(std::string const& curve_directory)
	: curve_directory(curve_directory)
	, kd_tree(isNear)
#ifdef WITH_OPENMP
	, num_threads(omp_get_max_threads())
#else
	, num_threads(1)
#endif
	, thread_data_vec(num_threads)
{
}

Query::~Query()
{
	delete frechet;
	frechet = nullptr;

	for (auto& thread_data: thread_data_vec) {
		delete thread_data.frechet;
		thread_data.frechet = nullptr;
	}
}

void Query::readCurveData(std::string const& curve_data_file)
{
	is_ready = false;

	// read filenames of curve files
	std::ifstream file(curve_data_file);
	std::vector<std::string> curve_filenames;
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			curve_filenames.push_back(line);
		}
	}
	else {
		ERROR("The curve data file could not be opened: " << curve_data_file);
	}

	// read curves
	curve_data.clear();
	curve_data.reserve(curve_filenames.size());

	for (auto const& curve_filename: curve_filenames) {
		std::ifstream curve_file(curve_directory + curve_filename);
		if (curve_file.is_open()) {
			curve_data.emplace_back();
			parser::readCurve(curve_file, curve_data.back());
			curve_data.back().filename = curve_filename;

			if (curve_data.back().empty()) { curve_data.pop_back(); }
		}
		else {
			ERROR("A curve file could not be opened: " << curve_directory + curve_filename);
		}
	}
}

void Query::readQueryCurves(std::string const& query_curves_file)
{
	query_elements.clear();

	// read filenames of curve files and distances
	std::ifstream file(query_curves_file);
	std::vector<std::string> curve_filenames;
	if (file.is_open()) {
		std::stringstream ss;
		ss << file.rdbuf();

		std::string distance_string;
		std::string curve_filename;
		while (ss >> curve_filename >> distance_string) {
			curve_filenames.push_back(curve_filename);
			query_elements.emplace_back(Curve(), std::stod(distance_string));
		}
	}
	else {
		ERROR("The curve data file could not be opened: " << query_curves_file);
	}

	// read curves
	for (std::size_t i = 0; i < curve_filenames.size(); ++i) {
		std::ifstream curve_file(curve_directory + curve_filenames[i]);
		if (curve_file.is_open()) {
			parser::readCurve(curve_file, query_elements[i].curve);
			query_elements[i].curve.filename = curve_filenames[i];
		}
		else {
			ERROR("A curve file could not be opened: " << curve_directory + curve_filenames[i]);
		}
	}
}

void Query::setAlgorithm(std::string const& frechet_version)
{
	delete frechet;
	frechet = nullptr;

	if (frechet_version == "light") {
		frechet = new FrechetLight();
		for (auto& thread_data: thread_data_vec) {
			thread_data.frechet = new FrechetLight();
		}
	}
	else if (frechet_version == "naive") {
		frechet = new FrechetNaive();
		for (auto& thread_data: thread_data_vec) {
			thread_data.frechet = new FrechetNaive();
		}
	}
	else {
		ERROR("Unknown Frechet version: " << frechet_version << "\n"
			  "Known Frechet versions: light, naive");
	}
}

void Query::getReady()
{
	results.clear();

	//
	// build all the data structures and make queries ready
	//

	// for sequential
	kd_tree.clear();
	for (CurveID id = 0; id < curve_data.size(); ++id) {
		auto const& curve = curve_data[id];
		kd_tree.add(toKdPoint(curve), id);
	}
	kd_tree.build();

	is_ready = true;
}

void Query::run()
{
	assert(is_ready);
	results.clear();

	for (auto const& query_element: query_elements) {
		run_impl(query_element.curve, query_element.distance);
	}
}

void Query::run_parallel()
{
	assert(is_ready);

	results.clear();
	results.resize(query_elements.size());

	global::times.startFrechetQuery();
#ifdef WITH_OPENMP
	#pragma omp parallel for schedule(guided) num_threads(num_threads)
#endif
	for (std::size_t i = 0; i < query_elements.size(); ++i) {
		auto const& query_element = query_elements[i];
		run_impl_parallel(query_element.curve, query_element.distance, results[i]);
	}
	global::times.stopFrechetQuery();
}

void Query::run(Curve const& curve, distance_t distance)
{
	assert(is_ready);
	results.clear();

	run_impl(curve, distance);
}

void Query::check_certificate(Certificate const& c, Times::CertType type) {
#ifdef CERTIFY
	if (c.isValid()) {
		if (c.isYes()) {
			global::times.incrementYesChecked(type);
		} else {
			global::times.incrementNoChecked(type);
		}


		global::times.startCheckCertificate();
		if (c.check()) {
			if (c.isYes()) {
				global::times.incrementYesCorrect(type);
			} else {
				global::times.incrementNoCorrect(type);
			}
		} else {
			std::cout << "WRONG CERTIFICATE of type " << type << "!\n";
		}
		global::times.stopCheckCertificate();

	} else {
		//std::cout << "INVALID CERTIFICATE!\n";
		global::times.incrementInvalid(type);
	}


#endif
}

void Query::run_impl(Curve const& curve, distance_t distance)
{
	assert(is_ready);
	assert(frechet != nullptr);

	// add new result for this query
	results.emplace_back();
	auto& result = results.back();

	// perform query
	global::times.startKdSearch();
	candidates.clear();
	kd_tree.search(toKdPoint(curve), distance, candidates);
	global::times.stopKdSearch();
	global::times.startCountingCandidatesEtc();

	for (auto candidate: candidates) {
		global::times.startFrechetQuery();
		global::times.incrementCandidates();

		auto const& query_curve = curve;
		auto const& candidate_curve = curve_data[candidate];
		auto const max_distance = distance;

		//TODO rewrite as "for all positive filters do ..." and "for all negative filters do ..."? 
		//TODO filter always reallocates traversal vector
		Filter filter(query_curve, candidate_curve, max_distance);

		if (filter.bichromaticFarthestDistance()) {
			result.addCurve(candidate);
			global::times.stopFrechetQuery();
			global::times.incrementFilteredByBichromaticFarthestDistance();
			check_certificate(filter.getCertificate(), Times::FILTER);
			continue;
		}

		global::times.startGreedy();
		global::times.startCountingGreedySteps();
		PointID pos1;
		PointID pos2;
		if (filter.adaptiveGreedy(pos1, pos2)) {
			result.addCurve(candidate);
			global::times.stopGreedy();
			global::times.stopCountingGreedySteps();
			global::times.stopFrechetQuery();
			global::times.incrementFilteredByGreedy();
			check_certificate(filter.getCertificate(), Times::FILTER);
			continue;
		}
		global::times.stopGreedy();
		global::times.stopCountingGreedySteps();

		global::times.startNegative();
		if (filter.negative(pos1, pos2)) {
			global::times.stopNegative();
			global::times.stopFrechetQuery();
			global::times.incrementFilteredByNegative();
			assert(not filter.getCertificate().isValid());
			check_certificate(filter.getCertificate(), Times::FILTER);
			continue;
		}
		global::times.stopNegative();

		global::times.startSimultaneousGreedy();
		if (filter.adaptiveSimultaneousGreedy()) {
			result.addCurve(candidate);
			global::times.stopSimultaneousGreedy();
			global::times.stopFrechetQuery();
			global::times.incrementFilteredBySimultaneousGreedy();
			check_certificate(filter.getCertificate(), Times::FILTER);
			continue;
		}
		global::times.stopSimultaneousGreedy();

		global::times.startCountingSplits();
		global::times.startLessThan();
		if (frechet->lessThan(max_distance, query_curve, candidate_curve)) {
			result.addCurve(candidate);
			global::times.incrementPosNotFiltered();

		}
		global::times.stopLessThan();
#ifdef CERTIFY
		//NOTE: we do not count the cost of creating filter certificate (this is essentially only remembering traversals)
		global::times.startComputeCertificate();
		Certificate& c = frechet->computeCertificate();
		global::times.stopComputeCertificate();

		check_certificate(c, Times::COMPLETE);
		
#endif
		global::times.stopCountingSplits();
		global::times.stopFrechetQuery();
	}
	global::times.stopCountingCandidatesEtc();
}

void Query::run_impl_parallel(Curve const& curve, distance_t distance, Result& result)
{
	assert(is_ready);
	assert(frechet != nullptr);

#ifdef WITH_OPENMP
	auto& thread_data = thread_data_vec[omp_get_thread_num()];
#else
	auto& thread_data = thread_data_vec[0];
#endif
	auto& frechet = *thread_data.frechet;
	auto& candidates = thread_data.candidates;

	// perform query
	candidates.clear();
	kd_tree.search(toKdPoint(curve), distance, candidates);

	for (auto candidate: candidates) {
		auto const& query_curve = curve;
		auto const& candidate_curve = curve_data[candidate];
		auto const max_distance = distance;

		Filter filter(query_curve, candidate_curve, max_distance);

		if (filter.bichromaticFarthestDistance()) {
			result.addCurve(candidate);
			continue;
		}

		PointID pos1;
		PointID pos2;
		if (filter.adaptiveGreedy(pos1, pos2)) {
			result.addCurve(candidate);
			continue;
		}
		if (filter.negative(pos1, pos2)) {
			continue;
		}
		if (filter.adaptiveSimultaneousGreedy()) {
			result.addCurve(candidate);
			continue;
		}
		if (frechet.lessThan(max_distance, query_curve, candidate_curve)) {
			result.addCurve(candidate);
		}
	}
}

Results const& Query::getResults() const
{
	return results;
}

void Query::saveResults(std::string const& results_file) const
{
	std::ofstream file(results_file);
	if (file.is_open()) {
		for (auto const& result: results) {
			for (auto curve_id: result.curve_ids) {
				file << curve_data[curve_id].filename << " ";
			}
			file << "\n";
		}
	}
}

int Query::getHash() const
{
	int checksum = 0;
	for (auto const& result: results) {
		checksum += result.curve_ids.size();
	}

	return checksum;
}

void Query::printQueryInformation(std::size_t query_index) const
{
	auto const& query_element = query_elements[query_index];

	std::cout << "Query curve: " << query_element.curve.filename << "\n";
	std::cout << "Query distance: " << query_element.distance << "\n";
}

Curve const& Query::getCurve(std::size_t curve_index) const
{
	return curve_data[curve_index];
}

Curves const& Query::getCurves() const
{
	return curve_data;
}

void Query::printDataStats(bool as_table) const
{
	double mean_hops;
	double stddev_hops;
	double mean_length;
	double stddev_length;
	Curve::ExtremePoints data_extreme_points;

	mean_hops = 0.;
	mean_length = 0.;
	for (auto& curve: curve_data) {
		mean_hops += curve.size();
		mean_length += curve.curve_length(0, curve.size()-1);
	}
	mean_hops /= curve_data.size();
	mean_length /= curve_data.size();

	stddev_hops = 0.;
	stddev_length = 0.;
	for (auto& curve: curve_data) {
		stddev_hops += std::pow(mean_hops - curve.size(), 2.);
		stddev_length += std::pow(mean_length - curve.curve_length(0, curve.size()-1), 2.);
	}
	stddev_hops /= curve_data.size();
	stddev_length /= curve_data.size();
	stddev_hops = std::sqrt(stddev_hops);
	stddev_length = std::sqrt(stddev_length);

	data_extreme_points.min_x = data_extreme_points.min_y = 10000000000.;
	data_extreme_points.max_x = data_extreme_points.max_y = -10000000000.;
	for (auto& curve: curve_data) {
		auto curve_extremes = curve.getExtremePoints();
		data_extreme_points.min_x = std::min(data_extreme_points.min_x, curve_extremes.min_x);
		data_extreme_points.max_x = std::max(data_extreme_points.max_x, curve_extremes.max_x);
		data_extreme_points.min_y = std::min(data_extreme_points.min_y, curve_extremes.min_y);
		data_extreme_points.max_y = std::max(data_extreme_points.max_y, curve_extremes.max_y);
	}

	std::cout << std::setiosflags(std::ios::fixed) << std::setprecision(3);
	if (as_table) {
		std::cout << curve_data.size() << " & " << mean_hops << " & " << stddev_hops << " & " << mean_length << " & " << stddev_length << " & [" << data_extreme_points.min_x << ", " << data_extreme_points.max_x << "] \\times [" << data_extreme_points.min_y << ", " << data_extreme_points.max_y << "] \\\\\n";
	}
	else {
		std::cout << "Number of curves: " << curve_data.size() << "\n";
		std::cout << "Mean hops: " << mean_hops << "\n";
		std::cout << "Stddev hops: " << stddev_hops << "\n";
		std::cout << "Mean length: " << mean_length << "\n";
		std::cout << "Stddev length: " << stddev_length << "\n";
		std::cout << "Extreme points (min_x, max_x, min_y, max_y):\n"
			<< data_extreme_points.min_x << " " << data_extreme_points.max_x << " "
			<< data_extreme_points.min_y << " " << data_extreme_points.max_y << "\n";
	}
}

void Query::setRules(std::array<bool,5> const& enable)
{
	frechet->setRules(enable);
}

void Query::setPruningLevel(int pruning_level)
{
	frechet->setPruningLevel(pruning_level);
}

distance_t Query::getUpperBoundDistance() const
{
	if (curve_data.size() <= 1) { return 0.; }

	auto extreme = curve_data.front().getExtremePoints();
	Point min_point{extreme.min_x, extreme.min_y};
	Point max_point{extreme.max_x, extreme.max_y};

	for (auto const& curve: curve_data) {
		extreme = curve.getExtremePoints();
		min_point.x = std::min(min_point.x, extreme.min_x);
		min_point.y = std::min(min_point.y, extreme.min_y);
		max_point.x = std::max(max_point.x, extreme.max_x);
		max_point.y = std::max(max_point.y, extreme.max_y);
	}

	return min_point.dist(max_point);
}

auto Query::getHardInstances() -> HardInstances
{
	HardInstances hard_instances;

	assert(is_ready);
	for (auto const& query_element: query_elements) {
		assert(frechet != nullptr);
		auto const& curve = query_element.curve;
		auto const& distance = query_element.distance;

		// perform query
		candidates.clear();
		kd_tree.search(toKdPoint(curve), distance, candidates);

		for (auto candidate: candidates) {
			auto const& query_curve = curve;
			auto const& candidate_curve = curve_data[candidate];
			auto const max_distance = distance;

			Filter filter(query_curve, candidate_curve, max_distance);

			if (filter.bichromaticFarthestDistance()) {
				continue;
			}

			PointID pos1;
			PointID pos2;
			if (filter.adaptiveGreedy(pos1, pos2)) {
				continue;
			}
			if (filter.negative(pos1, pos2)) {
				continue;
			}
			if (filter.adaptiveSimultaneousGreedy()) {
				continue;
			}

			// if we reached this point, we found a hard instance
			hard_instances.push_back({query_curve, candidate_curve, max_distance});
		}
	}

	return hard_instances;
}
