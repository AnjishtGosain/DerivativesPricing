#ifndef __DIFFERENTIALEVOLUTION_H__ 
#define __DIFFERENTIALEVOLUTION_H__

#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <functional>
#include "../Enumerations/Implementation.h"

namespace optimisers 
{
	class DifferentialEvolution  
	{
	public:
		DifferentialEvolution(const int& D, const double& F, const double& CR, const std::shared_ptr<std::vector<double>>& lowerBounds,
			const std::shared_ptr<std::vector<double>>& upperBounds, 
			const std::function<double (const std::shared_ptr<std::vector<double>>&, const int&)>& functionToOptimise, 
			const enumerations::Implementation implementation, const int& initialPopulationParameter, int seed = -1);
		DifferentialEvolution() = default;
		~DifferentialEvolution() = default;

		// Analytic Pricing functions
		const std::shared_ptr<std::vector<double>> solve(const double& tolerance);

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		DifferentialEvolution& operator = (DifferentialEvolution const&) = delete;
		DifferentialEvolution(DifferentialEvolution const&) = delete;

	private:
		int m_D;
		int m_N;
		double m_F;
		double m_CR;
		std::shared_ptr<std::vector<double>> m_lowerBounds;
		std::shared_ptr<std::vector<double>> m_upperBounds;
		std::mt19937 m_generator;
		std::function<double(const std::shared_ptr<std::vector<double>>&, const int&)> m_functionToOptimise;
		std::vector<double> m_functionValues;
		int m_seed;
		enumerations::Implementation m_implementation;

		// Setters
		void setD(const int& value);
		void setNAndImplementation(const enumerations::Implementation& implementation, const int& value);
		void setF(const double& value);
		void setCR(const double& value);
		void setLowerBounds(const std::shared_ptr<std::vector<double>>& value);
		void setUpperBounds(const std::shared_ptr<std::vector<double>>& value);
		void setFunctionToOptimise(const std::function<double(const std::shared_ptr<std::vector<double>>&, const int&)>& value) 
			{ m_functionToOptimise = value; }
		void setSeed(int value);

		// Vectors required in the algorithm
		std::vector<std::vector<double>> m_targetVectors;
		std::vector<std::vector<double>> m_donorVectors;
		std::vector<std::vector<double>> m_trialVectors;

		// Functions for vector construction
		void constructInitialTargetVectors();
		void constructDonorVectors();
		void constructTrialVectors();
	};
}

#endif // !__DIFFERENTIALEVOLUTION_H__
