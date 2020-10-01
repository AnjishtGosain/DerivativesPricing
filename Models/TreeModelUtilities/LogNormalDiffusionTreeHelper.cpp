#include "LogNormalDiffusionTreeHelper.h"
#include "../BlackScholes.h"
#include "../../Enumerations/ExerciseType.h"

using namespace std;
using namespace models;
using namespace enumerations;

// This function calculates the up and down states, and the probabilities of hitting those states, for a binomial tree discretisation of
// a lognormal diffusion process.
const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> 
	models::LogNormalDiffusionTreeHelper::calculateDiffusionStatesAndProbabilities(const double timeStepSize, 
	const double impliedVolatility, const double discountRate, const double costOfCarry, const enumerations::Implementation& implementation)
{
	// Calculate diffusion states and probabilities
	// ---------------------------------------------------------------------------
	vector<double> diffusionStates;
	vector<double> diffusionProbabilities;
	diffusionStates.reserve(2);
	diffusionProbabilities.reserve(2);
	auto r = 0.0; // parameter for Tian implementation
	auto v = 0.0; // parameter for Tian implementation

	switch (implementation)
	{
	case enumerations::Implementation::One:
		// Cox Ross Rubinstein
		diffusionStates.push_back(impliedVolatility * sqrt(timeStepSize)); // up state
		diffusionStates.push_back(-1.0 * impliedVolatility * sqrt(timeStepSize)); // down state
		break;
	case enumerations::Implementation::Two:
		// Tian
		r = exp((discountRate - costOfCarry) * timeStepSize);
		v = exp(pow(impliedVolatility,2) * timeStepSize);
		diffusionStates.push_back(log(0.5 * r * v * (v + 1 + sqrt(pow(v, 2) + 2 * v - 3)))); // up state
		diffusionStates.push_back(log(0.5 * r * v * (v + 1 - sqrt(pow(v, 2) + 2 * v - 3)))); // down state
		break;
	default:
		throw invalid_argument("The constructTree method for the BlackScholesSingleNormalJump model only supports two tree implementations.");
		break;
	}

	// The up and down probabilities are as usual (originally proposed by Nelson and Ramaswamy (1990) to achieve constant volatility on the tree).
	auto diffusionUpProbability = 0.5 * (1.0 + sqrt(timeStepSize) * (discountRate - costOfCarry - 0.5 * pow(impliedVolatility, 2)) / 
		impliedVolatility);
	auto diffusionDownProbability = 1.0 - diffusionUpProbability;
	diffusionProbabilities.push_back(move(diffusionUpProbability));
	diffusionProbabilities.push_back(move(diffusionDownProbability));

	if (diffusionUpProbability > 1.00000001 || diffusionUpProbability < -0.00000001)
		throw invalid_argument("Diffusion Probabilities outside of [0,1] have been generated. Increase the number of time steps.");

	auto diffusionStatesPtr = make_shared<vector<double>>(move(diffusionStates));
	auto diffusionProbabilitiesPtr = make_shared<vector<double>>(move(diffusionProbabilities));

	return { diffusionStatesPtr, diffusionProbabilitiesPtr };
}


// This function calculates the states and probabilities for a single normal jump based on 5 states and moment matching
const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> 
	models::LogNormalDiffusionTreeHelper::calculateNormalJumpStatesAndProbabilities
	(const double jumpMean, const double jumpVolatility, const double jumpTime, const double timeToExpiry)
{
	// Calculate diffusion jump states - i.e. simultaneous diffusion and jump
	// ---------------------------------------------------------------------------

	auto jumpSize = jumpVolatility;
	vector<double> jumpStates;
	jumpStates.reserve(5);

	// calculate the jump diffusion states
	for (int j = 2; j > -3; j--)
		jumpStates.push_back(jumpMean + j * jumpVolatility);


	// Calculate diffusion jump probabilities 
	// ---------------------------------------------------------------------------
	vector<double> jumpProbabilities;
	jumpProbabilities.reserve(5);

	// These probabilities have been determined based on moment matching the first four non-central moments of the normal distribution,
	// along with the condition that the sum of all probabilities must equal to 1.
	jumpProbabilities.push_back(1.0 / 12.0);
	jumpProbabilities.push_back(1.0 / 6.0);
	jumpProbabilities.push_back(1.0 / 2.0);
	jumpProbabilities.push_back(1.0 / 6.0);
	jumpProbabilities.push_back(1.0 / 12.0);

	auto jumpProbabilitiesPtr = make_shared<vector<double>>(move(jumpProbabilities));
	auto jumpStatesPtr = make_shared<vector<double>>(move(jumpStates));

	return { jumpStatesPtr, jumpProbabilitiesPtr };
}


