#include <cmath>
#include <random>
#include <boost/math/distributions/normal.hpp>
#include "BlackScholes.h"
#include "TreeModelUtilities/LogNormalDiffusionTreeHelper.h"

using namespace std;
using namespace models;
using namespace enumerations;

models::BlackScholes::BlackScholes(const double costOfCarry, const double discountRate, const double impliedVolatility, 
	const double initialUnderlyingPrice, const enumerations::UnderlyingCode underlyingCode)
{
	setCostOfCarry(costOfCarry);
	setDiscountRate(discountRate);
	setImpliedVolatility(impliedVolatility);
	setInitialUnderlyingPrice(initialUnderlyingPrice);
	setUnderlyingCode(underlyingCode);
}

void models::BlackScholes::setImpliedVolatility(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The implied volatility must be positive.");
	m_impliedVolatility = value;
}

void models::BlackScholes::setInitialUnderlyingPrice(const double& value)
{
	/* HACK!!!!
	if (value < 0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
		*/
	if (value < -0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
	m_initialUnderlyingPrice = value;
}


//// Functions for the exact solution for the price of a vanilla option
// ============================================================================ 

// Classical Black Scholes Equation for the price of a European option
const std::shared_ptr<double> models::BlackScholes::calculateAnalyticSolution(const double& strike, const double& timeToExpiry,
	const OptionRight& optionRight, const ExerciseType& exerciseType)
{
	// Input validation
	if (exerciseType != ExerciseType::european)
		throw invalid_argument("This model only supports European vanilla options.");

	// Construct a standard normal distribution
	boost::math::normal z; // normal variate with mean 0 and variance 1. Using boost here as normal inversion is not in std

	// Compute d1 and d2
	auto d1 = (log(m_initialUnderlyingPrice / strike) + timeToExpiry * (m_discountRate - m_costOfCarry + pow(m_impliedVolatility, 2) / 2.0)) / 
		(m_impliedVolatility*sqrt(timeToExpiry));
	auto d2 = d1 - m_impliedVolatility * sqrt(timeToExpiry);

	// Price 
	auto callSign = optionRight == OptionRight::call ? 1.0 : -1.0;
	auto price = callSign * exp(-1. * m_discountRate * timeToExpiry) * (
		m_initialUnderlyingPrice * exp((m_discountRate - m_costOfCarry) * timeToExpiry) * boost::math::cdf(z, callSign * d1) - 
		strike * boost::math::cdf(z, callSign * d2)
		);
	auto pricePtr = make_shared<double>(move(price));
	return pricePtr;
}


//// Functions for the Monte Carlo simulation of vanilla option payoffs
// ============================================================================ 

//// Monte carlo simulation of the underlying price at a future time 
const std::shared_ptr<vector<double>> models::BlackScholes::generateMonteCarloSimulations(const int& nPaths, const double& time, int seed)
{
	// Construct a standard normal distribution
	if (seed == -1) // if the seed is equal to -1, then generate a random seed
	{
		random_device rd;
		seed = rd();
	}

	mt19937 mersenneTwisterEngine(seed);
	normal_distribution<> normalDistribution{ 0,1 };

	// Simulate normals, and calculate the underlying price
	vector<double> simulatedPrices;
	simulatedPrices.reserve(nPaths);
	for (int i = 0; i < nPaths; i++)
	{
		auto simulatedNormal = normalDistribution(mersenneTwisterEngine);
		auto simulatedPrice = m_initialUnderlyingPrice * exp((m_discountRate - m_costOfCarry - 0.5 * pow(m_impliedVolatility, 2)) * time
			+ m_impliedVolatility * sqrt(time) * simulatedNormal);
		simulatedPrices.push_back(simulatedPrice);
	}
	auto simulatedPricesPtr = make_shared<vector<double>>(move(simulatedPrices));
	return simulatedPricesPtr;
}


//// Functions for the construction trees
// ============================================================================ 

//// Constructs the binomial recombining tree
//// The main difference between the classical Cox Ross Rubinstein method and the Tian method is that the up and down multipliers
//// are determined by an additional degree of moment matching. The probabilities of the up and down states occuring in both methods
//// is the same. Implementation One use Cox Ross Rubinstein, whilst Implementation Two uses Tian.
const std::shared_ptr<models::Tree> models::BlackScholes::constructTree(const int& nTimeSteps, const double& timeToExpiry, 
	const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation)
{
	// Calculate the state and probabilities for diffusion
	// ---------------------------------------------------------------------------
	// As the time steps are of the same size throughout the tree, the multipliers and probabilities are invariant with time.
	auto timeStepSize = timeToExpiry / (double)nTimeSteps;

	shared_ptr<vector<double>> diffusionStatesPtr, diffusionProbabilitiesPtr;
	tie(diffusionStatesPtr, diffusionProbabilitiesPtr) = LogNormalDiffusionTreeHelper::calculateDiffusionStatesAndProbabilities(timeStepSize, 
		m_impliedVolatility, m_discountRate, m_costOfCarry, implementation);

	// This model does not support dividends. Hence, set dividend amount to 0, and the dividend payment date to after the time to expiry
	auto dividendTime = timeToExpiry + 1.0;
	auto dividendAmount = 0.0;
	auto treeNodesPtr = LogNormalDiffusionTreeHelper::constructRecombiningTree(m_initialUnderlyingPrice, nTimeSteps, timeToExpiry, m_impliedVolatility,
		upperLimitStandardDeviation, lowerLimitStandardDeviation, dividendTime, dividendAmount, diffusionStatesPtr, diffusionProbabilitiesPtr);
	auto treePtr = make_shared<Tree>(treeNodesPtr, nTimeSteps, timeToExpiry);
	return treePtr;
}


//// Calculates the option value at the current tree node with smoothing
const std::shared_ptr<double> models::BlackScholes::smoothedValueAtTreeNode(const double underlyingPrice, 
	const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize)
{
	// need to construct a new Black Scholes model with a undelrying asset price as at the current tree node. First make copies of the parameters.
	BlackScholes tempBlackScholes(m_costOfCarry, m_discountRate, m_impliedVolatility, underlyingPrice, m_underlyingCode);
	auto forwardValue = tempBlackScholes.calculateAnalyticSolution(
		vanillaOption->getStrike(),
		timeStepSize,
		vanillaOption->getOptionRight(),
		ExerciseType::european); // at the second last time point, American options are effectively European
	auto valuePtr = make_shared<double>(*forwardValue);
	return valuePtr;
}

const bool models::BlackScholes::supportsVanillaOptionSmoothing(const double & timeStart, const double & timeEnd)
{
	return m_supportsVanillaOptionSmoothing;
}



