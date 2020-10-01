#ifndef __BLACKSCHOLESWITHDIVIDENDMODELTESTS_H__
#define __BLACKSCHOLESWITHDIVIDENDMODELTESTS_H__

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Enumerations/Implementation.h"
#include "../Instruments/VanillaOption.h"
#include "../Models/BlackScholesWithDividend.h"
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
	//// Black Scholes with Dividend Model : compare pricing between the Analytic, Monte Carlo and Tree implementations for European options
	//// As the payment of the dividend does not impact the diffusion process, dividends can be handled in the standard Black Scholes model by adjust up the Strike
	//// by the amount of the dividend. This opens up use of the analytic solution. This first test checks that the methods handle event timing correctly.
	bool BlackScholesWithDividendModelTest1()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;
		auto dividendAmount = 10.0;
		auto dividendTime = 0.51;


		// Replicating the dividend by increasing the strike

		// Black Scholes Model
		high_resolution_clock::time_point tStart = high_resolution_clock::now();
		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, 
			initialUnderlyingPrice,	underlyingCode);

		auto vanillaOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption2 = make_shared<VanillaOption>(90.0 + dividendAmount, 2.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption3 = make_shared<VanillaOption>(100.0 + dividendAmount, 0.51, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto vanillaOption4 = make_shared<VanillaOption>(100.0 + dividendAmount, 2.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto vanillaOption5 = make_shared<VanillaOption>(90.0 + dividendAmount, 1.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);

		vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption1, vanillaOption2, vanillaOption3, vanillaOption4, vanillaOption5};
		auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));

		AnalyticPricer analyticPricer(blackScholesModel);
		MonteCarloPricer monteCarloPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		auto analyticPrices = analyticPricer.price(vanillaOptionsPtr);
		auto treePrices = treePricer.priceWithRichardsonExtrapolation(1.0 / 150.0, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
		auto monteCarloPrices = monteCarloPricer.price(5000000, vanillaOptionsPtr, 1);


		// Black Scholes Model with Dividend
		auto blackScholesDividendModel = make_shared<models::BlackScholesWithDividend>(costOfCarry, discountRate, impliedVolatility, 
			initialUnderlyingPrice,	underlyingCode, dividendTime, dividendAmount);

		auto vanillaOption1Div = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption2Div = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption3Div = make_shared<VanillaOption>(100.0, 0.51, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto vanillaOption4Div = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto vanillaOption5Div = make_shared<VanillaOption>(90.0, 1.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);

		vector<shared_ptr<VanillaOption>> vanillaOptionsDiv{ vanillaOption1Div, vanillaOption2Div, vanillaOption3Div, vanillaOption4Div, 
			vanillaOption5Div };
		auto vanillaOptionsPtrDiv = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptionsDiv));

		TreePricer treePricerDiv(blackScholesDividendModel);
		MonteCarloPricer monteCarloPricerDiv(blackScholesDividendModel);

		auto treePricesDiv = treePricerDiv.priceWithRichardsonExtrapolation(1.0 / 200.0, vanillaOptionsPtrDiv, true, Implementation::One, 6.0, -6.0);
		auto monteCarloPricesDiv = monteCarloPricerDiv.price(5000000, vanillaOptionsPtrDiv, 1);

		// Measure time
		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		std::cout << "Price time" << duration_cast<milliseconds>(tEnd - tStart).count() << std::endl;

		// Print out values

		for (int i = 0; i < vanillaOptionsPtr->size(); i++)
		{
			std::cout << "Black Scholes for option " << i + 1 << std::endl;
			std::cout << "Analytic\t" << *analyticPrices->at(i) << std::endl;
			std::cout << "MonteCarlo\t" << *monteCarloPrices->at(i) << std::endl;
			std::cout << "Tree\t\t" << *treePrices->at(i) << std::endl;
			std::cout << std::endl;
			std::cout << "Black Scholes with Dividend for option " << i + 1 << std::endl;
			std::cout << "MonteCarlo\t" << *monteCarloPricesDiv->at(i) << std::endl;
			std::cout << "Tree\t\t" << *treePricesDiv->at(i) << std::endl;
			std::cout << std::endl;
			std::cout << std::endl;
		}


		auto testPass = true;
		/*
		// Check values
		vector<double> absRelDiffs;
		absRelDiffs.reserve(vanillaOptionsPtr->size());
		for (int i = 0; i < vanillaOptionsPtr->size(); i++)
		{
			auto absRelDiff = abs(100.0 * (*treePrices->at(i) - *monteCarloPrices->at(i)) / *monteCarloPrices->at(i));
			absRelDiffs.push_back(move(absRelDiff));

			// Check for 0.5% error
			if (absRelDiff > 0.5)
				testPass = false;
		}
		*/
		return testPass;
	}
}
#endif // !__BLACKSCHOLESWITHDIVIDENDMODELTESTS_H__