// calculate the jump diffusion states and probabilities
const std::tuple<std::shared_ptr<std::vector<double>>, std::shared_ptr<std::vector<double>>> 
models::LogNormalDiffusionTreeHelper::calculateJumpDiffusionStatesAndProbabilities
(const std::shared_ptr<std::vector<double>> jumpStatesPtr, const std::shared_ptr<std::vector<double>> diffusionStatesPtr,
	const std::shared_ptr<std::vector<double>> jumpProbabilitiesPtr, const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr)
{
	auto nJumpStates = jumpStatesPtr->size();
	auto nDiffusionStates = diffusionStatesPtr->size();
	auto nJumpDiffusionStates = nJumpStates + nDiffusionStates;
	vector<double> jumpDiffusionStates;
	vector<double> jumpDiffusionProbabilities;
	jumpDiffusionStates.reserve(nJumpDiffusionStates);
	jumpDiffusionProbabilities.reserve(nJumpDiffusionStates);
	for (int j = 0; j < nJumpStates; j++)
		for (int i = 0; i < nDiffusionStates; i++)
		{
			jumpDiffusionStates.push_back(diffusionStatesPtr->at(i) + jumpStatesPtr->at(j));
			jumpDiffusionProbabilities.push_back(diffusionProbabilitiesPtr->at(i) * jumpProbabilitiesPtr->at(j));
		}
	auto jumpDiffusionStatesPtr = make_shared<vector<double>>(move(jumpDiffusionStates));
	auto jumpDiffusionProbabilitiesPtr = make_shared<vector<double>>(move(jumpDiffusionProbabilities));
	return { jumpDiffusionStatesPtr, jumpDiffusionProbabilitiesPtr };
}


// Loosely coupled smoother for vanilla option
const std::shared_ptr<double> models::LogNormalDiffusionTreeHelper::treeNodeEuropeanOptionValue(const double costOfCarry, const double discountRate,
	const double impliedVolatility, const double underlyingPrice, const enumerations::UnderlyingCode underlyingCode, const double strike, 
	const double timeStepSize, const enumerations::OptionRight optionRight)
{
	// need to construct a new Black Scholes model with a undelrying asset price as at the current tree node. First make copies of the parameters.
	BlackScholes tempBlackScholes(costOfCarry, discountRate, impliedVolatility, underlyingPrice, underlyingCode);
	auto forwardValue = tempBlackScholes.calculateAnalyticSolution(
		strike,
		timeStepSize,
		optionRight,
		ExerciseType::european);
	return forwardValue;
}


// Construct the whole recombining tree
std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>> 
models::LogNormalDiffusionTreeHelper::constructRecombiningTree(
	const double initialUnderlyingPrice, const int nTimeSteps, const double timeToExpiry, const double impliedVolatility,
	const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation, const double dividendTime, const double dividendAmount,
	const std::shared_ptr<std::vector<double>> diffusionStatesPtr, const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr)
{
	// Initialise tree
	// ---------------------------------------------------------------------------
	auto timeStepSize = timeToExpiry / (double)nTimeSteps;
	vector<shared_ptr<vector<shared_ptr<TreeNode>>>> treeNodes;
	treeNodes.reserve(nTimeSteps + 1); // if there are n time steps, then there will be n+1 times


	// Construct the very first node
	// ---------------------------------------------------------------------------
	auto nFutureNodesPerCurrentNode = diffusionStatesPtr->size();
	auto probabilitiesPtr = diffusionProbabilitiesPtr;
	auto initialNode = LogNormalDiffusionTreeHelper::constructInitialTreeNode(initialUnderlyingPrice, nFutureNodesPerCurrentNode, probabilitiesPtr);
	treeNodes.push_back(move(initialNode));


	// Construct the remaining nodes in forward time
	// ---------------------------------------------------------------------------
	auto oneStandardDeviationMove = initialUnderlyingPrice * exp(impliedVolatility);
	for (int i = 1; i < nTimeSteps + 1; i++)
	{
		auto time = i * timeStepSize;
		auto upperLimit = oneStandardDeviationMove * exp(upperLimitStandardDeviation * sqrt(time));
		auto lowerLimit = fmax(0.00000001, oneStandardDeviationMove * exp(lowerLimitStandardDeviation * sqrt(time)));
		auto isDividendPaid = dividendTime < time + 0.00000001 ? true : false;
		auto isLastTime = i == nTimeSteps ? true : false;
		auto treeNodesIPtr = LogNormalDiffusionTreeHelper::constructRecombiningTreeNodes(treeNodes[i-1], upperLimit, lowerLimit,
			diffusionStatesPtr, diffusionProbabilitiesPtr, isDividendPaid, dividendAmount, isLastTime);
		treeNodes.push_back(move(treeNodesIPtr));
	}


	// Deduct dividends from tree
	// ---------------------------------------------------------------------------
	auto treeNodesPtr = make_shared<vector<shared_ptr<vector<shared_ptr<TreeNode>>>>>(move(treeNodes));
	LogNormalDiffusionTreeHelper::deductDividend(treeNodesPtr, dividendTime, dividendAmount, timeToExpiry, timeStepSize, nTimeSteps);

	return treeNodesPtr;
}


