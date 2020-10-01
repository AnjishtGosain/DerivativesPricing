#include <map>
#include <vector>
#include <numeric>
#include "TreePricer.h"
#include "../Enumerations/ExerciseType.h"
#include "../Models/TreeModelUtilities/TreeNode.h"

using namespace std;
using namespace enumerations;
using namespace models;

pricers::TreePricer::TreePricer(const std::shared_ptr<models::ITreeModel>& model)
{
	setModel(model);
}

//// Returns the price of the vanilla options with Richardson Extrapolation
//// Suppose the size of the time step is 1/4 of a year. Then, under Richardson Extrapolation, the option price is the average of the price
//// calculated with a time step of 1/4 of a year, and 1/2 of a year.
//// The upper and lower standard deviation bounds are used to truncate the tree, but are currently deprecated.
const std::shared_ptr<std::vector<std::shared_ptr<double>>> pricers::TreePricer::priceWithRichardsonExtrapolation(const int& nTimeSteps, 
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions, const bool & useVanillaOptionSmoothing,
	const enumerations::Implementation& implementation, const double& upperLimitStandardDeviation, const double& lowerLimitStandardDeviation)
{
	auto fullTimeStepPrices = price(nTimeSteps, vanillaOptions, useVanillaOptionSmoothing, implementation, upperLimitStandardDeviation, 
		lowerLimitStandardDeviation);
	auto halfTimeStepPrices = price(2 * nTimeSteps, vanillaOptions, useVanillaOptionSmoothing, implementation, upperLimitStandardDeviation,
		lowerLimitStandardDeviation);
	vector<shared_ptr<double>> prices;
	prices.reserve(fullTimeStepPrices->size());
	for (int i = 0; i < fullTimeStepPrices->size(); i++)
	{
		auto price = make_shared<double>(0.5 * (*fullTimeStepPrices->at(i) + *halfTimeStepPrices->at(i)));
		prices.push_back(move(price));
	}
	auto pricesPtr = make_shared <vector<shared_ptr<double>>>(move(prices));
	return pricesPtr;
}


