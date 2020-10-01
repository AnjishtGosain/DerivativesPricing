#ifndef __OPTIMISERSTESTS_H__
#define __OPTIMISERSTESTS_H__

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include "../Optimisers/DifferentialEvolution.h"

using namespace std;
using namespace std::chrono;
using namespace optimisers;

namespace tests
{
	// Check whether the differential evolution implementation works by finding the minimum to the two dimension version of Ackley's function. 
	// This minimum is (0, 0).
	double AckelysFunction(const std::shared_ptr<std::vector<double>>& xValues, const int& N)
	{
		auto pi = 3.1415926535897;
		auto div = 1.0 / (double)N;
		auto x1 = xValues->at(0);
		auto x2 = xValues->at(1);
		auto value = 20.0 + exp(1.0) - 20.0 * exp(-0.2 * sqrt(div * (pow(x1, 2) + pow(x2, 2)))) - exp(div*(cos(2.0*pi*x1) + cos(2.0*pi*x2)));
		return value;
	}

	bool OptimisersTest1()
	{
		high_resolution_clock::time_point tStart = high_resolution_clock::now();

		function<double (const shared_ptr<vector<double>>&, const int&)> ackelysFunctionPtr = AckelysFunction;
		auto lowerBoundsPtr = make_shared<vector<double>>(vector<double>{-5.0, -5.0});
		auto upperBoundsPtr = make_shared<vector<double>>(vector<double>{5.0, 5.0});

		auto xValuesPtr = make_shared<vector<double>>(vector<double>{0.0001, 0.0001});
		auto value = ackelysFunctionPtr(xValuesPtr, 2);
		std::cout << value << std::endl;

		DifferentialEvolution optimiser(2, 0.5, 0.1, lowerBoundsPtr, upperBoundsPtr, ackelysFunctionPtr, Implementation::One, 10000, 0);
		auto solution = optimiser.solve(0.003);

		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		std::cout << "Optimisation time" << duration_cast<milliseconds>(tEnd - tStart).count() << std::endl;

		// Check values
		auto testPass = true;
		return testPass;
	}

}
#endif // !__OPTIMISERSTESTS_H__
