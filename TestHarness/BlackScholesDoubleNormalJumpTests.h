#ifndef __BLACKSCHOLESDOUBLENORMALJUMPTESTS_H__
#define __BLACKSCHOLESDOUBLENORMALJUMPTESTS_H__

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Enumerations/Implementation.h"
#include "../Instruments/VanillaOption.h"
#include "../Models/BlackScholesDoubleNormalJump.h"
#include "../Models/BlackScholesSingleNormalJump.h"
#include "../Pricers/MonteCarloPricer.h"
#include "../Pricers/TreePricer.h"

using namespace std;
using namespace std::chrono;
using namespace enumerations;
using namespace instruments;
using namespace models;
using namespace pricers;

namespace tests
{
	// Replicate single jump
	bool BlackScholesDoubleNormalJumpTest1()
	{
		high_resolution_clock::time_point tStart = high_resolution_clock::now();
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		// Replicating the dividend by increasing the strike

		auto dividendAmount = 10.0;
		auto dividendTime = 0.51;
		auto jumpTime = 7.0/365.0;
		auto jumpMean1 = 0.20;
		auto jumpVolatility1 = 0.3;
		auto jumpMean2 = -0.10;
		auto jumpVolatility2 = 0.4;
		//auto bernoulliProability = 0.999991;
		//auto bernoulliProability = 0.000001;
		auto bernoulliProability = 0.3;

		// Black Scholes Model with single normal jump
		//auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(costOfCarry, discountRate, impliedVolatility,
		//	initialUnderlyingPrice, underlyingCode, dividendTime, dividendAmount, jumpTime, jumpMean1, jumpVolatility1);
		auto blackScholesSingleNormalJumpModel = make_shared<models::BlackScholesSingleNormalJump>(costOfCarry, discountRate, impliedVolatility,
			initialUnderlyingPrice, underlyingCode, dividendTime, dividendAmount, jumpTime, jumpMean2, jumpVolatility2);
		auto blackScholesDoubleNormalJumpModel = make_shared<models::BlackScholesDoubleNormalJump>(costOfCarry, discountRate, impliedVolatility, 
			initialUnderlyingPrice,	underlyingCode, dividendTime, dividendAmount, jumpTime, jumpMean1, jumpVolatility1, jumpMean2, jumpVolatility2,
			bernoulliProability);

		auto vanillaOption5 = make_shared<VanillaOption>(90.0, 0.4, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption5 };
		auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));

		MonteCarloPricer monteCarloSinglePricer(blackScholesSingleNormalJumpModel);
		TreePricer treeSinglePricer(blackScholesSingleNormalJumpModel);
		MonteCarloPricer monteCarloDoublePricer(blackScholesDoubleNormalJumpModel);
		TreePricer treeDoublePricer(blackScholesDoubleNormalJumpModel);

		auto treeSinglePrices = treeSinglePricer.priceWithRichardsonExtrapolation(10, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
		auto monteCarloSinglePrices = monteCarloSinglePricer.price(5000000, vanillaOptionsPtr, 1);
		auto treeDoublePrices = treeDoublePricer.priceWithRichardsonExtrapolation(10, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
		auto monteCarloDoublePrices = monteCarloDoublePricer.price(5000000, vanillaOptionsPtr, 1);

		// Measure time
		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		std::cout << "Price time" << duration_cast<milliseconds>(tEnd - tStart).count() << std::endl;

		// Print out values

		for (int i = 0; i < vanillaOptionsPtr->size(); i++)
		{
			std::cout << "Black Scholes with SingleNormalJump for option " << i + 1 << std::endl;
			std::cout << "MonteCarlo\t" << *monteCarloSinglePrices->at(i) << std::endl;
			std::cout << "Tree\t\t" << *treeSinglePrices->at(i) << std::endl;
			std::cout << "Black Scholes with DoubleNormalJump for option " << i + 1 << std::endl;
			std::cout << "MonteCarlo\t" << *monteCarloDoublePrices->at(i) << std::endl;
			std::cout << "Tree\t\t" << *treeDoublePrices->at(i) << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
		}


		// Check values
		auto testPass = true;
		return testPass;
	}
}
#endif // !__BLACKSCHOLESDOUBLENORMALJUMPTESTS_H__
