#include "PartA.h"
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
double PartA::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
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
double PartA::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
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
double PartA::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
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
double PartA::price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T, 
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
std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> PartA::constructVanillaOptionsPtr(const double & strike, const bool & is_call, 
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
double PartA::constructTreePrice(const std::shared_ptr<models::ITreeModel>& model, const double& timeStepSize, 
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptionsPtr)
{
	// Construct Pricer
	TreePricer treePricer(model);

	// Price option
	auto price = treePricer.priceWithRichardsonExtrapolation(timeStepSize, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
	return *price->at(0);
}