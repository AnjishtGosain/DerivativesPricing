#include <cmath>
#include <random>
#include <math.h>
#include <boost/math/distributions/normal.hpp>
#include "BlackScholesSingleNormalJump.h"
#include "BlackScholes.h"
#include "TreeModelUtilities/LogNormalDiffusionTreeHelper.h"

using namespace std;
using namespace models;
using namespace enumerations;

models::BlackScholesSingleNormalJump::BlackScholesSingleNormalJump(const double& costOfCarry, const double& discountRate, 
	const double& impliedVolatility, const double& initialUnderlyingPrice, const enumerations::UnderlyingCode& underlyingCode, 
	const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const double& jumpVolatility)
{
	setCostOfCarry(costOfCarry);
	setDiscountRate(discountRate);
	setImpliedVolatility(impliedVolatility);
	setInitialUnderlyingPrice(initialUnderlyingPrice);
	setUnderlyingCode(underlyingCode);
	setDividendTime(dividendTime);
	setDividendAmount(dividendAmount);
	setJumpTime(jumpTime);
	setJumpMean(jumpMean);
	setJumpVolatility(jumpVolatility);
}

void models::BlackScholesSingleNormalJump::setImpliedVolatility(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The implied volatility must be positive.");
	m_impliedVolatility = value;
}

void models::BlackScholesSingleNormalJump::setJumpVolatility(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The jump volatility must be positive.");
	m_jumpVolatility = value;
}

void models::BlackScholesSingleNormalJump::setInitialUnderlyingPrice(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
	m_initialUnderlyingPrice = value;
}

void models::BlackScholesSingleNormalJump::setDividendTime(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The time of the dividend payment must be greater than 0.");
	m_dividendTime = value;
}

void models::BlackScholesSingleNormalJump::setJumpTime(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The time of the jump must be positive.");
	m_jumpTime = value;
}


void models::BlackScholesSingleNormalJump::setDividendAmount(const double& value)
{
	if (value < 0.00000001)
		throw invalid_argument("The initial underlying price must be positive.");
	m_dividendAmount = value;
}


//// Functions for the Monte Carlo simulation of vanilla option payoffs
// ============================================================================ 

