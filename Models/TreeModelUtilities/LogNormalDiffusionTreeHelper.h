#ifndef __LOGNORMALDIFFUSIONTREEHELPER_H__ 
#define __LOGNORMALDIFFUSIONTREEHELPER_H__

#include <iostream>
#include <vector>
#include <memory>
#include <tuple>
#include "../../Instruments/VanillaOption.h"
#include "../../Enumerations/Implementation.h"
#include "../../Enumerations/OptionRight.h"
#include "TreeNode.h"
#include "Tree.h"

namespace models
{
	// This is a helper class to assist with the construction of trees for models with lognormal diffusion asset price processes. This class 
	// cannot be directly instantiated as a model in its own right, but rather, is intended for use by models.

	class LogNormalDiffusionTreeHelper
	{
	public:
		// calculation of probabilities and states
		static const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> 
			calculateDiffusionStatesAndProbabilities
			(const double timeStepSize, const double impliedVolatility, const double discountRate, 
				const double costOfCarry, const enumerations::Implementation& implementation);

		static const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> 
			calculateNormalJumpStatesAndProbabilities
			(const double jumpMean, const double jumpVolatility, const double jumpTime, const double timeToExpiry);

		static const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>>
			calculateJumpDiffusionStatesAndProbabilities
			(const std::shared_ptr<std::vector<double>> jumpStatesPtr, const std::shared_ptr<std::vector<double>> diffusionStatesPtr,
				const std::shared_ptr<std::vector<double>> jumpProbabilitiesPtr, const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr);


		// helper functions of option value smoothing
		static const std::shared_ptr<double> 
			treeNodeEuropeanOptionValue
			(const double costOfCarry, const double discountRate, const double impliedVolatility, const double underlyingPrice, 
				const enumerations::UnderlyingCode underlyingCode, const double strike, 
				const double timeStepSize, const enumerations::OptionRight optionRight);
		

		// helper functions for the deduction of dividend
		static void 
			deductDividend
			(std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>& treePtr, const double& dividendTime, 
				const double& dividendAmount, const double& timeToExpiry, const double& timeStepSize, const int& nTimeSteps);


		// functions for the construction of recombining trees
		static std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>> 
			constructInitialTreeNode
			(const double initialUnderlyingPrice, const int& nFutureNodesPerCurrentNode, const std::shared_ptr<std::vector<double>>& probabilitiesPtr);

		static std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>> 
			constructRecombiningTreeNodes
			(std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>& previousNodes, const double& upperLimit, const double& lowerLimit, 
				const std::shared_ptr<std::vector<double>> diffusionStatesPtr, const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr, 
				const bool& isDividendPaid, const double dividendAmount = -1.0, const bool& isLastTime = false);

		static std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>
			constructRecombiningTree
			(const double initialUnderlyingPrice, const int nTimeSteps, const double timeToExpiry, const double impliedVolatility, 
				const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation, const double dividendTime, 
				const double dividendAmount, const std::shared_ptr<std::vector<double>> diffusionStatesPtr, 
				const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr);


		// functions for the construction of a jump diffusion tree
		static std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>
			constructJumpDiffusionTree(const int nTimeSteps, const double timeToExpiry,
				const double jumpTime, const double dividendTime, const double dividendAmount, const double initialUnderlyingPrice,
				const std::shared_ptr<std::vector<double>> jumpDiffusionStatesPtr,
				const std::shared_ptr<std::vector<double>> jumpDiffusionProbabilitiesPtr,
				const std::shared_ptr<std::vector<double>> diffusionStatesPtr,
				const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr);

	private:
		LogNormalDiffusionTreeHelper() {};
	};
}

#endif // !__LOGNORMALDIFFUSIONTREEHELPER_H__
