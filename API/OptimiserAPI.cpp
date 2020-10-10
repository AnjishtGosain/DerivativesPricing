#include "OptimiserAPI.h"
#include <vector>
#include <memory>
#include "../Models/BlackScholesSingleNormalJump.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Pricers/TreePricer.h"
#include <functional>
#include "../Optimisers/DifferentialEvolution.h"
#include <unordered_map>
#include <map>

using namespace std;
using namespace models;
using namespace instruments;
using namespace enumerations;
using namespace pricers;
using namespace optimisers;
using namespace std::placeholders;


// Functions for Optimisation Problem 1
// ----------------------------------------------------------------------------

// Function which revalues a set of options, and returns the mean squared error of the valuation.
double OptimiserAPI::meanSquaredErrorProblem1(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
	std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
	const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
	const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const std::shared_ptr<std::vector<double>>& volatilitiesToOptimise, const int& D)
{
	if (optionsPtr->size() != optionPricesPtr->size())
		throw invalid_argument("The prices do not correspond to the provided options.");

	// Construct the model object
	auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(costOfCarry, discountRate, volatilitiesToOptimise->at(0), 
		initialUnderlyingPrice,	UnderlyingCode::BHP, dividendTime, dividendAmount, jumpTime, jumpMean, volatilitiesToOptimise->at(1));

	// Construct the pricer object
	TreePricer treePricer(blackScholesSingleNormalJumpModel);

	auto meanSquareError = 0.0;
	auto averagingFactor = 1.0 / (double)optionsPtr->size();
	auto treePrice = treePricer.priceWithRichardsonExtrapolation(nTimeSteps, optionsPtr, true, Implementation::One, 6.0, -6.0);
	for (int i = 0; i < optionsPtr->size(); i++)
		meanSquareError += pow((*treePrice->at(i) - *optionPricesPtr->at(i)), 2);

	meanSquareError *= averagingFactor;
	return meanSquareError;
}


std::shared_ptr<std::vector<double>> OptimiserAPI::optimiseProblem1(const std::string & jsonInputFilePath, 
	const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
	const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const double& F, const double& CR, const int& N,
	std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed)
	
{
	// read in the JSON option inputs
	shared_ptr<vector<shared_ptr<double>>> optionPricesPtr;
	shared_ptr<vector<shared_ptr<VanillaOption>>> optionsPtr;
	tie(optionPricesPtr, optionsPtr) = AmericanOptionJSONReader(jsonInputFilePath);

	auto pricingFunctionPtr = bind(meanSquaredErrorProblem1, optionPricesPtr, optionsPtr, costOfCarry, discountRate, initialUnderlyingPrice,
		dividendTime, dividendAmount, jumpTime, jumpMean, nTimeSteps, _1, _2);

	DifferentialEvolution optimiser(2, F, CR, lowerBounds, upperBounds, pricingFunctionPtr, Implementation::One, N, seed);
	auto solution = optimiser.solve(tolerance);

	return solution;
}


// Functions for Optimisation Problem 2
// ----------------------------------------------------------------------------


// Function which revalues a set of options, and returns to mean squared error of the valuation. 
double OptimiserAPI::meanSquaredErrorProblem2(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
	std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
	const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const std::shared_ptr<std::vector<double>>& parametersToOptimise, const int& D)
{
	if (optionsPtr->size() != optionPricesPtr->size())
		throw invalid_argument("The prices do not correspond to the provided options.");

	// Construct the model object
	auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(
		parametersToOptimise->at(0), 
		parametersToOptimise->at(1), 
		parametersToOptimise->at(2), 
		initialUnderlyingPrice,	UnderlyingCode::BHP, dividendTime, 
		parametersToOptimise->at(3), 
		jumpTime, jumpMean, 
		parametersToOptimise->at(4));

	// Construct the pricer object
	TreePricer treePricer(blackScholesSingleNormalJumpModel);

	auto meanSquareError = 0.0;
	auto averagingFactor = 1.0 / (double)optionsPtr->size();
	auto treePrice = treePricer.priceWithRichardsonExtrapolation(nTimeSteps, optionsPtr, true, Implementation::One, 6.0, -6.0);
	for (int i = 0; i < optionsPtr->size(); i++)
		meanSquareError += pow((*treePrice->at(i) - *optionPricesPtr->at(i)), 2);

	meanSquareError *= averagingFactor;
	return meanSquareError;
}


std::shared_ptr<std::vector<double>> OptimiserAPI::optimiseProblem2(const std::string & jsonInputFilePath, 
	const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const double& F, const double& CR, const int& N,
	std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed)
{
	// read in the JSON option inputs
	shared_ptr<vector<shared_ptr<double>>> optionPricesPtr;
	shared_ptr<vector<shared_ptr<VanillaOption>>> optionsPtr;
	tie(optionPricesPtr, optionsPtr) = AmericanOptionJSONReader(jsonInputFilePath);

	auto pricingFunctionPtr = bind(meanSquaredErrorProblem2, optionPricesPtr, optionsPtr, initialUnderlyingPrice,
		dividendTime, jumpTime, jumpMean, nTimeSteps, _1, _2);

	DifferentialEvolution optimiser(5, F, CR, lowerBounds, upperBounds, pricingFunctionPtr, Implementation::One, N, seed);
	auto solution = optimiser.solve(tolerance);

	return solution;
}
