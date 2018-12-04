#pragma once

#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <vector>

// Comment this this in to disable almost all timings.
// This makes a big difference in time measurements, especially when parallelized.
#define TURBO

#if !defined(TURBO) || defined(CERTIFY)
struct Times
{
	using hrc = std::chrono::high_resolution_clock;
	using time_point = hrc::time_point;
	using ns = std::chrono::nanoseconds;

	double stop(time_point start) {
		return std::chrono::duration_cast<ns>(hrc::now() - start).count();
	}

	void reset() {
		*this = Times();
	}

	double preprocessing_sum = 0.;
	double reading_query_curve_sum = 0.;
	double kd_search_sum = 0.;
	double frechet_query_sum = 0.;
	double tests_sum = 0.;
	double tests_boxes_sum = 0.;
	double tests_boundaries_sum = 0.;
	double pruning_sum = 0.;
	double splits_sum = 0.;
	double reachability_sum = 0.;
	double greedy_sum = 0.;
	double simultaneous_greedy_sum = 0.;
	double negative_sum = 0.;
	double lessthan_sum = 0.;
	double certcomp_sum = 0.;
	double certcompyes_sum = 0.;
	double certcompno_sum = 0.;
	double certcheck_sum = 0.;
	double buildorthrange_sum = 0.;
	double findno_sum = 0.;

	time_point preprocessing_start;
	time_point reading_query_curve_start;
	time_point kd_search_start;
	time_point frechet_query_start;
	time_point tests_start;
	time_point tests_boxes_start;
	time_point tests_boundaries_start;
	time_point pruning_start;
	time_point splits_start;
	time_point reachability_start;
	time_point greedy_start;
	time_point simultaneous_greedy_start;
	time_point negative_start;
	time_point lessthan_start;
	time_point certcomp_start;
	time_point certcompyes_start;
	time_point certcompno_start;
	time_point certcheck_start;
	time_point buildorthrange_start;
	time_point findno_start;
	
	size_t numSplitCounts = 0;
	size_t sum_numSplits = 0;
	size_t numSplits;
	void startCountingSplits() { numSplits = 0; }
	void newSplit() { numSplits++; }
	void stopCountingSplits() { sum_numSplits += numSplits; numSplits = 0; numSplitCounts++; }
	size_t numFreeTestCounts = 0;
	size_t sum_numFreeTestSteps = 0;
	size_t sum_sizeFreeTestSteps = 0;
	size_t numFreeTestSteps;
	size_t sizeFreeTestSteps;
	void startCountingFreeTests() { numFreeTestSteps = 0; sizeFreeTestSteps = 0; }
	void incrementFreeTests(size_t stepsize) { numFreeTestSteps++; sizeFreeTestSteps += stepsize; }
	void stopCountingFreeTests() { sum_numFreeTestSteps += numFreeTestSteps; sum_sizeFreeTestSteps += sizeFreeTestSteps; numFreeTestSteps = 0; sizeFreeTestSteps = 0; numFreeTestCounts++; }
	size_t numGreedyCounts = 0;
	size_t sum_numGreedySteps = 0;
	size_t sum_sizeGreedySteps = 0;
	size_t numGreedySteps;
	size_t sizeGreedySteps;
	void startCountingGreedySteps() { numGreedySteps = 0; sizeGreedySteps = 0; }
	void incrementGreedySteps(size_t stepsize) { numGreedySteps++; sizeGreedySteps += stepsize; }
	void stopCountingGreedySteps() { sum_numGreedySteps += numGreedySteps; sum_sizeGreedySteps += sizeGreedySteps; numGreedySteps = 0; sizeGreedySteps = 0; numGreedyCounts++; }

