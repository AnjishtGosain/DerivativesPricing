#include <vector>
#include <numeric>
#include "AnalyticPricer.h"
#include "../Enumerations/ExerciseType.h"

using namespace std;
using namespace enumerations;

pricers::AnalyticPricer::AnalyticPricer(const std::shared_ptr<models::IAnalyticModel>& model)
{
	setModel(model);
}

// Returns the price of the vanilla options
const std::shared_ptr<std::vector<std::shared_ptr<double>>> pricers::AnalyticPricer::price(
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions)
{
	auto nValillaOptions = vanillaOptions->size();
	vector<shared_ptr<double>> prices;
	prices.reserve(nValillaOptions);
	for (int i = 0; i < nValillaOptions; i++)
	{
		prices.push_back(pricers::AnalyticPricer::price(vanillaOptions->at(i)));
	}
	auto pricesPtr = make_shared<vector<shared_ptr<double>>>(move(prices));
	return pricesPtr;
}


//// Calculate the analytic price for a single vanilla option
const std::shared_ptr<double> pricers::AnalyticPricer::price(const std::shared_ptr<instruments::VanillaOption>& vanillaOption)
{
	// Input validation. Check that the model and vanilla option have the same underlying
	if (m_model->getUnderlyingCode() != vanillaOption->getUnderlyingCode())
		throw invalid_argument("The vanilla option doesn not have the same undelrying as the pricing model.");

	auto pricePtr = m_model->calculateAnalyticSolution(
		vanillaOption->getStrike(),
		vanillaOption->getTimeToExpiry(),
		vanillaOption->getOptionRight(),
		vanillaOption->getExerciseType());
	return pricePtr;
}