// constructs the first node in a tree
std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>> models::LogNormalDiffusionTreeHelper::constructInitialTreeNode(
	const double initialUnderlyingPrice, const int & nFutureNodesPerCurrentNode, const std::shared_ptr<std::vector<double>>& probabilitiesPtr)
{
	vector<shared_ptr<TreeNode>> treeNodesI;
	auto nMaxCurrentTimeNodes = 1; // the maximum number of nodes at the current time step
	treeNodesI.reserve(nMaxCurrentTimeNodes);
	vector<int> valuesIndex;
	valuesIndex.reserve(nFutureNodesPerCurrentNode);
	for (int k = 0; k < nFutureNodesPerCurrentNode; k++)
		valuesIndex.push_back(k);
	auto valuesIndexPtr = make_shared<vector<int>>(move(valuesIndex));

	// Construct the node
	auto treeNodePtr = make_shared<TreeNode>(initialUnderlyingPrice, valuesIndexPtr, probabilitiesPtr);
	treeNodesI.push_back(move(treeNodePtr));
	auto treeNodesIPtr = make_shared<vector<shared_ptr<TreeNode>>>(move(treeNodesI));
	return treeNodesIPtr;
}


// given the previous nodes at the previous time point, constructs the nodes at the current time point
std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>> models::LogNormalDiffusionTreeHelper::constructRecombiningTreeNodes(
	std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>& previousNodes, const double& upperLimit, const double& lowerLimit,  
	const std::shared_ptr<std::vector<double>> diffusionStatesPtr, const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr,
	const bool& isDividendPaid, const double dividendAmount, const bool& isLastTime)
{
	vector<shared_ptr<TreeNode>> treeNodesI;
	auto nPreviousNodes = previousNodes->size();
	auto nMaxCurrentTimeNodes = nPreviousNodes + 1; // the maximum number of nodes at the current time step
	treeNodesI.reserve(nMaxCurrentTimeNodes);


	// Add the first point 
	// ---------------------------------------------------------------------------

	// the first point for all other times will be an up move from the first node at the previous time step.
	// the next point will then be a down move from the first node at the previous time step. 
	auto nextTimeIndex = 0;
	auto upValue = previousNodes->at(0)->getValue() * exp(diffusionStatesPtr->at(0)); 
	// Check whether the upper limit has been hit
	if (upValue < upperLimit)
	{
		vector<int> valuesIndex = { 0, 1 };
		auto valuesIndexPtr = make_shared<vector<int>>(move(valuesIndex));
		auto probabilitiesPtr = diffusionProbabilitiesPtr;
		auto treeNode = isLastTime ? make_shared<TreeNode>(upValue) : make_shared<TreeNode>(upValue, valuesIndexPtr, probabilitiesPtr);
		treeNodesI.push_back(treeNode);
		nextTimeIndex++;
	}
	else // the upper limit has been hit, so the first node of the previous time point needs to be amended such that the price always goes down
	{
		auto valuesIndexPtr = make_shared<vector<int>>(vector<int>{0}); // the first value at the current time will be a down move
		auto probabilitiesPtr = make_shared<vector<double>>(vector<double>{1.0});
		previousNodes->at(0)->setForwardValuesIndex(valuesIndexPtr);
		previousNodes->at(0)->setForwardProbabilities(probabilitiesPtr);
	}


	// Add the remaining points 
	// ---------------------------------------------------------------------------

	for (int j = 0; j < nPreviousNodes; j++)
	{
		auto downValue = previousNodes->at(j)->getValue() * exp(diffusionStatesPtr->at(1));
		// Check whether the lower bound, or the zero absorbing boundary (due to the payment of the dividend) has been hit
		if ((isDividendPaid && (downValue - dividendAmount < 0.00000001)) || downValue < lowerLimit) // remove down move from previous node
		{
			auto upIndex = previousNodes->at(j)->getForwardValuesIndex()->at(0);
			auto valuesIndexPtr = make_shared<vector<int>>(vector<int>{upIndex}); // the first value at the current time will be a down move
			auto probabilitiesPtr = make_shared<vector<double>>(vector<double>{1.0});
			previousNodes->at(j)->setForwardValuesIndex(valuesIndexPtr);
			previousNodes->at(j)->setForwardProbabilities(probabilitiesPtr);
		}
		else
		{
			auto upIndex = nextTimeIndex;
			nextTimeIndex++;
			auto downIndex = nextTimeIndex;
			auto valuesIndexPtr = make_shared<vector<int>>(vector<int>{move(upIndex), move(downIndex)});
			auto probabilitiesPtr = diffusionProbabilitiesPtr;
			auto treeNode = isLastTime ? make_shared<TreeNode>(downValue) : make_shared<TreeNode>(downValue, valuesIndexPtr, probabilitiesPtr);
			treeNodesI.push_back(treeNode);
		}
	}


	// Construct the nodes
	// ---------------------------------------------------------------------------

	auto treeNodesIPtr = make_shared<vector<shared_ptr<TreeNode>>>(move(treeNodesI));
	return treeNodesIPtr;
}