	size_t numCandidateCounts = 0;
	size_t sum_numCandidates = 0;
	size_t sum_numFilteredByBichromaticFarthestDistance = 0;
	size_t sum_numFilteredByGreedy = 0;
	size_t sum_numFilteredBySimultaneousGreedy = 0;
	size_t sum_numFilteredByNegative = 0;
	size_t sum_numPosNotFiltered = 0;
	size_t numCandidates;
	size_t numFilteredByBichromaticFarthestDistance;
	size_t numFilteredByGreedy;
	size_t numFilteredBySimultaneousGreedy;
	size_t numFilteredByNegative;
	size_t numPosNotFiltered;
	void startCountingCandidatesEtc() { numCandidates = 0; numFilteredByBichromaticFarthestDistance = 0; numFilteredByGreedy = 0; numFilteredBySimultaneousGreedy = 0; numFilteredByNegative = 0; numPosNotFiltered = 0; }
	void incrementPosNotFiltered() { numPosNotFiltered++; }
	void incrementFilteredByBichromaticFarthestDistance() { numFilteredByBichromaticFarthestDistance++; }
	void incrementFilteredByGreedy() { numFilteredByGreedy++; }
	void incrementFilteredBySimultaneousGreedy() { numFilteredBySimultaneousGreedy++; }
	void incrementFilteredByNegative() { numFilteredByNegative++; }
	void incrementCandidates() { numCandidates++; }
	void stopCountingCandidatesEtc() { sum_numCandidates += numCandidates; sum_numFilteredByBichromaticFarthestDistance += numFilteredByBichromaticFarthestDistance; sum_numFilteredByGreedy += numFilteredByGreedy; sum_numFilteredBySimultaneousGreedy += numFilteredBySimultaneousGreedy; sum_numFilteredByNegative += numFilteredByNegative; sum_numPosNotFiltered += numPosNotFiltered; numCandidateCounts++; }

	void startPreprocessing() { preprocessing_start = hrc::now(); };
	void startReadingQueryCurve() { reading_query_curve_start = hrc::now(); }
	void startKdSearch() { kd_search_start = hrc::now(); }
	void startFrechetQuery() { frechet_query_start = hrc::now(); }
	void startTests() { tests_start = hrc::now(); }
	void startTestsBoxes() { tests_boxes_start = hrc::now(); }
	void startTestsBoundaries() { tests_boundaries_start = hrc::now(); }
	void startPruning() { pruning_start = hrc::now(); }
	void startSplits() { splits_start = hrc::now(); }
	void startReachability() { reachability_start = hrc::now(); }
	void startGreedy() { greedy_start = hrc::now(); }
	void startSimultaneousGreedy() { simultaneous_greedy_start = hrc::now(); }
	void startNegative() { negative_start = hrc::now(); }
	void startLessThan() { lessthan_start = hrc::now(); }
	

	void stopPreprocessing() { preprocessing_sum += stop(preprocessing_start); }
	void stopReadingQueryCurve() { reading_query_curve_sum += stop(reading_query_curve_start); }
	void stopKdSearch() { kd_search_sum += stop(kd_search_start); }
	void stopFrechetQuery() {  frechet_query_sum += stop(frechet_query_start); }
	void stopTests() { tests_sum += stop(tests_start); }
	void stopTestsBoxes() { tests_boxes_sum += stop(tests_boxes_start); }
	void stopTestsBoundaries() { tests_boundaries_sum += stop(tests_boundaries_start); }
	void stopPruning() { pruning_sum += stop(pruning_start); }
	void stopSplits() { splits_sum += stop(splits_start); }
	void stopReachability() { reachability_sum += stop(reachability_start); }
	void stopGreedy() { greedy_sum += stop(greedy_start); }
	void stopSimultaneousGreedy() { simultaneous_greedy_sum += stop(simultaneous_greedy_start); }
	void stopNegative() { negative_sum += stop(negative_start); }
	void stopLessThan() { lessthan_sum += stop(lessthan_start); }

	//certificates
	void startComputeCertificate() { certcomp_start = hrc::now(); }
	void startComputeYesCertificate() { certcompyes_start = hrc::now(); }
	void startComputeNoCertificate() { certcompno_start = hrc::now(); }
	void startCheckCertificate() { certcheck_start = hrc::now(); }
	void startBuildOrthRangeSearch() { buildorthrange_start = hrc::now(); }
	void startFindNoTraversal() { findno_start = hrc::now(); }

	void stopComputeCertificate() { certcomp_sum += stop(certcomp_start); }
	void stopComputeYesCertificate() { certcompyes_sum += stop(certcompyes_start); }
	void stopComputeNoCertificate() { certcompno_sum += stop(certcompno_start); }
	void stopCheckCertificate() { certcheck_sum += stop(certcheck_start); }
	void stopBuildOrthRangeSearch() { buildorthrange_sum += stop(buildorthrange_start); }
	void stopFindNoTraversal() { findno_sum += stop(findno_start); }


