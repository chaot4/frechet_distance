#include "times.h"

#include <iomanip>

namespace global { Times times; }

std::ostream& operator<<(std::ostream& out, Times const& times)
{
	out << std::setprecision(3) << std::fixed
	<< "preprocessing: " << times.preprocessing_sum/1000000000. << "s\n"
	<< "reading query curves: " << times.reading_query_curve_sum/1000000000. << "s\n"
	<< "kd search: " << times.kd_search_sum/1000000000. << "s\n"
	<< "frechet query (total " << times.frechet_query_sum/1000000000. << "s):\n"
	<< "   - greedy: " << times.greedy_sum/1000000000. << "s\n"
	<< "   - simultaneous greedy: " << times.simultaneous_greedy_sum/1000000000. << "s\n"
	<< "   - negative: " << times.negative_sum/1000000000. << "s\n"
	<< "   - lessthan: " << times.lessthan_sum/1000000000. << "s\n";
#ifdef CERTIFY
	out << std::setprecision(3) << std::fixed
	<< "   - certificate computation: " << times.certcomp_sum/1000000000. << "s\n"
	<< "   \t* YES: " << times.certcompyes_sum/1000000000. << "s\n"
	<< "   \t* NO: " << times.certcompno_sum/1000000000. << "s\n"
	<< "   \t   -- build OrthRangeSearch data structure: " << times.buildorthrange_sum/1000000000. << "s\n"
	<< "   \t   -- find traversal: " << times.findno_sum/1000000000. << "s\n"
	<< "   - certificate check: " << times.certcheck_sum/1000000000. << "s\n";
#endif 
	/*<< "   - free tests: " << times.tests_sum/1000000000. << "s\n"
	<< "     - for boxes: " << times.tests_boxes_sum/1000000000. << "s\n"
	<< "     - for boundaries: " << times.tests_boundaries_sum/1000000000. << "s\n"
	<< "   - pruning: " << times.pruning_sum/1000000000. << "s\n"
	<< "   - splits: " << times.splits_sum/1000000000. << "s\n"
	<< "   - reachability: " << times.reachability_sum/1000000000. << "s\n";*/
	
	double avgCandidates = ((double) times.sum_numCandidates) / ((double) times.numCandidateCounts);
	double avgFilteredByBichromaticFarthestDistance = ((double) times.sum_numFilteredByBichromaticFarthestDistance) / ((double) times.numCandidateCounts);
	double avgFilteredByGreedy = ((double) times.sum_numFilteredByGreedy) / ((double) times.numCandidateCounts);
	double avgFilteredBySimultaneousGreedy = ((double) times.sum_numFilteredBySimultaneousGreedy) / ((double) times.numCandidateCounts);
	double avgFilteredByNegative = ((double) times.sum_numFilteredByNegative) / ((double) times.numCandidateCounts);
	double avgPosNotFiltered = ((double) times.sum_numPosNotFiltered) / ((double) times.numCandidateCounts);
	out << "#candidate-curves = " << avgCandidates << "\n";
	out << "#filtered-curves = " << avgFilteredByBichromaticFarthestDistance + avgFilteredByGreedy + avgFilteredBySimultaneousGreedy + avgFilteredByNegative << "\n";
	out << "#YES-curves = " << avgFilteredByBichromaticFarthestDistance + avgFilteredByGreedy + avgFilteredBySimultaneousGreedy + avgPosNotFiltered << "\n";
	out << "#YES-curves after bichromatic farthest distance = " << avgFilteredByGreedy + avgFilteredBySimultaneousGreedy + avgPosNotFiltered << "\n";
	out << "#YES-curves after greedy = " << avgPosNotFiltered + avgFilteredBySimultaneousGreedy << "\n";
	out << "#YES-curves after simultaneous greedy = " << avgPosNotFiltered << "\n";
	out << "#NO-curves = " << avgCandidates - avgFilteredByBichromaticFarthestDistance - avgFilteredByGreedy - avgFilteredBySimultaneousGreedy - avgPosNotFiltered << "\n";
	out << "#NO-curves after negative filter = " << avgCandidates - avgFilteredByBichromaticFarthestDistance - avgFilteredByGreedy - avgFilteredBySimultaneousGreedy - avgPosNotFiltered - avgFilteredByNegative << "\n";
	
	// karl:
	/*out << "numSplits:\n";
	size_t maximum = *std::max_element(times.numsSplits.begin(), times.numsSplits.end());
	for (size_t i = 1; i<=maximum; i++) {
		out << std::count(times.numsSplits.begin(), times.numsSplits.end(), i) << ", ";
	}
	out << "\n";
	out << "times 0: " << std::count(times.numsSplits.begin(), times.numsSplits.end(), 0) << "\n";*/
	out << "average number of boxes per call of lessThan: " 
	<< ((double) times.sum_numSplits) / ((double) times.numSplitCounts) << "\n";
	out << "number of calls of lessThan: " << times.numSplitCounts << "\n";
	
	out << "average number of steps per call of freeTestExactUsingHeuristic: " 
	<< ((double) times.sum_numFreeTestSteps) / ((double) times.numFreeTestCounts) << "\n";
	out << "average number of steps per call of freeTestExact: " 
	<< ((double) times.sum_sizeFreeTestSteps) / ((double) times.numFreeTestCounts) << "\n";
	out << "number of calls of freeTestExactUsingHeuristic: " << times.numFreeTestCounts << "\n";
	
	out << "average number of steps per call of adaptiveGreedy: " 
	<< ((double) times.sum_numGreedySteps) / ((double) times.numGreedyCounts) << "\n";
	out << "average distance walked per call of adaptiveGreedy: " 
	<< ((double) times.sum_sizeGreedySteps) / ((double) times.numGreedyCounts) << "\n";
	out << "number of calls of adaptiveGreedy: " << times.numGreedyCounts << "\n";

#ifdef CERTIFY
	out << "average number of empty_intervals per NO (complete search) certificate: " 
	<< ((double) times.cert_empty_total_sum) / ((double) times.cert_empty_counts) << "\n";
	out << "\t - start set: " 
	<< ((double) times.cert_empty_initial_sum) / ((double) times.cert_empty_counts) << "\n";
	out << "\t - encountered: " 
	<< ((double) times.cert_empty_enc_sum) / ((double) times.cert_empty_counts) << "\n";
	out << "average number of internal node visits in orthogonal range search data structure " 
	<< ((double) times.orth_range_visit_sum) / ((double) times.cert_empty_counts) << "\n";
	out << "average number of internal nodes in orthogonal range search data structure " 
	<< ((double) times.orth_range_size_sum) / ((double) times.cert_empty_counts) << "\n";
	out << "number of NO certificates: " << times.cert_empty_counts << "\n";

	std::cout << "\nFilter certificates:\n";
	std::cout << "Number of correct YES filter certificates: " << global::times.numYesCorrect[Times::FILTER] << "/" << global::times.numYesChecked[Times::FILTER] << std::endl;
	std::cout << "Number of correct NO filter certificates: " << global::times.numNoCorrect[Times::FILTER] << "/" << global::times.numNoChecked[Times::FILTER] << std::endl;
	std::cout << "Number of invalid filter certificates: " << global::times.numInvalid[Times::FILTER] << std::endl;

	std::cout << "\nComplete search certificates:\n";
	std::cout << "Number of correct YES complete search certificates: " << global::times.numYesCorrect[Times::COMPLETE] << "/" << global::times.numYesChecked[Times::COMPLETE] << std::endl;
	std::cout << "Number of correct NO complete search certificates: " << global::times.numNoCorrect[Times::COMPLETE] << "/" << global::times.numNoChecked[Times::COMPLETE] << std::endl;
	std::cout << "Number of invalid complete search certificates: " << global::times.numInvalid[Times::COMPLETE] << std::endl;

#endif
	/*out << "numsFreeTestSteps:\n";
	maximum = *std::max_element(times.numsFreeTestSteps.begin(), times.numsFreeTestSteps.end());
	for (size_t i = 1; i<=maximum; i++) {
		out << std::count(times.numsFreeTestSteps.begin(), times.numsFreeTestSteps.end(), i) << ", ";
	}
	out << "\n";
	out << "sizesFreeTestSteps:\n";
	maximum = *std::max_element(times.sizesFreeTestSteps.begin(), times.sizesFreeTestSteps.end());
	for (size_t i = 1; i<=maximum; i++) {
		out << std::count(times.sizesFreeTestSteps.begin(), times.sizesFreeTestSteps.end(), i) << ", ";
	}
	out << "\n";*/

	

	return out;
}
