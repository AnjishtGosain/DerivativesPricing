#include <random>
#include <algorithm>
#include "DifferentialEvolution.h"
#include <chrono>

using namespace std;
using namespace enumerations;
using namespace std::chrono;

optimisers::DifferentialEvolution::DifferentialEvolution(const int & D, const double & F, const double & CR, 
	const std::shared_ptr<std::vector<double>>& lowerBounds, const std::shared_ptr<std::vector<double>>& upperBounds, 
	const std::function<double(const std::shared_ptr<std::vector<double>>&, const int&)>& functionToOptimise, 
	const enumerations::Implementation implementation, const int & initialPopulationParameter, int seed)
{
	setD(D);
	setNAndImplementation(implementation, initialPopulationParameter);
	setCR(CR);
	setCR(CR);
	setLowerBounds(lowerBounds);
	setUpperBounds(upperBounds);
	setFunctionToOptimise(functionToOptimise);
	setSeed(seed);

	std::vector<double> m_functionValues; // used to store the values of the target vectors for the current iteration
	m_functionValues.reserve(m_N);
}

void optimisers::DifferentialEvolution::setD(const int& value)
{
	if (value < 1)
		throw invalid_argument("The number of parameters to solve must be greater than 0.");
	m_D = value;
}

void optimisers::DifferentialEvolution::setNAndImplementation(const enumerations::Implementation& implementation, const int& value)
{
	m_implementation = implementation;
	switch (m_implementation)
	{
	case Implementation::One:
		if(value < 4)
			throw invalid_argument("The number of target vectors must be 4 or greater.");
		m_N = value;
		break;
	case Implementation::Two:
		if(pow(value, m_D) < 4)
			throw invalid_argument("The number of target vectors must be 4 or greater.");
		m_N = pow(value, m_D);
	default:
		break;
	}
}

void optimisers::DifferentialEvolution::setF(const double& value)
{
	if (value < -0.00000001 || value > 2.00000001)
		throw invalid_argument("The constant F must be within the interval [0,2].");
	m_F = value;
}

void optimisers::DifferentialEvolution::setCR(const double& value)
{
	if (value < -0.00000001 || value > 1.00000001)
		throw invalid_argument("CR is a probability, and so must be between 0 and 1.");
	m_CR = value;
}

void optimisers::DifferentialEvolution::setLowerBounds(const std::shared_ptr<std::vector<double>>& value)
{
	if (value->size() != m_D) // the lower bound must be finite as the intial target vectors are sampled from a uniform distribution
		throw invalid_argument("The number of lower bounds do not match up with the number of parameters.");
	m_lowerBounds = value;
}

void optimisers::DifferentialEvolution::setUpperBounds(const std::shared_ptr<std::vector<double>>& value)
{
	if (value->size() != m_D) // the upper bound must be finite as the intial target vectors are sampled from a uniform distribution
		throw invalid_argument("The number of upper bounds do not match up with the number of parameters.");
	m_upperBounds = value;
}

void optimisers::DifferentialEvolution::setSeed(int value)
{
	if (value == -1) // for the default seed value, generate a new random seed
	{
		random_device rd;
		m_seed = rd();
	}
	else
	{
		m_seed = value;
	}
	std::mt19937 m_generator(m_seed); // initialise the Mersenne Twister generator to be used in the optimisation
}

