#ifndef __BLACKSCHOLESMODELTESTS_H__
#define __BLACKSCHOLESMODELTESTS_H__

#include <iostream>
#include <vector>
#include <memory>
#include <chrono>
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Enumerations/Implementation.h"
#include "../Instruments/VanillaOption.h"
#include "../Models/BlackScholes.h"
#include "../Pricers/MonteCarloPricer.h"
#include "../Pricers/AnalyticPricer.h"
#include "../Pricers/TreePricer.h"

using namespace std;
using namespace std::chrono;
using namespace enumerations;
using namespace instruments;
using namespace models;
using namespace pricers;

namespace tests
{
	//// Black Scholes Model : compare pricing between the Analytic, Monte Carlo and Tree implementations for European options
	bool BlackScholesModelTest1()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, initialUnderlyingPrice,
			underlyingCode);

		// Construct Vanilla Options
		auto vanillaOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption2 = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto vanillaOption3 = make_shared<VanillaOption>(110.0, 1.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto vanillaOption4 = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);

		vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption1, vanillaOption2, vanillaOption3, vanillaOption4 };
		auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));

		// Construct Pricers
		MonteCarloPricer monteCarloPricer(blackScholesModel);
		AnalyticPricer analyticPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		// Price options
		auto analyticPrices = analyticPricer.price(vanillaOptionsPtr);
		auto monteCarloPrices = monteCarloPricer.price(3000000, vanillaOptionsPtr, 1);
		auto treePricesWithoutSmoothing = treePricer.price(300, vanillaOptionsPtr, false, Implementation::One, 6.0, -6.0); // using Cox Ross Rubinstein
		auto treePricesWithSmoothingAndRichardsonExtrapolation = treePricer.priceWithRichardsonExtrapolation(100, vanillaOptionsPtr, true,
			Implementation::One, 6.0, -6.0);

		// Check values
		auto testPass = true;
		vector<double> absRelDiffsMonteCarlo;
		vector<double> absRelDiffsTreeWithoutSmoothing;
		vector<double> absRelDiffsTreeWithSmoothingAndRichardsonExtrapolation;

		absRelDiffsMonteCarlo.reserve(vanillaOptionsPtr->size());
		absRelDiffsTreeWithoutSmoothing.reserve(vanillaOptionsPtr->size());
		absRelDiffsTreeWithSmoothingAndRichardsonExtrapolation.reserve(vanillaOptionsPtr->size());

		for (int i = 0; i < vanillaOptionsPtr->size(); i++)
		{
			auto absRelDiffMonteCarlo = abs(100.0 * (*monteCarloPrices->at(i) - *analyticPrices->at(i)) / *analyticPrices->at(i));

			auto absRelDiffTreeWithoutSmoothing = abs(100.0 * (*treePricesWithoutSmoothing->at(i) - *analyticPrices->at(i)) / *analyticPrices->at(i));

			auto absRelDiffTreeWithSmoothingAndRichardsonExtrapolation = abs(100.0 *
				(*treePricesWithSmoothingAndRichardsonExtrapolation->at(i) - *analyticPrices->at(i)) / *analyticPrices->at(i));

			absRelDiffsMonteCarlo.push_back(move(absRelDiffMonteCarlo));
			absRelDiffsTreeWithoutSmoothing.push_back(move(absRelDiffTreeWithoutSmoothing));
			absRelDiffsTreeWithSmoothingAndRichardsonExtrapolation.push_back(move(absRelDiffTreeWithSmoothingAndRichardsonExtrapolation));

			// Check for 0.3% error
			if (absRelDiffMonteCarlo > 0.3) {
				testPass = false;
				std::cout << "Monte Carlo Price: " << *monteCarloPrices->at(i) 
					<< "\t Analytic Price:" << *analyticPrices->at(i)
					<< "\t Relative Difference:" << absRelDiffMonteCarlo 
					<< std::endl;
			}
			if (absRelDiffTreeWithoutSmoothing > 0.3) {
				testPass = false;
				std::cout << "Non Smooth Tree Price: " << *treePricesWithoutSmoothing->at(i) 
					<< "\t Analytic Price:" << *analyticPrices->at(i)
					<< "\t Relative Difference:" << absRelDiffTreeWithoutSmoothing
					<< std::endl;
			}
			if (absRelDiffTreeWithSmoothingAndRichardsonExtrapolation > 0.3) {
				testPass = false;
				std::cout << "Smooth Tree Price: " << *treePricesWithSmoothingAndRichardsonExtrapolation->at(i) 
					<< "\t Analytic Price:" << *analyticPrices->at(i)
					<< "\t Relative Difference:" << absRelDiffTreeWithSmoothingAndRichardsonExtrapolation
					<< std::endl;
			}
		}

		return testPass;
	}

	//// Black Scholes Model : sanity check on smoothing functionality for tree model
	//// Price a single European option on a tree with smoothing, and only a single time step. In this case, the tree model should return 
	//// exactly the Black Scholes closed form solution.
	bool BlackScholesModelTest2()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, initialUnderlyingPrice,
			underlyingCode);

		// Construct Vanilla Options
		auto vanillaOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption1 };
		auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));

		// Construct Pricers
		AnalyticPricer analyticPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		// Price options
		auto analyticPrices = analyticPricer.price(vanillaOptionsPtr);
		auto treePricesWithSmoothing = treePricer.price(1, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0); // using Cox Ross Rubinstein

		// Check values
		auto testPass = true;
		auto absoluteRelativeDifference = 100.0 * (*treePricesWithSmoothing->at(0) - *analyticPrices->at(0)) / *analyticPrices->at(0);
		if (absoluteRelativeDifference > 0.001)
			testPass = false;

		return testPass;
	}


	//// Black Scholes Model : Comparision of European call analytic solution against American call tree solution. It's never optimal to exercise an American
	// call option, hence, it's price should be the same as an otherwise equivalent European call option. 
	bool BlackScholesModelTest3()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, initialUnderlyingPrice,
			underlyingCode);

		// Construct Vanilla Options
		auto europeanOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto europeanOption2 = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto europeanOption3 = make_shared<VanillaOption>(110.0, 1.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		auto europeanOption4 = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::european, OptionRight::call, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> europeanOptions{ europeanOption1, europeanOption2, europeanOption3, europeanOption4 };
		auto europeanOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(europeanOptions));

		auto americanOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::american, OptionRight::call, UnderlyingCode::BHP);
		auto americanOption2 = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::american, OptionRight::call, UnderlyingCode::BHP);
		auto americanOption3 = make_shared<VanillaOption>(110.0, 1.0, ExerciseType::american, OptionRight::call, UnderlyingCode::BHP);
		auto americanOption4 = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::american, OptionRight::call, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> americanOptions{ americanOption1, americanOption2, americanOption3, americanOption4 };
		auto americanOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(americanOptions));

		// Construct Pricers
		AnalyticPricer analyticPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		// Price options
		auto europeanAnalyticPrices = analyticPricer.price(europeanOptionsPtr);
		auto europeanTreePrices = treePricer.priceWithRichardsonExtrapolation(100, europeanOptionsPtr, true, Implementation::One, 6.0, -6.0);
		auto americanTreePrices = treePricer.priceWithRichardsonExtrapolation(100, americanOptionsPtr, true, Implementation::One, 6.0, -6.0);

		// Check values
		auto testPass = true;
		vector<double> absRelDiffsEuropeanTree;
		vector<double> absRelDiffsAmericanTree;
		vector<double> absRelDiffsEuropeanAmericanTree;
		absRelDiffsEuropeanTree.reserve(europeanOptionsPtr->size());
		absRelDiffsAmericanTree.reserve(americanOptionsPtr->size());
		absRelDiffsEuropeanAmericanTree.reserve(americanOptionsPtr->size());
		for (int i = 0; i < europeanOptionsPtr->size(); i++)
		{
			auto absRelDiffEuropeanTree = abs(100.0 * (*europeanTreePrices->at(i) - *europeanAnalyticPrices->at(i)) / *europeanAnalyticPrices->at(i));
			auto absRelDiffAmericanTree = abs(100.0 * (*americanTreePrices->at(i) - *europeanAnalyticPrices->at(i)) / *europeanAnalyticPrices->at(i));
			auto absRelDiffEuropeanAmericanTree = abs(100.0 * (*americanTreePrices->at(i) - *europeanTreePrices->at(i)) / *europeanTreePrices->at(i));

			absRelDiffsEuropeanTree.push_back(move(absRelDiffEuropeanTree));
			absRelDiffsAmericanTree.push_back(move(absRelDiffAmericanTree));
			absRelDiffsEuropeanAmericanTree.push_back(move(absRelDiffEuropeanAmericanTree));

			// Check for 0.3% error
			if (absRelDiffEuropeanTree > 0.3 || absRelDiffAmericanTree > 0.3 || absRelDiffEuropeanAmericanTree > 0.0001)
				testPass = false;
			// Lower threshold for the last because the prices should match. For the first two, there will be some simulation error. 
		}
		return testPass;
	}

	//// Black Scholes Model : Comparision of European put analytic solution against American put tree solution. In all case the American should
	//// be greater in value
	bool BlackScholesModelTest4()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, initialUnderlyingPrice,
			underlyingCode);

		// Construct Vanilla Options
		auto europeanOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto europeanOption2 = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto europeanOption3 = make_shared<VanillaOption>(110.0, 1.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		auto europeanOption4 = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> europeanOptions{ europeanOption1, europeanOption2, europeanOption3, europeanOption4 };
		auto europeanOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(europeanOptions));

		auto americanOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::american, OptionRight::put, UnderlyingCode::BHP);
		auto americanOption2 = make_shared<VanillaOption>(90.0, 2.0, ExerciseType::american, OptionRight::put, UnderlyingCode::BHP);
		auto americanOption3 = make_shared<VanillaOption>(110.0, 1.0, ExerciseType::american, OptionRight::put, UnderlyingCode::BHP);
		auto americanOption4 = make_shared<VanillaOption>(100.0, 2.0, ExerciseType::american, OptionRight::put, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> americanOptions{ americanOption1, americanOption2, americanOption3, americanOption4 };
		auto americanOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(americanOptions));

		// Construct Pricers
		AnalyticPricer analyticPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		// Price options
		auto europeanAnalyticPrices = analyticPricer.price(europeanOptionsPtr);
		auto americanTreePrices = treePricer.priceWithRichardsonExtrapolation(100, americanOptionsPtr, true, Implementation::One, 6.0, -6.0);

		// Check values
		auto testPass = true;
		vector<double> diffsAmericanTree;
		diffsAmericanTree.reserve(americanOptionsPtr->size());
		for (int i = 0; i < europeanOptionsPtr->size(); i++)
		{
			auto diffAmericanTree = *americanTreePrices->at(i) - *europeanAnalyticPrices->at(i);
			diffsAmericanTree.push_back(move(diffAmericanTree));

			// Check for whether the American put is less valuable than the European put
			if (diffAmericanTree < 0.000000001)
				testPass = false;
		}
		return testPass;
	}

	//// Black Scholes Model : convergence comparison of Monte Carlo, CRR Tree and Tian Tree 
	bool BlackScholesModelPerformanceTest()
	{
		// Construct Model
		auto costOfCarry = 0.03;
		auto discountRate = 0.06;
		auto impliedVolatility = 0.1;
		auto initialUnderlyingPrice = 100.0;
		auto underlyingCode = UnderlyingCode::BHP;

		auto blackScholesModel = make_shared<models::BlackScholes>(costOfCarry, discountRate, impliedVolatility, initialUnderlyingPrice,
			underlyingCode);

		// Construct Vanilla Options
		auto vanillaOption1 = make_shared<VanillaOption>(105.0, 0.5, ExerciseType::european, OptionRight::put, UnderlyingCode::BHP);
		vector<shared_ptr<VanillaOption>> vanillaOptions{ vanillaOption1 };
		auto vanillaOptionsPtr = make_shared < vector<shared_ptr<VanillaOption>>>(move(vanillaOptions));

		// Construct Pricers
		MonteCarloPricer monteCarloPricer(blackScholesModel);
		AnalyticPricer analyticPricer(blackScholesModel);
		TreePricer treePricer(blackScholesModel);

		// Analytic pricing
		auto analyticPrices = analyticPricer.price(vanillaOptionsPtr);
		std::cout << "Analytic Price \t" << *analyticPrices->at(0) << std::endl;

		// Monte Carlo Pricing
		for (int i = 1; i < 250; i++)
		{
			high_resolution_clock::time_point tStart = high_resolution_clock::now();
			auto monteCarloPrices = monteCarloPricer.price(1200 * i, vanillaOptionsPtr, 1);
			high_resolution_clock::time_point tEnd = high_resolution_clock::now();
			std::cout << "Monte Carlo Price;" << *monteCarloPrices->at(0)
				<< ";time;" << duration_cast<milliseconds>(tEnd - tStart).count()
				<< std::endl;
		}

		// Cox Ross Rubinstein Tree Pricing
		for (int i = 1; i < 250; i++)
		{
			high_resolution_clock::time_point tStart = high_resolution_clock::now();
			auto treePrices = treePricer.price(1.0 / (double)i, vanillaOptionsPtr, false, Implementation::One, 6.0, -6.0);
			high_resolution_clock::time_point tEnd = high_resolution_clock::now();
			std::cout << "CRR;" << *treePrices->at(0)
				<< ";time;" << duration_cast<milliseconds>(tEnd - tStart).count()
				<< std::endl;
		}

		// Cox Ross Rubinstein Tree Pricing with Smoothing and Richardson Extrapolation
		for (int i = 1; i < 250; i++)
		{
			high_resolution_clock::time_point tStart = high_resolution_clock::now();
			auto treePrices = treePricer.priceWithRichardsonExtrapolation(1.0 / (double)i, vanillaOptionsPtr, true, Implementation::One, 6.0, -6.0);
			high_resolution_clock::time_point tEnd = high_resolution_clock::now();
			std::cout << "CRR, Smoothing, Richardson Tree Price;" << *treePrices->at(0)
				<< ";time;" << duration_cast<milliseconds>(tEnd - tStart).count()
				<< std::endl;
		}

		// Tian Tree Pricing with Smoothing and Richardson Extrapolation
		for (int i = 1; i < 250; i++)
		{
			high_resolution_clock::time_point tStart = high_resolution_clock::now();
			auto treePrices = treePricer.priceWithRichardsonExtrapolation(1.0 / (double)i, vanillaOptionsPtr, true, Implementation::Two, 6.0, -6.0);
			high_resolution_clock::time_point tEnd = high_resolution_clock::now();
			std::cout << "Tian, Smoothing, Richardson Tree Price;" << *treePrices->at(0)
				<< ";time;" << duration_cast<milliseconds>(tEnd - tStart).count()
				<< std::endl;
		}

		auto testPass = true;
		return testPass;
	}
}
#endif // !__BLACKSCHOLESMODELTESTS_H__
