#include <map>
#include <vector>
#include <numeric>
#include <random>
#include "MonteCarloPricer.h"
#include "../Enumerations/ExerciseType.h"

using namespace std;
using namespace enumerations;

pricers::MonteCarloPricer::MonteCarloPricer(const std::shared_ptr<models::IMonteCarloModel>& model)
{
	setModel(model);
}

// Returns the price of the vanilla options
const std::shared_ptr<std::vector<std::shared_ptr<double>>> pricers::MonteCarloPricer::price(const int& nPaths,
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions, int seed)
{
	// Input validation
	// Check that the model and vanilla options have the same underlying
	auto nValillaOptions = vanillaOptions->size();
	auto modelUnderlyingCode = m_model->getUnderlyingCode();
	for (int i = 0; i < nValillaOptions; i++)
	{
		auto vanillaOptionUnderlyingCode = vanillaOptions->at(i)->getUnderlyingCode();
		if (vanillaOptionUnderlyingCode != modelUnderlyingCode)
			throw invalid_argument("The vanilla options are not on the same undelrying as the pricing model.");
	}

	// Simulate the underlying values based on the unique times to expiry in the vanilla options vector
	map<double, shared_ptr<vector<double>>> simulationsByTimeToExpiry;
	for (int i = 0; i < nValillaOptions; i++)
	{
		auto timeToExpiry = vanillaOptions->at(i)->getTimeToExpiry();
		if (simulationsByTimeToExpiry.count(timeToExpiry) == 0) // simulation for the current option's time to expiry has not been done.
		{
			auto simulation = m_model->generateMonteCarloSimulations(nPaths, timeToExpiry, seed);
			simulationsByTimeToExpiry.insert({ timeToExpiry, simulation });
		}
		else
			continue;
	}

	// Calculate the price for each vanilla option
	vector<shared_ptr<double>> prices;
	prices.reserve(nValillaOptions);
	for (int i = 0; i < nValillaOptions; i++)
	{
		auto timeToExpiry = vanillaOptions->at(i)->getTimeToExpiry();
		prices.push_back(pricers::MonteCarloPricer::price(m_model->getDiscountRate(), simulationsByTimeToExpiry[timeToExpiry], vanillaOptions->at(i)));
	}
	auto pricesPtr = make_shared<vector<shared_ptr<double>>>(move(prices));
	return pricesPtr;
}


//// Calculate the Monte Carlo price for a single European option, given the simulated underlying prices from a model
const std::shared_ptr<double> pricers::MonteCarloPricer::price(const double& discountRate, 
	const std::shared_ptr<std::vector<double>>& simulatedUnderlyingPrices, const std::shared_ptr<instruments::VanillaOption>& vanillaOption)
{
	// Input validation. This function can only be used to price European options.
	if (vanillaOption->getExerciseType() != ExerciseType::european)
		throw invalid_argument("Monte carlo pricing is currently only supported for European options.");

	// Calculate the option payoff for each path, and return the discounted arithmetic average
	auto nPaths = simulatedUnderlyingPrices->size();
	vector<double> simulatedPayoffs;
	simulatedPayoffs.reserve(nPaths);
	for (int i = 0; i < nPaths; i++)
	{
		auto simulatedPayoff = vanillaOption->intrinsicValue(simulatedUnderlyingPrices->at(i));
		simulatedPayoffs.push_back(*simulatedPayoff);
	}
	auto price = exp(-discountRate * vanillaOption->getTimeToExpiry()) * accumulate(simulatedPayoffs.begin(), simulatedPayoffs.end(), 0.0) / nPaths;
	auto pricePtr = make_shared<double>(move(price));
	return pricePtr;
}


