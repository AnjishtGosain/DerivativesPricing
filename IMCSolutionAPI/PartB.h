#ifndef __PARTB_H__
#define __PARTB_H__

#include <vector>
#include <memory>
#include "JSONUtilities.h"
#include "../Instruments/VanillaOption.h"


class PartB
{
public:

	// Pricing for Part B5

	static void priceVanillaOptionsBlackScholesDoubleNormalJumpModel(const std::string & jsonInputFilePath,
		const std::string & jsonOutputFilePath, const double & initialUnderlyingPrice, const double & jumpMean, const int & nTimeSteps);

	static void priceAmericanOptionsBlackScholesSingleNormalJumpModel(const std::string& jsonInputFilePath, const std::string& jsonOutputFilePath,
		const double& costOfCarry, const double& discountRate, const double& impliedVolatility, const double& initialUnderlyingPrice, 
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const double& jumpVolatility,
		const int& nTimeSteps);


	// Optimisation for Part B6

	static std::shared_ptr<std::vector<double>> optimisePartB6(const std::string & jsonInputFilePath, 
		const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const double& F, const double& CR, const int& N,
		std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed = -1);

	static double meanSquaredErrorPartB6(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
		std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
		const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const std::shared_ptr<std::vector<double>>& volatilitiesToOptimise, const int& D);


	// Optimisation for Part B7

	static std::shared_ptr<std::vector<double>> optimisePartB7(const std::string & jsonInputFilePath,
		const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const double& F, const double& CR, const int& N,
		std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed);

	static double meanSquaredErrorPartB7(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
		std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
		const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const std::shared_ptr<std::vector<double>>& parametersToOptimise, const int& D);

private:
	PartB() {};
};

#endif // !__PARTB_H__
