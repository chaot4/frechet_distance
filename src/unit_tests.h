#pragma once

#include <iostream>
#include "geometry_basics.h"
#include "frechet_light.h"
#include "certificate.h"
#include "curves.h"

namespace unit_tests
{
	void testAll();

	void testLightCertificate();
	void testLightCertificate(std::string curve1file, std::string curve2file, distance_t distance);

}
