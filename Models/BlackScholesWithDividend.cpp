#include <cmath>
#include <random>
#include <math.h>
#include <boost/math/distributions/normal.hpp>
#include "BlackScholesWithDividend.h"
#include "BlackScholes.h"
#include "TreeModelUtilities/LogNormalDiffusionTreeHelper.h"

using namespace std;
using namespace models;
using namespace enumerations;

models::BlackScholesWithDividend::BlackScholesWithDividend(const double costOfCarry, const double discountRate, const double impliedVolatility, 
	const double initialUnderlyingPrice, const enumerations::UnderlyingCode underlyingCode, const double dividendTime, const double dividendAmount)
{
	setCostOfCarry(costOfCarry);
	setDiscountRate(discountRate);
	setImpliedVolatility(impliedVolatility);
	setInitialUnderlyingPrice(initialUnderlyingPrice);
	setUnderlyingCode(underlyingCode);
	setDividendTime(dividendTime);
	setDividendAmount(dividendAmount);
}

void models::BlackScholesWithDividend::setImpliedVolatility(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The implied volatility must be positive.");
	m_impliedVolatility = value;
}

void models::BlackScholesWithDividend::setInitialUnderlyingPrice(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
	m_initialUnderlyingPrice = value;
}

void models::BlackScholesWithDividend::setDividendTime(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The time of the dividend payment must be greater than 0.");
	m_dividendTime = value;
}

void models::BlackScholesWithDividend::setDividendAmount(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
	m_dividendAmount = value;
}


//// Functions for the Monte Carlo simulation of vanilla option payoffs
// ============================================================================ 

//// Monte carlo simulation of the underlying price at a future time 
const std::shared_ptr<vector<double>> models::BlackScholesWithDividend::generateMonteCarloSimulations(const int& nPaths, const double& time, 
	int seed)
{
	// Construct a standard normal distribution
	auto modSeed = seed;
	if (seed == -1) // if the seed is equal to -1, then generate a random seed
	{
		random_device rd;
		modSeed = rd();
	}

	mt19937 mersenneTwisterEngine(modSeed);
	normal_distribution<> normalDistribution{ 0,1 };

	// Simulate normals, and calculate the underlying price
	vector<double> simulatedPrices;
	simulatedPrices.reserve(nPaths);
	// Need to do a two stage simulation. First at the dividend date, and then till expiry
	vector<double> simulationInterval;
	if (m_dividendTime < time + 0.00000001)
	{
		simulationInterval.push_back(m_dividendTime);
		simulationInterval.push_back(time - m_dividendTime);
	}
	else
	{
		simulationInterval.push_back(time);
	}
	vector<double> initialPrices(nPaths, m_initialUnderlyingPrice);
	for (int i = 0; i < nPaths; i++)
	{
		for (int j = 0; j < simulationInterval.size(); j++)
		{
			auto simulatedNormal = normalDistribution(mersenneTwisterEngine);
			auto simulatedPrice = initialPrices.at(i) * exp((m_discountRate - m_costOfCarry - 0.5 * pow(m_impliedVolatility, 2)) * simulationInterval[j]
				+ m_impliedVolatility * sqrt(simulationInterval[j]) * simulatedNormal);
			if (j == 0 && m_dividendTime < time + 0.00000001 && simulatedPrice - m_dividendAmount < 0.00000001)
				simulatedPrice = 0.0;
			initialPrices.at(i) = simulatedPrice;
		}
		if (m_dividendTime < time + 0.00000001) // deduct the dividend at the very end
			initialPrices.at(i) -= m_dividendAmount;
		simulatedPrices.push_back(fmax(0.0, initialPrices.at(i)));
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

//// The dividend is a deterministic value, the payoff of which does not impact the dynamics of CRR or Tian methods. For all nodes after
//// the dividend payment date, the dividend can simply be deducted from the value at the node. Note that his has to be done after the tree has 
//// already been constructed, because the payment of the dividend does not impact the diffusion process.

const std::shared_ptr<models::Tree> models::BlackScholesWithDividend::constructTree(const int& nTimeSteps, const double& timeToExpiry, 
	const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation)

{
	// Calculate the state and probabilities for diffusion
	// ---------------------------------------------------------------------------
	// As the time steps are of the same size throughout the tree, the multipliers and probabilities are invariant with time.
	auto timeStepSize = timeToExpiry / (double)nTimeSteps;

	shared_ptr<vector<double>> diffusionStatesPtr, diffusionProbabilitiesPtr;
	tie(diffusionStatesPtr, diffusionProbabilitiesPtr) = LogNormalDiffusionTreeHelper::calculateDiffusionStatesAndProbabilities(timeStepSize, 
		m_impliedVolatility, m_discountRate, m_costOfCarry, implementation);

	auto treeNodesPtr = LogNormalDiffusionTreeHelper::constructRecombiningTree(m_initialUnderlyingPrice, nTimeSteps, timeToExpiry, m_impliedVolatility,
		upperLimitStandardDeviation, lowerLimitStandardDeviation, m_dividendTime, m_dividendAmount, diffusionStatesPtr, diffusionProbabilitiesPtr);
	auto treePtr = make_shared<Tree>(treeNodesPtr, nTimeSteps, timeToExpiry);
	return treePtr;
}


//// Calculates the option value at the current tree node with smoothing
const std::shared_ptr<double> models::BlackScholesWithDividend::smoothedValueAtTreeNode(const double underlyingPrice, 
	const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize)
{
	// This function is only called from with TreePricer. If the dividend was paid prior to the second last time in the grid, then its value
	// has already been deducted from the underlying price. However, if the dividend falls after the second last time, but before option expiry, 
	// then smoothing is not implemented because it would involve an adjustement of the Strike.

	// need to construct a new Black Scholes model with a undelrying asset price as at the current tree node. 
	BlackScholes tempBlackScholes(m_costOfCarry, m_discountRate, m_impliedVolatility, underlyingPrice, m_underlyingCode);
	auto forwardValue = tempBlackScholes.calculateAnalyticSolution(
		vanillaOption->getStrike(),
		timeStepSize,
		vanillaOption->getOptionRight(),
		ExerciseType::european); // at the second last time point, American options are effectively European
	auto valuePtr = make_shared<double>(*forwardValue);
	return valuePtr;
}

const bool models::BlackScholesWithDividend::supportsVanillaOptionSmoothing(const double & timeStart, const double & timeEnd)
{
	// If the dividend falls after the second last time, but before option expiry, then no smoothing
	auto smoothing = (m_dividendTime > timeStart - 0.00000001) && (m_dividendTime < timeEnd + 0.00000001) ? false : m_supportsVanillaOptionSmoothing; 
	return smoothing;
}