// Deduct dividends from the tree
void models::LogNormalDiffusionTreeHelper::deductDividend
	(std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>& treePtr,
	const double& dividendTime, const double& dividendAmount, const double& timeToExpiry, const double& timeStepSize, const int& nTimeSteps)
{
	// Loop through each time step after the dividend payment, and the deduct the dividend from each tree node
	if (dividendTime <= timeToExpiry + 0.00000001)
	{
		auto dividendPaymentTimeStep = (int)ceil(dividendTime / timeStepSize); // round up if the dividend is paid between discretisation times
		for (int i = dividendPaymentTimeStep; i < nTimeSteps + 1; i++)
		{
			for (int j = 0; j < treePtr->at(i)->size(); j++)
			{
				auto value = fmax(0.0, treePtr->at(i)->at(j)->getValue() - dividendAmount);
				treePtr->at(i)->at(j)->setValue(value);
			}
		}
	}

}



// Construct a jump diffusion tree
std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>> 
	models::LogNormalDiffusionTreeHelper::constructJumpDiffusionTree(const int nTimeSteps, const double timeToExpiry,
		const double jumpTime,  const double dividendTime, const double dividendAmount, const double initialUnderlyingPrice,
		const std::shared_ptr<std::vector<double>> jumpDiffusionStatesPtr,
		const std::shared_ptr<std::vector<double>> jumpDiffusionProbabilitiesPtr,
		const std::shared_ptr<std::vector<double>> diffusionStatesPtr,
		const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr)
{
	// Initialise tree
	// ---------------------------------------------------------------------------
	vector<shared_ptr<vector<shared_ptr<TreeNode>>>> treeNodes;
	auto timeStepSize = timeToExpiry / (double)nTimeSteps;
	treeNodes.reserve(nTimeSteps + 1); // if there are two time steps, then there are a total of 3 times


	// Construct the tree in forward time
	// ---------------------------------------------------------------------------

	// Nodes are deducted by the amount of the dividend in the next stage of the calculation. At this stage only the $0 absorbing boundary is enforced.

	auto nPreviousTimeNodes = 0; // total number of nodes at the previous time step
	auto nCurrentTimeNodes = 0; // total number of nodes at the current time steps
	auto nFutureNodesPerCurrentNode = 0; // the number of states to which we branch out from each node at the current time
	auto nCurrentNodesPerPreviousNode = 2;
	auto currentTimeIsRecombining = true; // identifies if the tree is still in recombining phase 
	auto nextTimeIsRecombining = true; // identifies if the tree is recombining in the next phase
	for (int i = 0; i < nTimeSteps + 1; i++)
	{
		vector<double> states;
		vector<double> statesProbabilities;
		auto time = timeStepSize * (double)i;
		// Determine if the jump has fallen within the last time step
		if ((jumpTime > time - timeStepSize + 0.00000001) && (jumpTime <= time + 0.00000001))
		{
			nCurrentNodesPerPreviousNode = jumpDiffusionProbabilitiesPtr->size();
			nCurrentTimeNodes = nPreviousTimeNodes * nCurrentNodesPerPreviousNode; // 2 diffusion states, 5 jump states
			nFutureNodesPerCurrentNode = 2; // all subsequent times will only have diffusion 
			currentTimeIsRecombining = false;
			nextTimeIsRecombining = false;
		}
		else if (jumpTime <= time - timeStepSize + 0.00000001) // the jump has already happened
		{
			nCurrentNodesPerPreviousNode = 2;
			nCurrentTimeNodes = nPreviousTimeNodes * nCurrentNodesPerPreviousNode; // 2 diffusion states
			nFutureNodesPerCurrentNode = 2;
			currentTimeIsRecombining = false;
			nextTimeIsRecombining = false;
		}
		else // the jump has not yet happened, so we are still a recombining tree
		{
			nCurrentTimeNodes = i + 1;
			nCurrentNodesPerPreviousNode = 2;
			currentTimeIsRecombining = true;
			if ((jumpTime <= time + timeStepSize + 0.00000001) && (jumpTime > time + 0.00000001)) // does the jump happen in the next time step
			{
				nFutureNodesPerCurrentNode = jumpDiffusionProbabilitiesPtr->size();
				nextTimeIsRecombining = false;
			}
			else
			{
				nFutureNodesPerCurrentNode = 2;
				nextTimeIsRecombining = true;
			}
		}


		vector<shared_ptr<TreeNode>> treeNodesI;
		treeNodesI.reserve(nCurrentTimeNodes);

		// Determine if the dividend falls within the current time step. This is used to enforce the 0 absorbing boundary. 
		auto dividendDeduction = (dividendTime >= time - 0.00000001) && (dividendTime < time + timeStepSize - 0.00000001) ? dividendAmount : 0.0;

		if (i == 0) // i.e. the start of the tree
		{
			if (nextTimeIsRecombining == true)
			{
				vector<int> futureValuesIndex = { 0, 1 };
				auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
				auto treeNode = make_shared<TreeNode>(initialUnderlyingPrice, futureValuesIndexPtr, diffusionProbabilitiesPtr);
				treeNodesI.push_back(treeNode);
			}
			else // i.e. the jump happens within the first time step
			{
				auto index = 0;
				vector<int> futureValuesIndex;
				futureValuesIndex.reserve(nFutureNodesPerCurrentNode);
				for (int k = 0; k < nFutureNodesPerCurrentNode; k++)
				{
					futureValuesIndex.push_back(index);
					index++;
				}
				auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
				auto treeNode = make_shared<TreeNode>(initialUnderlyingPrice, futureValuesIndexPtr, jumpDiffusionProbabilitiesPtr);
				treeNodesI.push_back(treeNode);
			}
		}

		if (i != 0 && i != nTimeSteps) // i.e. the middle of the tree
		{
			if (currentTimeIsRecombining == true && nextTimeIsRecombining == true) // i.e. construct a recombining tree
			{
				// the first point for all other times will be an up move from the first node at the previous time step.
				// the next point will then be a down move from the first node at the previous time step. 
				auto upValue = treeNodes[i - 1]->at(0)->getValue() * exp(diffusionStatesPtr->at(0));
				vector<int> futureValuesIndex = { 0, 1 };
				auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
				auto treeNode = make_shared<TreeNode>(upValue, futureValuesIndexPtr, diffusionProbabilitiesPtr);
				treeNodesI.push_back(treeNode);
				for (int j = 1; j < nCurrentTimeNodes; j++)
				{
					auto downValue = fmax(0.0, treeNodes[i - 1]->at(j - 1)->getValue() * exp(diffusionStatesPtr->at(1)));
					if (downValue - dividendDeduction < 0.00000001) // although the dividend should be deducted at the end, 0.0 is an absorbing boundary
						downValue = 0.0;
					vector<int> futureValuesIndex = { j, j + 1 };
					auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
					auto treeNode = make_shared<TreeNode>(downValue, futureValuesIndexPtr, diffusionProbabilitiesPtr);
					treeNodesI.push_back(treeNode);
				}
			}

			if (currentTimeIsRecombining == true && nextTimeIsRecombining == false)
			{
				// Current node is recombining
				// the first point will be an up move from the first node at the previous time step.
				// the next point will then be a down move from the first node at the previous time step. 
				auto upValue = treeNodes[i - 1]->at(0)->getValue() * exp(diffusionStatesPtr->at(0));
				// Future nodes are not recombining
				auto index = 0;
				vector<int> futureValuesIndex;
				futureValuesIndex.reserve(nFutureNodesPerCurrentNode);
				for (int k = 0; k < nFutureNodesPerCurrentNode; k++)
				{
					futureValuesIndex.push_back(index);
					index++;
				}
				auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
				auto treeNode = make_shared<TreeNode>(upValue, futureValuesIndexPtr, jumpDiffusionProbabilitiesPtr);
				treeNodesI.push_back(treeNode);

				// The remaining nodes at the current time will be down moves
				for (int j = 1; j < nCurrentTimeNodes; j++)
				{
					// Current node is recombining
					auto downValue = fmax(0.0, treeNodes[i - 1]->at(j - 1)->getValue() * exp(diffusionStatesPtr->at(1)));
					if (downValue - dividendDeduction < 0.00000001) // although the dividend should be deducted at the end, 0.0 is an absorbing boundary
						downValue = 0.0;
					// Future nodes are not recombining
					vector<int> futureValuesIndex;
					futureValuesIndex.reserve(nFutureNodesPerCurrentNode);
					for (int k = 0; k < nFutureNodesPerCurrentNode; k++)
					{
						futureValuesIndex.push_back(index);
						index++;
					}
					auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
					auto treeNode = make_shared<TreeNode>(downValue, futureValuesIndexPtr, jumpDiffusionProbabilitiesPtr);
					treeNodesI.push_back(treeNode);
				}
			}

			if (currentTimeIsRecombining == false) // i.e. construct a non-recombining tree
			{
				auto index = 0;
				for (int j = 0; j < nPreviousTimeNodes; j++)
				{
					auto statePtr = nCurrentNodesPerPreviousNode == 2 ? diffusionStatesPtr : jumpDiffusionStatesPtr;
					auto probabilitiesPtr = nFutureNodesPerCurrentNode == 2 ? diffusionProbabilitiesPtr : jumpDiffusionProbabilitiesPtr;

					for (int k = 0; k < statePtr->size(); k++)
					{
						auto value = fmax(0.0, treeNodes[i - 1]->at(j)->getValue() * exp(statePtr->at(k)));
						if (value - dividendDeduction < 0.00000001) // although the dividend should be deducted at the end, 0.0 is an absorbing boundary
							value = 0.0;
						// Future nodes are not recombining
						vector<int> futureValuesIndex;
						futureValuesIndex.reserve(nFutureNodesPerCurrentNode);
						for (int k = 0; k < nFutureNodesPerCurrentNode; k++)
						{
							futureValuesIndex.push_back(index);
							index++;
						}
						auto futureValuesIndexPtr = make_shared<vector<int>>(move(futureValuesIndex));
						auto treeNode = make_shared<TreeNode>(value, futureValuesIndexPtr, probabilitiesPtr);
						treeNodesI.push_back(treeNode);
					}
				}
			}
		}

		if (i == nTimeSteps) // i.e. the end of the tree (it is more efficient to split out the nTimeSteps iteration from the middle of tree 
		{
			if (currentTimeIsRecombining == true) // i.e. construct a recombining tree
			{
				// the first point for all other times will be an up move from the first node at the previous time step.
				// the next point will then be a down move from the first node at the previous time step. 
				auto upValue = treeNodes[i - 1]->at(0)->getValue() * exp(diffusionStatesPtr->at(0));
				auto treeNode = make_shared<TreeNode>(upValue);
				treeNodesI.push_back(treeNode);
				for (int j = 1; j < nCurrentTimeNodes; j++)
				{
					auto downValue = fmax(0.0, treeNodes[i - 1]->at(j - 1)->getValue() * exp(diffusionStatesPtr->at(1)));
					if (downValue - dividendDeduction < 0.00000001) // although the dividend should be deducted at the end, 0.0 is an absorbing boundary
						downValue = 0.0;
					auto treeNode = make_shared<TreeNode>(downValue);
					treeNodesI.push_back(treeNode);
				}
			}
			else
			{
				for (int j = 0; j < nPreviousTimeNodes; j++)
				{
					auto statePtr = nCurrentNodesPerPreviousNode == 2 ? diffusionStatesPtr : jumpDiffusionStatesPtr;
					auto probabilitiesPtr = nCurrentNodesPerPreviousNode == 2 ? diffusionProbabilitiesPtr : jumpDiffusionProbabilitiesPtr;

					for (int k = 0; k < statePtr->size(); k++)
					{
						auto value = fmax(0.0, treeNodes[i - 1]->at(j)->getValue() * exp(statePtr->at(k)));
						if (value - dividendDeduction < 0.00000001) // although the dividend should be deducted at the end, 0.0 is an absorbing boundary
							value = 0.0;
						auto treeNode = make_shared<TreeNode>(value);
						treeNodesI.push_back(treeNode);
					}
				}
			}
		}


		// Add the nodes at the current time step to the tree
		treeNodes.push_back(make_shared<vector<shared_ptr<TreeNode>>>(treeNodesI));
		nPreviousTimeNodes = nCurrentTimeNodes;
	}


	// Deduct dividends from tree
	// ---------------------------------------------------------------------------
	auto treeNodesPtr = make_shared<vector<shared_ptr<vector<shared_ptr<TreeNode>>>>>(move(treeNodes));
	LogNormalDiffusionTreeHelper::deductDividend(treeNodesPtr, dividendTime, dividendAmount, timeToExpiry, timeStepSize, nTimeSteps);

	return treeNodesPtr;
}


