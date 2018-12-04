#pragma once

#include "defs.h"
#include "frechet_abstract.h"
#include "geometry_basics.h"
#include "curves.h"

class FrechetNaive final : public FrechetAbstract
{
public:
	FrechetNaive() {
		std::cout << "Initializing FrechetNaive algorithm...\n";
	}; // = default;
	bool lessThan(distance_t distance, Curve const& curve1, Curve const& curve2);
	bool lessThanWithFilters(distance_t distance, Curve const& curve1, Curve const& curve2);
	Certificate&  computeCertificate() { return cert; }

private:
	Certificate cert;
};