// This function performs the Differential Evolution method. It is assumed that the minimum of the function provided for optimisation is 0
const std::shared_ptr<std::vector<double>> optimisers::DifferentialEvolution::solve(const double& tolerance)
{
	vector<double> solution;
	solution.reserve(m_D);

	// Construct the initial the target vectors
	constructInitialTargetVectors();
	auto minDiff = 10000.0;
	auto currentIteration = 0;
	high_resolution_clock::time_point tStart = high_resolution_clock::now();

	int minSolutionIndex;
	while (minDiff > tolerance)
	{
		constructDonorVectors();
		constructTrialVectors();

		// check whether any of the N target vectors have hit the minimum. If so, the solution is the first vector to do so.
		auto minSolutionIterator = std::min_element(m_functionValues.begin(), m_functionValues.end());
		std::cout << "Current Iterator = " << currentIteration << "\t Minimum target vector function value = " << *minSolutionIterator << endl;

		minSolutionIndex = distance(m_functionValues.begin(), minSolutionIterator);
		// cout the value
		high_resolution_clock::time_point tEnd = high_resolution_clock::now();
		std::cout << "Optimisation time " << duration_cast<seconds>(tEnd - tStart).count() << " seconds." << std::endl;
		for (int i = 0; i < m_D; i++)
		{
			std::cout << "Solution " << i << " = " << m_targetVectors.at(minSolutionIndex).at(i) << std::endl;
		}

		currentIteration++;
		minDiff = *minSolutionIterator;

	}

	auto solutionPtr = make_shared<vector<double>>(move(m_targetVectors.at(minSolutionIndex)));
	return solutionPtr;
}

// Method which constructs the initial target vectors
void optimisers::DifferentialEvolution::constructInitialTargetVectors()
{
	// Initialise the containers for the target vectors
	vector<vector<double>> targetVectors;
	targetVectors.reserve(m_N);
	for (int i = 0; i < m_N; i++)
	{
		vector<double> targetVector;
		targetVector.reserve(m_D);
		targetVectors.push_back(move(targetVector));
	}

	// Construct the target vector based on the selected implementation
	switch (m_implementation)
	{
	case Implementation::One: // Sample from the state space using psuedo random randoms
		for (int j = 0; j < m_D; j++)
		{
			// Construct uniform distribution for the current parameter
			uniform_real_distribution<double> uniformDistribution(m_lowerBounds->at(j), m_upperBounds->at(j));
			for (int i = 0; i < m_N; i++)
			{
				auto simulatedValue = uniformDistribution(m_generator);
				targetVectors.at(i).push_back(move(simulatedValue));
			}
		}
		break;
	case Implementation::Two: // construct a uniform grid on the state space
		/*
		auto samplesPerParameter = (int)pow(m_N, 1.0 / (double)m_D);
		for (int j = 0; j < m_D; j++)
		{
			auto stepSize = (m_upperBounds->at(j) - m_lowerBounds->at(j)) / samplesPerParameter;
			for (int i = 0; i < m_N; i++)
			{
				auto simulatedValue = uniformDistribution(m_generator);
				targetVectors.at(i).push_back(move(simulatedValue));
			}
		}
		*/
		break;
	default:
		throw invalid_argument("The Differential Evolution algorithm only supports two different implementations for the construction of the initial target vectors");
		break;


	}
	m_targetVectors = move(targetVectors);
	return;
}

