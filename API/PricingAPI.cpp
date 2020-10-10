#include "PricingAPI.h"
#include "../Models/BlackScholes.h"
#include "../Models/BlackScholesWithDividend.h"
#include "../Models/BlackScholesSingleNormalJump.h"
#include "../Models/BlackScholesDoubleNormalJump.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Pricers/TreePricer.h"

using namespace std;
using namespace models;
using namespace instruments;
using namespace enumerations;
using namespace pricers;

// Black Scholes price for an American option. The last parameter determines the length of the time step in the tree
double PricingAPI::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
	const double & sigma, const double& timeStepSize)
{
	// Construct the model
	auto underlyingCode = UnderlyingCode::BHP; // this is just a dummy
	auto blackScholesModel = make_shared<models::BlackScholes>(q, r, sigma, s_0, underlyingCode);

	// Construct Vanilla Option
	auto vanillaOptionsPtr = constructVanillaOptionsPtr(strike, is_call, T);

	// Construct tree and get price
	auto price = constructTreePrice(blackScholesModel, timeStepSize, vanillaOptionsPtr);
	return price;
}

// Black Scholes with Dividend price for an American option. The last parameter determines the length of the time step in the tree
double PricingAPI::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
	const double & sigma, const double& t_div, const double& div, const double& timeStepSize)
{
	// Construct the model
	auto underlyingCode = UnderlyingCode::BHP; // this is just a dummy
	auto blackScholesWithDividendModel = make_shared<models::BlackScholesWithDividend>(q, r, sigma, s_0, underlyingCode, t_div, div);

	// Construct Vanilla Option
	auto vanillaOptionsPtr = constructVanillaOptionsPtr(strike, is_call, T);

	// Construct tree and get price
	auto price = constructTreePrice(blackScholesWithDividendModel, timeStepSize, vanillaOptionsPtr);
	return price;
}

// Black Scholes with Dividend and single normal jump price for an American option. 
double PricingAPI::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
	const double & sigma, const double& t_div, const double& div, const double& jumpTime, const double& jumpMean, const double& jumpVol, 
	const double& timeStepSize)
{
	// Construct the model
	auto underlyingCode = UnderlyingCode::BHP; // this is just a dummy
	auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(q, r, sigma, s_0, underlyingCode, t_div, div,
		jumpTime, jumpMean, jumpVol);

	// Construct Vanilla Option
	auto vanillaOptionsPtr = constructVanillaOptionsPtr(strike, is_call, T);

	// Construct tree and get price
	auto price = constructTreePrice(blackScholesSingleNormalJumpModel, timeStepSize, vanillaOptionsPtr);
	return price;
}

// Black Scholes with Dividend and double normal jump price for an American option. 
double PricingAPI::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
	const double & sigma, const double& t_div, const double& div, const double& jumpTime, const double& jumpMean1, const double& jumpVol1, 
	const double& jumpMean2, const double& jumpVol2, const double& bernoulliProbability, const double& timeStepSize)
{
	// Construct the model
	auto underlyingCode = UnderlyingCode::BHP; // this is just a dummy
	auto blackScholesDoubleNormalJumpModel = make_shared<models::BlackScholesDoubleNormalJump>(q, r, sigma, s_0, underlyingCode, t_div, div,
		jumpTime, jumpMean1, jumpVol1, jumpMean2, jumpVol2, bernoulliProbability);

	// Construct Vanilla Option
	auto vanillaOptionsPtr = constructVanillaOptionsPtr(strike, is_call, T);

	// Construct tree and get price
	auto price = constructTreePrice(blackScholesDoubleNormalJumpModel, timeStepSize, vanillaOptionsPtr);
	return price;
}



// Helper function which structures the option contract into the required form for pricing
std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> PricingAPI::constructVanillaOptionsPtr(const double & strike, const bool & is_call, 
	const double & T)
{
	auto underlyingCode = UnderlyingCode::BHP; // this is just a dummy
	auto optionRight = is_call ? OptionRight::call : OptionRight::put;
	auto exerciseType = ExerciseType::american;
	auto vanillaOption = make_shared<VanillaOption>(strike, T, exerciseType, optionRight, underlyingCode);

	vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption };
	auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));
	return vanillaOptionsPtr;
}

// Helper function which constructs the pricing tree given a model, and then computes the price
double PricingAPI::constructTreePrice(const std::shared_ptr<models::ITreeModel>& model, const double& timeStepSize, 
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptionsPtr)
{
	// Construct Pricer
	TreePricer treePricer(model);

	// Price option
	auto price = treePricer.priceWithRichardsonExtrapolation(timeStepSize, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
	return *price->at(0);
}


// Functions for Pricing Problem 1
// ----------------------------------------------------------------------------


void PricingAPI::priceVanillaOptionsBlackScholesDoubleNormalJumpModel(const std::string & jsonInputFilePath, 
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

void PricingAPI::priceAmericanOptionsBlackScholesSingleNormalJumpModel(const std::string & jsonInputFilePath, const std::string & jsonOutputFilePath, const double & costOfCarry, 
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

