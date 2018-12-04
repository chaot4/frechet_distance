#pragma once

#include "frechet_abstract.h"
#include "geometry_basics.h"
#include "query_helper.h"
#include "times.h"
#include "curves.h"

#include <string>

class Query
{
public:
	Query(std::string const& curve_directory);
	~Query();

	void readCurveData(std::string const& curve_data_file);
	void readQueryCurves(std::string const& query_curves_file);
	void setAlgorithm(std::string const& frechet_version);
	void getReady();

	void run();
	void run_parallel();
	void run(Curve const& curve, distance_t distance);

	Results const& getResults() const;
	void saveResults(std::string const& results_file) const;

	// for comparison with the old implementation
	int getHash() const;
	void printQueryInformation(std::size_t query_index) const;
	Curve const& getCurve(std::size_t curve_index) const;
	Curves const& getCurves() const;
	void printDataStats(bool as_table = false) const;

	// yes, this is ugly... but easiest way for testing.
	void setRules(std::array<bool,5> const& enable);
	void setPruningLevel(int pruning_level);

	// get an upper bound on the fréchet distance of all curves in the data set
	distance_t getUpperBoundDistance() const;

	struct HardInstance {
		Curve const& curve1;
		Curve const& curve2;
		distance_t distance;
	};
	using HardInstances = std::vector<HardInstance>;
	HardInstances getHardInstances();

private:
	bool is_ready = false;
	FrechetAbstract* frechet = nullptr;

	std::string const curve_directory;

	QueryElements query_elements;
	Curves curve_data;
	CurveIDs candidates;
	Results results;

	Tree kd_tree;

	std::size_t num_threads;
	struct ThreadData {
		FrechetAbstract* frechet = nullptr;
		CurveIDs candidates;
	};
	std::vector<ThreadData> thread_data_vec;

	void run_impl(Curve const& curve, distance_t distance);
	void run_impl_parallel(Curve const& curve, distance_t distance, Result& result);

	void check_certificate(Certificate const& cert, Times::CertType type);
};
