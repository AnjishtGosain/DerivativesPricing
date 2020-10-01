#include <iostream>
#include "VanillaOption.h"

using namespace std;

using namespace enumerations;

instruments::VanillaOption::VanillaOption(const double& strike, const double& timeToExpiry, const enumerations::ExerciseType& exerciseType, 
	const enumerations::OptionRight& optionRight, const enumerations::UnderlyingCode& underlyingCode)
{
	setStrike(strike);
	setTimeToExpiry(timeToExpiry);
	setExerciseType(exerciseType);
	setOptionRight(optionRight);
	setUnderlyingCode(underlyingCode);
}

void instruments::VanillaOption::setStrike(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The strike for an option must be greater than 0.");

	m_strike = value;
}

void instruments::VanillaOption::setTimeToExpiry(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The time to expiry for an option must be greater than 0.");
	m_timeToExpiry = value;
}

//// The following function is used to enforce the exercise condition for American options in a binomial tree. 
//// <forwardPrice> is the value of the option at the Binomial node prior to the enforcement of the exercise condition.
//// <underlyingPrice> is the value of the underlying asset at the Binomial node.
const std::shared_ptr<double> instruments::VanillaOption::valueAtTreeNode (const double& forwardValue, const double& underlyingPrice) const
{
	// Validate Inputs
	if (forwardValue < -0.00000001)
		throw invalid_argument("The option price at the current node is less than zero.");
	if (underlyingPrice < -0.00000001)
		throw invalid_argument("The undelrying price at the current node is less than zero.");

	double output;
	if (m_exerciseType == ExerciseType::american)
		output = fmax(forwardValue, *intrinsicValue(underlyingPrice));
	else
		output = forwardValue; // for a European simply return the value at the current node

	auto outputPtr = make_shared<double>(move(output));
	return outputPtr;
}

// Calculated the payoff for the option given the underlying price 
const std::shared_ptr<double> instruments::VanillaOption::intrinsicValue(const double & underlyingPrice) const
{
	auto exerciseSign = m_optionRight == OptionRight::call ? 1.0 : -1.0;
	auto intrinsicValue = fmax(0.0, exerciseSign * (underlyingPrice - m_strike));
	auto intrinsicValuePtr = make_shared<double>(move(intrinsicValue));
	return intrinsicValuePtr;
}

