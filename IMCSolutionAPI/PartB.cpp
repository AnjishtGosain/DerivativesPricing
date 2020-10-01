#include "PartB.h"
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


// Functions for Part B Question 5
// ----------------------------------------------------------------------------


void PartB::priceVanillaOptionsBlackScholesDoubleNormalJumpModel(const std::string & jsonInputFilePath, 
	const std::string & jsonOutputFilePath,  
	const double & initialUnderlyingPrice, const double & jumpMean, const int & nTimeSteps)
{
	// read in the JSON model and option values inputs
	auto modelOptionsPtr = VanillaOptionBlackScholesDoubleNormalJSONReader(jsonInputFilePath);

	// Group the options by Double Normal Jump model
	unordered_map<double, // Dividend amount
		unordered_map<double, // normal shift amount
		unordered_map<double, // cost of carry
		unordered_map<double, // discount rate
		unordered_map<double, // implied volatility 
		unordered_map<double, // jump volatility 
		unordered_map<double, // dividend time 
		unordered_map<double, // jump time 
		unordered_map<double, // bernoulli probability 
		unordered_map<int, shared_ptr<VanillaOption>>>>>>>>>>> options; // index

	for (auto it = modelOptionsPtr->begin(); it != modelOptionsPtr->end(); it++)
	{
		auto key = it->first;
		auto point = it->second;
		options[get<0>(point)->getDividendAmount()]
			[get<0>(point)->getJumpMean1()]
			[get<0>(point)->getCostOfCarry()]
			[get<0>(point)->getDiscountRate()]
			[get<0>(point)->getImpliedVolatility()]
			[get<0>(point)->getJumpVolatility1()]
			[get<0>(point)->getDividendTime()]
			[get<0>(point)->getJumpTime()]
			[get<0>(point)->getBernoulliProbability()]
			.emplace(key, get<1>(point));
	}

	// Initialise the container for the option prices
	unordered_map<int, shared_ptr<double>> optionPrices;

	// For each model, price the set of options
	for (auto it1 = options.begin(); it1 != options.end(); it1++)
	{
		auto dividendAmount = it1->first;
		auto options1 = it1->second;
		for (auto it2 = options1.begin(); it2 != options1.end(); it2++)
		{
			auto shiftAmount = it2->first;
			auto options2 = it2->second;
			for (auto it3 = options2.begin(); it3 != options2.end(); it3++)
			{
				auto costOfCarry = it3->first;
				auto options3 = it3->second;
				for (auto it4 = options3.begin(); it4 != options3.end(); it4++)
				{
					auto discountRate = it4->first;
					auto options4 = it4->second;
					for (auto it5 = options4.begin(); it5 != options4.end(); it5++)
					{
						auto impliedVolatility = it5->first;
						auto options5 = it5->second;
						for (auto it6 = options5.begin(); it6 != options5.end(); it6++)
						{
							auto jumpVolatility = it6->first;
							auto options6 = it6->second;
							for (auto it7 = options6.begin(); it7 != options6.end(); it7++)
							{
								auto dividendTime = it7->first;
								auto options7 = it7->second;
								for (auto it8 = options7.begin(); it8 != options7.end(); it8++)
								{
									auto jumpTime = it8->first;
									auto options8 = it8->second;
									for (auto it9 = options8.begin(); it9 != options8.end(); it9++)
									{
										auto bernoulliProbability = it9->first;
										auto options9 = it9->second;

										// Construct the index and option vectors for pricing
										auto nOptions = options9.size();
										vector<int> optionsIndex;
										vector<shared_ptr<VanillaOption>> options10;
										optionsIndex.reserve(nOptions);
										options10.reserve(nOptions);
										for (auto it10 = options9.begin(); it10 != options9.end(); it10++)
										{
											optionsIndex.push_back(it10->first);
											options10.push_back(it10->second);
										}

										// Construct the model object - need to make copies of the underlying price and jump mean
										auto uPrice = initialUnderlyingPrice;
										auto jMean1 = jumpMean + shiftAmount;
										auto jMean2 = jumpMean - shiftAmount;
										auto jVol1 = jumpVolatility;
										auto jVol2 = jumpVolatility;

										auto blackScholesDoubleNormalJumpModel = make_shared<models::BlackScholesDoubleNormalJump>(
											costOfCarry, discountRate, impliedVolatility, uPrice, UnderlyingCode::BHP, dividendTime,
											dividendAmount, jumpTime, jMean1, jVol1, jMean2, jVol2, bernoulliProbability);

										// Construct the pricer object
										TreePricer treePricer(blackScholesDoubleNormalJumpModel);

										vector<shared_ptr<double>> treePrices;
										auto options10Ptr = make_shared<vector<shared_ptr<VanillaOption>>>(options10);
										auto treePricesPtr = treePricer.priceWithRichardsonExtrapolation(nTimeSteps, options10Ptr,
											true, Implementation::One, 6.0, -6.0);

										// Append the option prices to the output vector
										for (auto i = 0; i < nOptions; i++)
											optionPrices.emplace(optionsIndex.at(i), treePricesPtr->at(i));
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// write out the calculated values
	auto optionPricesPtr = make_shared<unordered_map<int, shared_ptr<double>>>(move(optionPrices));
	VanillaOptionBlackScholesDoubleNormalJSONWriter(optionPricesPtr, modelOptionsPtr, jsonOutputFilePath);

	return;
}

void PartB::priceAmericanOptionsBlackScholesSingleNormalJumpModel(const std::string & jsonInputFilePath, const std::string & jsonOutputFilePath, const double & costOfCarry, 
	const double & discountRate, const double & impliedVolatility, const double & initialUnderlyingPrice, const double & dividendTime, 
	const double & dividendAmount, const double & jumpTime, const double & jumpMean, const double & jumpVolatility, const int & nTimeSteps)
{
	// read in the JSON option inputs
	shared_ptr<vector<shared_ptr<double>>> originalOptionPricesPtr;
	shared_ptr<vector<shared_ptr<VanillaOption>>> optionsPtr;
	tie(originalOptionPricesPtr, optionsPtr) = AmericanOptionJSONReader(jsonInputFilePath);

	// Construct the model object
	auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(costOfCarry, discountRate, impliedVolatility, 
		initialUnderlyingPrice,	UnderlyingCode::BHP, dividendTime, dividendAmount, jumpTime, jumpMean, jumpVolatility);

	// Construct the pricer object
	TreePricer treePricer(blackScholesSingleNormalJumpModel);

	vector<shared_ptr<double>> treePrices;
	auto treePricesPtr = treePricer.priceWithRichardsonExtrapolation(nTimeSteps, optionsPtr, true, Implementation::One, 6.0, -6.0);

	// write out the calculated values
	AmericanOptionJSONWriter(treePricesPtr, optionsPtr, jsonOutputFilePath);
	return;
}


// Functions for Part B Question 6
// ----------------------------------------------------------------------------

// Function which revalues a set of options, and returns to mean squared error of the valuation.
double PartB::meanSquaredErrorPartB6(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
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


std::shared_ptr<std::vector<double>> PartB::optimisePartB6(const std::string & jsonInputFilePath, 
	const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
	const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const double& F, const double& CR, const int& N,
	std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed)
	
{
	// read in the JSON option inputs
	shared_ptr<vector<shared_ptr<double>>> optionPricesPtr;
	shared_ptr<vector<shared_ptr<VanillaOption>>> optionsPtr;
	tie(optionPricesPtr, optionsPtr) = AmericanOptionJSONReader(jsonInputFilePath);

	auto pricingFunctionPtr = bind(meanSquaredErrorPartB6, optionPricesPtr, optionsPtr, costOfCarry, discountRate, initialUnderlyingPrice,
		dividendTime, dividendAmount, jumpTime, jumpMean, nTimeSteps, _1, _2);

	DifferentialEvolution optimiser(2, F, CR, lowerBounds, upperBounds, pricingFunctionPtr, Implementation::One, N, seed);
	auto solution = optimiser.solve(tolerance);

	return solution;
}


// Functions for Part B Question 7
// ----------------------------------------------------------------------------


// Function which revalues a set of options, and returns to mean squared error of the valuation. Used in part B7
double PartB::meanSquaredErrorPartB7(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
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


std::shared_ptr<std::vector<double>> PartB::optimisePartB7(const std::string & jsonInputFilePath, 
	const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
	const double& F, const double& CR, const int& N,
	std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed)
{
	// read in the JSON option inputs
	shared_ptr<vector<shared_ptr<double>>> optionPricesPtr;
	shared_ptr<vector<shared_ptr<VanillaOption>>> optionsPtr;
	tie(optionPricesPtr, optionsPtr) = AmericanOptionJSONReader(jsonInputFilePath);

	auto pricingFunctionPtr = bind(meanSquaredErrorPartB7, optionPricesPtr, optionsPtr, initialUnderlyingPrice,
		dividendTime, jumpTime, jumpMean, nTimeSteps, _1, _2);

	DifferentialEvolution optimiser(5, F, CR, lowerBounds, upperBounds, pricingFunctionPtr, Implementation::One, N, seed);
	auto solution = optimiser.solve(tolerance);

	return solution;
}