void optimisers::DifferentialEvolution::constructDonorVectors()
{
	// Initialise the containers for the donor vectors
	vector<vector<double>> donorVectors;
	donorVectors.reserve(m_N);
	for (int i = 0; i < m_N; i++)
	{
		vector<double> donorVector;
		donorVector.reserve(m_D);
		donorVectors.push_back(move(donorVector));
	}

	// For each of the target vectors, construct a new donor vector as the mutation of a random selection of three of the other target vectors

	for (int i = 0; i < m_N; i++)
	{
		// Construct uniform integer distribution for the current target vector
		// Implement a simplified Fisher-Yates algorithm. In essense, we are sampling form a vector without replacement.
		// For the ith iteration, swap the ith target with the 0th target. 
		// Generate a random integer between 1 and m_N - 1 (the maximum index). Swap the target at the generated index with the target at the second intdex.
		// Generate a random integer between 2 and m_N - 1. Swap the target at the generated index with the target at the third index.
		// Generate a random integer between 3 and m_N - 1. Swap the target at the generated index with the target at the fourth index.

		// Construct a vector with integers from 0 to m_N - 1
		vector<int> initialIndex;
		initialIndex.reserve(m_N);
		for (int k = 0; k < m_N; k++)
			initialIndex.push_back(k);

		// Construct uniform integer distributions
		uniform_int_distribution<int> uniformIntegerDistribution1(1, m_N - 1);
		uniform_int_distribution<int> uniformIntegerDistribution2(2, m_N - 1);
		uniform_int_distribution<int> uniformIntegerDistribution3(3, m_N - 1);

		// Generate samples
		auto randomIndex1 = uniformIntegerDistribution1(m_generator);
		auto randomIndex2 = uniformIntegerDistribution2(m_generator);
		auto randomIndex3 = uniformIntegerDistribution3(m_generator);

		// Start the swaping
		swap(initialIndex[0], initialIndex[i]);
		swap(initialIndex[1], initialIndex[randomIndex1]);
		swap(initialIndex[2], initialIndex[randomIndex2]);
		swap(initialIndex[3], initialIndex[randomIndex3]);
		/*HACK
		initialIndex.reserve(4);
		initialIndex.push_back(i);
		uniform_int_distribution<int> uniformIntegerDistribution1(0, m_N - 1);
		for (int a = 0; a < 3; a++)
		{
			auto hit = false;
			while (hit == false)
			{
				auto randomIndex = uniformIntegerDistribution1(m_generator);
				if (find(initialIndex.begin(), initialIndex.end(), randomIndex) == initialIndex.end())
				{
					initialIndex.push_back(randomIndex);
					hit = true;
				}
			}
		}
		*/

		// Mutate the random generated vectors
		for (int j = 0; j < m_D; j++)
		{
			auto tempValue = m_targetVectors.at(initialIndex[1]).at(j) + m_F * m_targetVectors.at(initialIndex[2]).at(j) - 
				m_F * m_targetVectors.at(initialIndex[3]).at(j);
			// Apply constraints to temp value
			auto tempValue2 = fmin(m_upperBounds->at(j), fmax(tempValue, m_lowerBounds->at(j)));
			donorVectors.at(i).push_back(move(tempValue2));
		}
	}
	m_donorVectors = move(donorVectors);
	return;
}

void optimisers::DifferentialEvolution::constructTrialVectors()
{
	// Initialise the containers for the trial vectors
	vector<vector<double>> trialVectors;
	trialVectors.reserve(m_N);
	for (int i = 0; i < m_N; i++)
	{
		vector<double> trialVector;
		trialVector.reserve(m_D);
		trialVectors.push_back(move(trialVector));
	}

	// For each of the target vectors, construct a new donor vector as the mutation of a random selection of three of the other target vectors

	for (int i = 0; i < m_N; i++)
	{
		// Randomly simulate the index of one of the parameters
		uniform_int_distribution<int> uniformIntegerDistribution(0, m_D - 1);
		auto randomIndex = uniformIntegerDistribution(m_generator);

		// Recombine the target and donor vectors
		for (int j = 0; j < m_D; j++)
		{
			// Simulate standard uniform for comparision to CR
			uniform_real_distribution<double> uniformRealDistribution(0, 1);
			auto randomCriterion = uniformRealDistribution(m_generator);

			auto tempValue = (randomCriterion <= m_CR || j == randomIndex) ? m_donorVectors.at(i).at(j) : m_targetVectors.at(i).at(j);
			trialVectors.at(i).push_back(tempValue);

		}
	}
	m_trialVectors = move(trialVectors);


	// Initialise the container for the function value vector
	vector<double> functionValues;
	functionValues.reserve(m_N);

	// compare function value for the current ith taget vector against that from the ith trail vector, the select the vector with the lower value

	for (int i = 0; i < m_N; i++)
	{
		auto targetVectorPtr = make_shared<vector<double>>(m_targetVectors.at(i));
		auto trialVectorPtr = make_shared<vector<double>>(m_trialVectors.at(i));
		auto targetFunctionValue = m_functionToOptimise(targetVectorPtr, m_D);
		auto trialFunctionValue = m_functionToOptimise(trialVectorPtr, m_D);
		functionValues.push_back(targetFunctionValue);
		if (trialFunctionValue < targetFunctionValue)
		{
			m_targetVectors.at(i) = move(m_trialVectors.at(i));
			functionValues.at(i) = move(trialFunctionValue);
		}
	}
	m_functionValues = move(functionValues);
	return;
}