//// DEPRECATED !!!!! ////

// Add the instantaneous jump and diffusion to the tree
// It is assumed that the recombining tree up until the jump has already been constructed. 
// For each of the previous nodes we first apply the jump. Then, for each post jump node, we start a new diffusion recombining tree.
// Once all of the post jump recombining trees have been constructed, they are aggregated back into the main tree. 
// <nTimeSteps> is interpreted as the number of time steps, from the end of the existing recombining tree, till the time to expiry.
// <timeToExpiry> is interpreted as the time to expiry from the end of the existing recombing tree.
// <upperLimitStandardDeviation> and <lowerLimitStandardDeviation> represent the max and minimun values of the diffusion, in terms of the 
// volatility of the diffusion process.

/*
std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>
	models::LogNormalDiffusionTreeHelper::constructJumpDiffusionTreeNodes(
	std::shared_ptr<Tree> tree, const int nPostJumpTimeSteps, const double postJumpTimeToExpiry,
	const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation, const double impliedVolatility, 
	const std::shared_ptr<std::vector<double>> jumpStatesPtr, 
	const std::shared_ptr<std::vector<double>> jumpProbabilitiesPtr, 
	const std::shared_ptr<std::vector<double>> diffusionStatesPtr,
	const std::shared_ptr<std::vector<double>> diffusionProbabilitiesPtr,
	double dividendAmount, double dividendTime)
{
	// The dividend may have already been paid prior to the jump time. In that case, the dividend time will be negative. If so, set it to 
	// an arbitrary amount of time after the time to expiry
	if (dividendTime < 0.0)
	{
		dividendTime = postJumpTimeToExpiry + 1.0;
		dividendAmount = 0.0;
	}


	// Extend the capacity of the tree to take in more points.
	auto treeNodes = tree->getTreeNodes();
	vector<shared_ptr<vector<shared_ptr<TreeNode>>>> extendedTreeNodes;
	extendedTreeNodes.reserve(nPostJumpTimeSteps + tree->getNTimesSteps());
	for (int i = 0; i < treeNodes->size(); i++)
		extendedTreeNodes.push_back(treeNodes->at(i));


	// For each node at the previous time step, apply the jump, and then construct a new recombining tree for each jumped value.
	auto previousNodesPtr = tree->getTreeNodes()->at(tree->getNTimesSteps());
	auto nPreviousNodes = previousNodesPtr->size();
	auto nJumpNodes = jumpProbabilitiesPtr->size();
	auto nJumpDiffusionNodes = nPreviousNodes * nJumpNodes;
	vector<shared_ptr<vector<shared_ptr<vector<shared_ptr<TreeNode>>>>>> jumpedRecombiningTree;
	jumpedRecombiningTree.reserve(nJumpDiffusionNodes);


	auto forwardIndex = 0; // the total number of points at the time immediately after the jump
	auto currentIndex = 0;
	for (int j = 0; j < nPreviousNodes; j++)
	{
		// Construct the recombining tree for each pure jump
		for (int k = 0; k < nJumpNodes; k++)
		{
			auto jumpValue = previousNodesPtr->at(j)->getValue() * exp(jumpStatesPtr->at(k));
			auto jumpValueTree = LogNormalDiffusionTreeHelper::constructRecombiningTree(jumpValue, nPostJumpTimeSteps, postJumpTimeToExpiry,
				impliedVolatility, upperLimitStandardDeviation, lowerLimitStandardDeviation, dividendTime, dividendAmount, diffusionStatesPtr,
				diffusionProbabilitiesPtr);
			jumpedRecombiningTree.push_back(jumpValueTree);

		}

		// Adjust the future index and probabilities of the previous node. 
		vector<double> probabilities;
		probabilities.reserve(nJumpDiffusionNodes); // the maximum number of probabilities (could be less if for example 0 is hit)
		for (int k = 0; k < nJumpNodes; k++)
		{
			auto jumpNode = jumpedRecombiningTree.at(j*nJumpNodes + k)->at(0)->at(0); // 0 is the pure jump
			auto jumpNodeDiffusionProbabilities = jumpNode->getForwardProbabilities(); // the up and down state probabilites from the pure jump node
			auto nForwardValues = jumpNode->getForwardValuesIndex()->size();
			forwardIndex += nForwardValues;
			for (int l = 0; l < nForwardValues; l++)
			{
				probabilities.push_back(jumpNodeDiffusionProbabilities->at(l) * jumpProbabilitiesPtr->at(k));
			}
		}

		vector<int> forwardValuesIndex;
		vector<double> forwardProbabilities;
		forwardValuesIndex.reserve(forwardIndex);
		forwardProbabilities.reserve(forwardIndex);
		for (int l = currentIndex; l < forwardIndex; l++)
		{
			forwardValuesIndex.push_back(l);
			forwardProbabilities.push_back(probabilities.at(l - currentIndex));
		}
		previousNodesPtr->at(j)->setForwardValuesIndex(make_shared<vector<int>>(move(forwardValuesIndex)));
		previousNodesPtr->at(j)->setForwardProbabilities(make_shared<vector<double>>(move(forwardProbabilities)));
		currentIndex = forwardIndex;
	}


	// Integrate the inidividual recombining trees from the jump nodes into the main tree
	for (int i = 1; i < nPostJumpTimeSteps + 1; i++) // start from 1 because we do not want to add the first point, which is just pure jump
	{
		// Loop through all of the nodes at the current time of the jump recombining trees, and reset the future values index.
		// Then, add the node to the dummy vector.
		auto forwardIndex = 0;
		auto runningIndex = 0;
		auto currentIndex = 0; // the total number of nodes at the current time step
		for (int j = 0; j < nJumpDiffusionNodes; j++)
		{
			auto valuesPtr = jumpedRecombiningTree.at(j)->at(i);
			for (int k = 0; k < valuesPtr->size(); k++)
			{
				if (i != nPostJumpTimeSteps) // the last time step does not need to be adjusted
				{
					// For each tree node, reset the index of the future tree nodes
					auto valuesIndex = valuesPtr->at(k)->getForwardValuesIndex();
					for (int l = 0; l < valuesIndex->size(); l++)
					{
						runningIndex = valuesIndex->at(l);
						valuesIndex->at(l) += forwardIndex;
					}
				}
				currentIndex++;
			}
			forwardIndex += runningIndex;
		}

		// Add the node to the dummy vector. Need to loop through the nodes again because we want to preserve memory capacity
		vector<shared_ptr<TreeNode>> treeNodesI;
		treeNodesI.reserve(currentIndex);
		for (int j = 0; j < nJumpDiffusionNodes; j++)
		{
			auto valuesPtr = jumpedRecombiningTree.at(j)->at(i);
			for (int k = 0; k < valuesPtr->size(); k++)
			{
				treeNodesI.push_back(valuesPtr->at(k));
			}
		}
		extendedTreeNodes.push_back(make_shared<vector<shared_ptr<TreeNode>>>(move(treeNodesI)));
	}

	// Finally, return the tree nodes
	auto extendedTreeNodesPtr = make_shared<vector<shared_ptr<vector<shared_ptr<TreeNode>>>>>(move(extendedTreeNodes));
	return extendedTreeNodesPtr;
}


*/
