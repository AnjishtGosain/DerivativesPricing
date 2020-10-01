// IMCSolutionAPI.cpp : Defines the entry point for the console application.
//

#include "JSONUtilities.h"
#include "PartA.h"
#include "PartB.h"
#include "../Instruments/VanillaOption.h"
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/Implementation.h"
#include <unordered_map>

using namespace std;
using namespace instruments;
using namespace enumerations;

void PartB5();
void PartB6();
void PartB7();

void PressEnterToContinue()
{
	int c;
	printf("Press ENTER to continue... ");
	fflush(stdout);
	do c = getchar(); while ((c != '\n') && (c != EOF));
}


int main()
{
	//PartB5();
	PartB6();
	//PartB7();

	PressEnterToContinue();
	return 0;
}

void PartB5()
{
	auto jsonInputFilePath = "C://Users//anjis//Downloads//take_home_datasetA.json";
	auto jsonOutputFilePath = "C://Users//anjis//Downloads//B5.json";
	auto initialUnderlyingPrice = 170.0;
	auto jumpMean = 0.0;
	auto nTimeSteps = 7;


	try
	{
		PartB::priceVanillaOptionsBlackScholesDoubleNormalJumpModel(jsonInputFilePath, jsonOutputFilePath, initialUnderlyingPrice,
			jumpMean, nTimeSteps);
	}
	catch (exception &err)
	{
		cerr << err.what() << endl;
	}
}

void PartB6()
{
	auto jsonInputFilePath = "C://Users//anjis//Downloads//take_home_datasetB.json";
	auto jsonOutputFilePath = "C://Users//anjis//Downloads//B6.json";
	auto costOfCarry = 0.004;
	auto discountRate = 0.02;
	auto initialUnderlyingPrice = 170.0;
	auto dividendTime = 14.0 / 365.0;
	auto dividendAmount = 0.5;
	auto jumpTime = 7.0 / 365.0;
	auto jumpMean = 0.0;
	//auto nTimeSteps = 5;
	auto nTimeSteps = 4;

	auto F = 0.5;
	auto CR = 0.1;
	//auto N = 10;
	auto N = 200;
	//auto lowerBounds = make_shared<vector<double>>(vector<double>{0.0, 0.0});
	//auto upperBounds = make_shared<vector<double>>(vector<double>{0.5, 0.5});
	auto lowerBounds = make_shared<vector<double>>(vector<double>{0.10, 0.01});
	auto upperBounds = make_shared<vector<double>>(vector<double>{0.20, 0.1});
	//auto tolerance = 0.04;
	//auto tolerance = 0.02;
	auto tolerance = 0.022;
	auto seed = 0;

	try
	{
		// Optimise the solution
		auto solution = PartB::optimisePartB6(jsonInputFilePath, costOfCarry, discountRate, initialUnderlyingPrice,
			dividendTime, dividendAmount, jumpTime, jumpMean, nTimeSteps,
			F, CR, N, lowerBounds, upperBounds, tolerance, seed);

		// Print out the calculated prices
		PartB::priceAmericanOptionsBlackScholesSingleNormalJumpModel(
			jsonInputFilePath, jsonOutputFilePath, costOfCarry, discountRate, solution->at(0), initialUnderlyingPrice,
			dividendTime, dividendAmount, jumpTime, jumpMean, solution->at(1), nTimeSteps);
	}
	catch (exception &err)
	{
		cerr << err.what() << endl;
	}
	return;
}

void PartB7()
{
	auto jsonInputFilePath = "C://Users//anjis//Downloads//take_home_datasetC.json";
	auto jsonOutputFilePath = "C://Users//anjis//Downloads//B7.json";
	auto initialUnderlyingPrice = 170.0;
	auto dividendTime = 14.0 / 365.0;
	auto jumpTime = 7.0 / 365.0;
	auto jumpMean = 0.0;
	//auto nTimeSteps = 4;
	auto nTimeSteps = 4;

	auto F = 0.5;
	auto CR = 0.1;
	//auto N = 10;
	//auto N = 200;
	auto N = pow(5,5);
	//auto lowerBounds = make_shared<vector<double>>(vector<double>{0.01, 0.01, 0.01, 0.2, 0.01});
	//auto upperBounds = make_shared<vector<double>>(vector<double>{0.1, 0.1, 0.5, 0.7, 0.3});
	auto lowerBounds = make_shared<vector<double>>(vector<double>{0.01, 0.01, 0.1, 0.1, 0.01});
	//auto upperBounds = make_shared<vector<double>>(vector<double>{0.1, 0.1, 0.3, 1.5, 0.2});
	auto upperBounds = make_shared<vector<double>>(vector<double>{0.1, 0.1, 0.3, 10.0, 0.2});
	auto tolerance = 0.001;
	auto seed = 0;

	try
	{
		// Optimise the solution
		auto solution = PartB::optimisePartB7(jsonInputFilePath, initialUnderlyingPrice,
			dividendTime, jumpTime, jumpMean, nTimeSteps,
			F, CR, N, lowerBounds, upperBounds, tolerance, seed);

		// Print out the calculated prices
		PartB::priceAmericanOptionsBlackScholesSingleNormalJumpModel(jsonInputFilePath, jsonOutputFilePath, solution->at(0), solution->at(1), solution->at(2), initialUnderlyingPrice,
			dividendTime, solution->at(3), jumpTime, jumpMean, solution->at(4), nTimeSteps);
	}
	catch (exception &err)
	{
		cerr << err.what() << endl;
	}
	return;
}