//// Returns the price of the vanilla options for a specified number of discrete time steps 
//// Trees for the underlying price are constructed using the private model, as delegated at run time.
//// Trees are constructred for each unique expiry in the set of options, and then the tree is passed on the option pricing function.
const std::shared_ptr<std::vector<std::shared_ptr<double>>> pricers::TreePricer::price(const int nTimeSteps,
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> vanillaOptions, const bool useVanillaOptionSmoothing, 
	const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation)
{
	// Input validation. Check that the model and vanilla options have the same underlying
	auto nValillaOptions = vanillaOptions->size();
	auto modelUnderlyingCode = m_model->getUnderlyingCode();
	for (int i = 0; i < nValillaOptions; i++)
	{
		auto vanillaOptionUnderlyingCode = vanillaOptions->at(i)->getUnderlyingCode();
		if (vanillaOptionUnderlyingCode != modelUnderlyingCode)
			throw invalid_argument("The vanilla options are not on the same undelrying as the pricing model.");
	}

	// Construct a tree for the underlying asset price for each unique times to expiry in the vanilla options vector
	map<double, shared_ptr<Tree>> treesByTimeToExpiry;
	for (int i = 0; i < nValillaOptions; i++)
	{
		auto timeToExpiry = vanillaOptions->at(i)->getTimeToExpiry();
		if (treesByTimeToExpiry.count(timeToExpiry) == 0) // tree for the current option's time to expiry has not been constructed.
		{
			//auto nTimeSteps = (int)(timeToExpiry / timeStepSize + 0.5); // round up the number of time steps
			auto tree = m_model->constructTree(nTimeSteps, timeToExpiry, implementation, upperLimitStandardDeviation, lowerLimitStandardDeviation);
			treesByTimeToExpiry.insert({ timeToExpiry, move(tree) });
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
		prices.push_back(pricers::TreePricer::price(m_model->getDiscountRate(), treesByTimeToExpiry[timeToExpiry], vanillaOptions->at(i), 
			useVanillaOptionSmoothing));
	}
	auto pricesPtr = make_shared<vector<shared_ptr<double>>>(move(prices));
	return pricesPtr;
}

//// Calculate the price for a Vanilla Option given a preconstructed tree and discount rate 
const std::shared_ptr<double> pricers::TreePricer::price(const double discountRate, const std::shared_ptr<models::Tree> tree, 
	const std::shared_ptr<instruments::VanillaOption> vanillaOption, const bool useVanillaOptionSmoothing)
{
	// Input validation - check whether the time to expiry of the provided tree is the same as that for the vanilla option
	if (abs(tree->getTimeToExpiry() - vanillaOption->getTimeToExpiry()) > 0.0000001)
		throw invalid_argument("The provided tree and vanilla option do not have the same time to maturity.");

	// Pricing is done in backwards time. Suppose there are n time steps. Then, if useVanillaOptionSmoothing is true, and the model supports it,
	// then we calculate the value at each node at time n - 1 as the smooth value. Otherwise, we start at time n, and calculate the payoff (i.e.
	// procede as usual.

	auto nTimeSteps = tree->getNTimesSteps();
	auto timeStepSize = (tree->getTimeToExpiry()) / nTimeSteps;
	auto treeNodes = tree->getTreeNodes();
	vector<double> futureValues; // option value at the tree nodes of the next time step
	vector<double> currentValues; // option value at the tree nodes of current time step
	auto maxFutureValuesNodes = treeNodes->at(nTimeSteps - 1)->size();
	futureValues.reserve(maxFutureValuesNodes);
	currentValues.reserve(maxFutureValuesNodes);

	auto currentValuesPtr = make_shared<vector<double>>(move(currentValues));
	auto futureValuesPtr = make_shared<vector<double>>(move(futureValues));

	// useVanillaOptionSmoothin determines whether or not to use the closed form solution in valuing the option 
	// The smoothing is only applicable if the current time is the second last time in the grid, with the last time being expiry
	for (int i = (nTimeSteps - 1); i >= 0; i--) 
	{

		auto futureTreeNodes = treeNodes->at(i + 1); // i = 2 returns the tree nodes at the last time step, i.e. expiry
		auto currentTreeNodes = treeNodes->at(i); 

		// Initialise the future values. i.e. calculate the option payoff at maturity, and apply any exercise conditions.
		// Only needs to done if there is no smoothing
		if (i == (nTimeSteps - 1) && !(useVanillaOptionSmoothing && m_model->supportsVanillaOptionSmoothing(timeStepSize * (double)i, 
			timeStepSize * (double)(i+1))))
		{
			for (int j = 0; j < futureTreeNodes->size(); j++) 
			{
				auto payoffAtNode = vanillaOption->intrinsicValue(futureTreeNodes->at(j)->getValue()); // at maturity the value is always intrinsic
				futureValuesPtr->push_back(*payoffAtNode);
			}
		}

		// Get the future tree nodes
		if (i == (nTimeSteps - 1) && useVanillaOptionSmoothing && m_model->supportsVanillaOptionSmoothing(timeStepSize * (double)i, 
			timeStepSize * (double)(i + 1)))
		{
			for (int j = 0; j < currentTreeNodes->size(); j++)
			{
				auto smoothedValue = m_model->smoothedValueAtTreeNode(
					currentTreeNodes->at(j)->getValue(),
					vanillaOption,
					timeStepSize);
				auto value = vanillaOption->valueAtTreeNode(*smoothedValue, currentTreeNodes->at(j)->getValue()); // apply Exercise conditions
				currentValuesPtr->push_back(*value);
			}
		}
		else // i.e. no smoothing, calculate the expected present value for each node, and apply any exercise conditions
		{
			for (int j = 0; j < currentTreeNodes->size(); j++)
			{
				auto currentTreeNode = currentTreeNodes->at(j);
				auto value = 0.0;
				// iterate through each of the forward underlying prices of the current node
				auto forwardValuesIndex = currentTreeNode->getForwardValuesIndex(); // the index of the nodes for the forward time
				auto forwardProbabilities = currentTreeNode->getForwardProbabilities(); // the index of the nodes for the forward time
				auto nForwardValues = currentTreeNode->getNForwardValues();
				for (int k = 0; k < currentTreeNode->getNForwardValues(); k++)
				{
					auto index = forwardValuesIndex->at(k);
					value += forwardProbabilities->at(k) * futureValuesPtr->at(index);
				}
				value = value * exp(-1.0 * m_model->getDiscountRate() * timeStepSize); // expected present value
				auto exercisedValue = vanillaOption->valueAtTreeNode(value, currentTreeNode->getValue());
				currentValuesPtr->push_back(*exercisedValue);
			}
		}

		// Swap around the points for the next time step
		futureValuesPtr = currentValuesPtr; // the current option values become the future values for the next time step
		vector<double> newEmptyVector;
		newEmptyVector.reserve(maxFutureValuesNodes);
		currentValuesPtr = make_shared<vector<double>>(newEmptyVector); // point the current values ptr to a new empty vector
	}

	// Return the option price
	auto pricePtr = make_shared<double>(move(futureValuesPtr->at(0))); // after the loop is done, the option price is pointed to by futureValuesPtr
	return pricePtr;
}

