#ifndef __OPTIMISERAPI_H__
#define __OPTIMISERAPI_H__

#include <vector>
#include <memory>
#include "JSONUtilities.h"
#include "../Instruments/VanillaOption.h"


class OptimiserAPI
{
public:

	// Functions for Optimisation Problem 1

	static std::shared_ptr<std::vector<double>> optimiseProblem1(const std::string & jsonInputFilePath, 
		const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const double& F, const double& CR, const int& N,
		std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed = -1);

	static double meanSquaredErrorProblem1(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
		std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
		const double& costOfCarry, const double& discountRate, const double& initialUnderlyingPrice,
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const std::shared_ptr<std::vector<double>>& volatilitiesToOptimise, const int& D);


	// Functions for Optimisation Problem 2

	static std::shared_ptr<std::vector<double>> optimiseProblem2(const std::string & jsonInputFilePath,
		const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const double& F, const double& CR, const int& N,
		std::shared_ptr<std::vector<double>> lowerBounds, std::shared_ptr<std::vector<double>> upperBounds, const double& tolerance, int seed);

	static double meanSquaredErrorProblem2(std::shared_ptr<std::vector<std::shared_ptr<double>>> optionPricesPtr,
		std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> optionsPtr,
		const double& initialUnderlyingPrice, const double& dividendTime, const double& jumpTime, const double& jumpMean, const int& nTimeSteps,
		const std::shared_ptr<std::vector<double>>& parametersToOptimise, const int& D);

private:
	OptimiserAPI() {};
};

#endif // !__OPTIMISERAPI_H__