	enum CertType { 
	  FILTER = 0,
	  COMPLETE = 1,
       	};

	size_t numYesChecked[2] = {0,0};
	size_t numYesCorrect[2] = {0,0};
	size_t numNoChecked[2] = {0,0};
	size_t numNoCorrect[2] = {0,0};
	size_t numInvalid[2] = {0,0};
	void incrementYesChecked(CertType type) { numYesChecked[type]++; }
	void incrementYesCorrect(CertType type) { numYesCorrect[type]++; }
	void incrementNoChecked(CertType type) { numNoChecked[type]++; }
	void incrementNoCorrect(CertType type) { numNoCorrect[type]++; }
	void incrementInvalid(CertType type) { numInvalid[type]++; }

	size_t cert_empty_total_sum = 0;
	size_t cert_empty_initial_sum = 0;
	size_t cert_empty_enc_sum = 0;
	size_t cert_empty_total, cert_empty_initial, cert_empty_enc;
	size_t cert_empty_counts = 0;

	size_t orth_range_visit_sum = 0;
	size_t orth_range_visit = 0;
	size_t orth_range_size_sum = 0;

	void startCountingCertEmptyIntervals(size_t num) { cert_empty_total_sum += num; cert_empty_initial = 0; cert_empty_enc = 0; cert_empty_counts++; orth_range_visit_sum = 0;}
	void incrementCertEmptyIntervalsInitial() {cert_empty_initial++; }
	void incrementCertEmptyIntervalsEncountered() {cert_empty_enc++; }
	void incrementCertOrthRangeNodeVisit() {orth_range_visit++; }
	void recordOrthRangeTreeSize(size_t num) {orth_range_size_sum  += num; }
	void stopCountingCertEmptyIntervals() { cert_empty_initial_sum += cert_empty_initial; cert_empty_enc_sum += cert_empty_enc; orth_range_visit_sum += orth_range_visit; }


};

#else

struct Times
{
	using hrc = std::chrono::high_resolution_clock;
	using time_point = hrc::time_point;
	using ns = std::chrono::nanoseconds;

	double stop(time_point start) {
		return std::chrono::duration_cast<ns>(hrc::now() - start).count();
	}

	void reset() {
		*this = Times();
	}

	double preprocessing_sum = 0.;
	double reading_query_curve_sum = 0.;
	double kd_search_sum = 0.;
	double frechet_query_sum = 0.;
	double tests_sum = 0.;
	double tests_boxes_sum = 0.;
	double tests_boundaries_sum = 0.;
	double pruning_sum = 0.;
	double splits_sum = 0.;
	double reachability_sum = 0.;
	double greedy_sum = 0.;
	double simultaneous_greedy_sum = 0.;
	double negative_sum = 0.;
	double lessthan_sum = 0.;
	double certcomp_sum = 0.;
	double certcompyes_sum = 0.;
	double certcompno_sum = 0.;
	double certcheck_sum = 0.;
	double buildorthrange_sum = 0.;
	double findno_sum = 0.;

	time_point preprocessing_start;
	time_point reading_query_curve_start;
	time_point kd_search_start;
	time_point frechet_query_start;
	time_point tests_start;
	time_point tests_boxes_start;
	time_point tests_boundaries_start;
	time_point pruning_start;
	time_point splits_start;
	time_point reachability_start;
	time_point greedy_start;
	time_point simultaneous_greedy_start;
	time_point negative_start;
	time_point lessthan_start;
	time_point certcomp_start;
	time_point certcompyes_start;
	time_point certcompno_start;
	time_point certcheck_start;
	time_point buildorthrange_start;
	time_point findno_start;
	
	size_t numSplitCounts = 0;
	size_t sum_numSplits = 0;
	size_t numSplits;
	void startCountingSplits() {}
	void newSplit() {}
	void stopCountingSplits() {}
	size_t numFreeTestCounts = 0;
	size_t sum_numFreeTestSteps = 0;
	size_t sum_sizeFreeTestSteps = 0;
	size_t numFreeTestSteps;
	size_t sizeFreeTestSteps;
	void startCountingFreeTests() {}
	void incrementFreeTests(size_t stepsize) {}
	void stopCountingFreeTests() {}
	size_t numGreedyCounts = 0;
	size_t sum_numGreedySteps = 0;
	size_t sum_sizeGreedySteps = 0;
	size_t numGreedySteps;
	size_t sizeGreedySteps;
	void startCountingGreedySteps() {}
	void incrementGreedySteps(size_t stepsize) {}
	void stopCountingGreedySteps() {}