//// Monte carlo simulation of the underlying price at a future time 
const std::shared_ptr<vector<double>> models::BlackScholesSingleNormalJump::generateMonteCarloSimulations(const int& nPaths, const double& time, 
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
	vector<double> simulationInterval;
	if (m_dividendTime < time + 0.00000001 && m_jumpTime < m_dividendTime)
	{
		simulationInterval.push_back(m_jumpTime);
		simulationInterval.push_back(m_dividendTime - m_jumpTime);
		simulationInterval.push_back(time - m_dividendTime);
	}
	else
	{
		simulationInterval.push_back(time);
	}

	vector<double> initialPrices(nPaths, m_initialUnderlyingPrice);
	for (int i = 0; i < nPaths; i++)
	{
		auto drift = m_discountRate - m_costOfCarry - 0.5 * pow(m_impliedVolatility, 2);
		auto simulatedJump = 0.0;
		auto simulatedNormal = 0.0;
		auto simulatedPrice = initialPrices.at(i);
		auto timeStep = 0.0;

		// If the jump time is before the dividend payment time, then we first simulate the asset value at the jump time, and then enforce the
		// zero absorbing boundary at the dividend time.
		if (m_dividendTime < time + 0.00000001 && m_jumpTime < m_dividendTime + 0.00000001)
		{
			// Simulate at jump date
			timeStep = m_jumpTime;
			simulatedJump = m_jumpMean + m_jumpVolatility * normalDistribution(mersenneTwisterEngine);
			simulatedNormal = normalDistribution(mersenneTwisterEngine);
			simulatedPrice = simulatedPrice * exp(drift * timeStep + simulatedJump + m_impliedVolatility * sqrt(timeStep) * simulatedNormal);
			// Simulate at dividend date
			timeStep = m_dividendTime - m_jumpTime;
			simulatedNormal = normalDistribution(mersenneTwisterEngine);
			simulatedPrice = simulatedPrice * exp(drift * timeStep + m_impliedVolatility * sqrt(timeStep) * simulatedNormal);
			if (simulatedPrice - m_dividendAmount < 0.00000001)
				simulatedPrice = 0.0;

			simulatedJump = 0.0;
			timeStep = time - m_dividendTime;
		}

		// If the jump happens after the dividend payment, then we simulate at the dividend payment date, enforce the absorbing boundary, and then
		// simply simulate at time
		if (m_jumpTime < time + 0.00000001)
		{
			if (m_dividendTime < time + 0.00000001 && m_jumpTime > m_dividendTime)
			{
				// Simulate at dividend date
				auto timeStep = m_dividendTime;
				simulatedNormal =normalDistribution(mersenneTwisterEngine);
				simulatedPrice = simulatedPrice * exp(drift * timeStep + m_impliedVolatility * sqrt(timeStep) * simulatedNormal);
				if (simulatedPrice - m_dividendAmount < 0.00000001)
					simulatedPrice = 0.0;
				timeStep = time - m_dividendTime;
			}
			else
			{
				timeStep = time;
			}
			simulatedJump = m_jumpMean + m_jumpVolatility * normalDistribution(mersenneTwisterEngine);
		}

		// Simulate the price at time
		simulatedNormal =normalDistribution(mersenneTwisterEngine);
		timeStep = time;
		simulatedJump = m_jumpMean + m_jumpVolatility * normalDistribution(mersenneTwisterEngine);
		simulatedPrice = simulatedPrice * exp(drift * timeStep + simulatedJump + m_impliedVolatility * sqrt(timeStep) * simulatedNormal);

		// Assign the simulated price to initial prices
		initialPrices.at(i) = simulatedPrice;

		// Deduct the dividend if it is paid
		if (m_dividendTime < time + 0.00000001) 
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

//// The single Normal jump is capture via a 5 node discretisation, with moment matching up to the fourth moment.

//// The dividend is a deterministic value, the payoff of which does not impact the dynamics of CRR or Tian methods. For all nodes after
//// the dividend payment date, the dividend can simply be deducted from the value at the node. Note that his has to be done after the tree has 
//// already been constructed, because the payment of the dividend does not impact the diffusion process.

const std::shared_ptr<models::Tree> models::BlackScholesSingleNormalJump::constructTree(const int& nTimeSteps, const double& timeToExpiry, 
	const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation)
{

	// Calculate the state and probabilities for diffusion and jumps
	// ---------------------------------------------------------------------------
	// As the time steps are of the same size throughout the tree, the multipliers and probabilities are invariant with time.
	auto timeStepSize = timeToExpiry / (double)nTimeSteps;

	shared_ptr<vector<double>> diffusionStatesPtr, diffusionProbabilitiesPtr;
	tie(diffusionStatesPtr, diffusionProbabilitiesPtr) = LogNormalDiffusionTreeHelper::calculateDiffusionStatesAndProbabilities(timeStepSize, 
		m_impliedVolatility, m_discountRate, m_costOfCarry, implementation);

	shared_ptr<vector<double>> jumpStatesPtr, jumpProbabilitiesPtr;
	tie(jumpStatesPtr, jumpProbabilitiesPtr) = LogNormalDiffusionTreeHelper::calculateNormalJumpStatesAndProbabilities(m_jumpMean,
		m_jumpVolatility, m_jumpTime, timeToExpiry);

	shared_ptr<vector<double>> jumpDiffusionStatesPtr, jumpDiffusionProbabilitiesPtr;
	tie(jumpDiffusionStatesPtr, jumpDiffusionProbabilitiesPtr) = LogNormalDiffusionTreeHelper::calculateJumpDiffusionStatesAndProbabilities(
		jumpStatesPtr, diffusionStatesPtr, jumpProbabilitiesPtr, diffusionProbabilitiesPtr);


	// Construct tree 
	// ---------------------------------------------------------------------------
	auto treeNodesPtr = LogNormalDiffusionTreeHelper::constructJumpDiffusionTree(nTimeSteps, timeToExpiry, m_jumpTime, m_dividendTime,
		m_dividendAmount, m_initialUnderlyingPrice, jumpDiffusionStatesPtr, jumpDiffusionProbabilitiesPtr, diffusionStatesPtr, 
		diffusionProbabilitiesPtr);
	auto treePtr = make_shared<Tree>(treeNodesPtr, nTimeSteps, timeToExpiry);
	return treePtr;
}


//// Calculates the option value at the current tree node with smoothing
const std::shared_ptr<double> models::BlackScholesSingleNormalJump::smoothedValueAtTreeNode(const double underlyingPrice, 
	const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize)
{
	// This function is only called from with TreePricer. If the dividend was paid prior to the second last time in the grid, then its value
	// has already been deducted from the underlying price. However, if the dividend falls after the second last time, but before option expiry, 
	// then smoothing is not implemented because it would involve an adjustement of the Strike.

	// need to construct a new Black Scholes model with a undelrying asset price as at the current tree node. First make copies of the parameters.
	auto value = LogNormalDiffusionTreeHelper::treeNodeEuropeanOptionValue(m_costOfCarry, m_discountRate, m_impliedVolatility, underlyingPrice,
		m_underlyingCode, vanillaOption->getStrike(), timeStepSize, vanillaOption->getOptionRight());
	return value;
}

const bool models::BlackScholesSingleNormalJump::supportsVanillaOptionSmoothing(const double & timeStart, const double & timeEnd)
{
	// If the dividend or jump date falls after the second last time, but before option expiry, then no smoothing
	auto smoothing = ((m_dividendTime > timeStart - 0.00000001) && (m_dividendTime < timeEnd + 0.00000001)) || 
		((m_jumpTime > timeStart - 0.00000001) && (m_jumpTime < timeEnd + 0.00000001)) ? false : m_supportsVanillaOptionSmoothing; 
	return smoothing;
}