	size_t numCandidateCounts = 0;
	size_t sum_numCandidates = 0;
	size_t sum_numFilteredByBichromaticFarthestDistance = 0; 
	size_t sum_numFilteredByGreedy = 0;
	size_t sum_numFilteredBySimultaneousGreedy = 0;
	size_t sum_numFilteredByNegative = 0;
	size_t sum_numPosNotFiltered = 0;
	size_t numCandidates;
	size_t numFilteredByBichromaticFarthestDistance; 
	size_t numFilteredByGreedy;
	size_t numFilteredBySimultaneousGreedy;
	size_t numFilteredByNegative;
	size_t numPosNotFiltered;
	void startCountingCandidatesEtc() {}
	void incrementPosNotFiltered() {}
	void incrementFilteredByBichromaticFarthestDistance() { }
	void incrementFilteredByGreedy() {}
	void incrementFilteredBySimultaneousGreedy() {}
	void incrementFilteredByNegative() {}
	void incrementCandidates() {}
	void stopCountingCandidatesEtc() {}

	void startPreprocessing() {}
	void startReadingQueryCurve() {}
	void startKdSearch() {}
	void startFrechetQuery() { frechet_query_start = hrc::now(); }
	void startTests() {}
	void startTestsBoxes() {}
	void startTestsBoundaries() {}
	void startPruning() {}
	void startSplits() {}
	void startReachability() {}
	void startGreedy() {}
	void startSimultaneousGreedy() {}
	void startNegative() {}
	void startLessThan() {}




	void stopPreprocessing() {}
	void stopReadingQueryCurve() {}
	void stopKdSearch() {}
	void stopFrechetQuery() {  frechet_query_sum += stop(frechet_query_start); }
	void stopTests() {}
	void stopTestsBoxes() {}
	void stopTestsBoundaries() {}
	void stopPruning() {}
	void stopSplits() {}
	void stopReachability() {}
	void stopGreedy() {}
	void stopSimultaneousGreedy() {}
	void stopNegative() {}
	void stopLessThan() {}

	//certificates
	void startComputeCertificate() {}
	void startComputeYesCertificate() {}
	void startComputeNoCertificate() {}
	void startCheckCertificate() {}
	void startBuildOrthRangeSearch() {}
	void startFindNoTraversal() {}

	void stopComputeCertificate() {}
	void stopComputeYesCertificate() {}
	void stopComputeNoCertificate() {}
	void stopCheckCertificate() {}
	void stopBuildOrthRangeSearch() {}
	void stopFindNoTraversal() {}

	enum CertType { 
	  FILTER = 0,
	  COMPLETE = 1,
       	};

	size_t numYesChecked[2] = {0,0};
	size_t numYesCorrect[2] = {0,0};
	size_t numNoChecked[2] = {0,0};
	size_t numNoCorrect[2] = {0,0};
	size_t numInvalid[2] = {0,0};
	void incrementYesChecked(CertType type) { }
	void incrementYesCorrect(CertType type) { }
	void incrementNoChecked(CertType type) { }
	void incrementNoCorrect(CertType type) { }
	void incrementInvalid(CertType type) { }

	size_t cert_empty_total_sum = 0;
	size_t cert_empty_initial_sum = 0;
	size_t cert_empty_enc_sum = 0;
	size_t cert_empty_total, cert_empty_initial, cert_empty_enc;
	size_t cert_empty_counts = 0;

	size_t orth_range_visit_sum = 0;
	size_t orth_range_visit = 0;
	size_t orth_range_size_sum = 0;

	void startCountingCertEmptyIntervals(size_t num) {}
	void incrementCertEmptyIntervalsInitial() {}
	void incrementCertEmptyIntervalsEncountered() {}
	void incrementCertOrthRangeNodeVisit() {}
	void recordOrthRangeTreeSize(size_t num) {}
	void stopCountingCertEmptyIntervals() {}
	//end certificates
};

#endif

namespace global { extern Times times; }

std::ostream& operator<<(std::ostream& out, Times const& times);
