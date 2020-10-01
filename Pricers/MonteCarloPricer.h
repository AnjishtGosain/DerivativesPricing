#ifndef __MONTECARLOPRICER_H__
#define __MONTECARLOPRICER_H__

#include <iostream>
#include <memory>
#include <vector>
#include "../Models/MonteCarloModelUtilities/IMonteCarloModel.h"
#include "../Instruments/VanillaOption.h"

namespace pricers
{
	//// Pricing of vanilla options based on Monte Carlo methods.
	class MonteCarloPricer
	{
	public:
		MonteCarloPricer(const std::shared_ptr<models::IMonteCarloModel>& model);
		MonteCarloPricer() = default;
		~MonteCarloPricer() = default;

		// Getters
		const std::shared_ptr<models::IMonteCarloModel> getModel() const { return m_model; }

		// Setters
		void setModel(const std::shared_ptr<models::IMonteCarloModel>& value) { m_model = value; }

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		MonteCarloPricer& operator = (MonteCarloPricer const&) = delete;
		MonteCarloPricer(MonteCarloPricer const&) = delete;

		// Pricing functions
		const std::shared_ptr<std::vector<std::shared_ptr<double>>> price(const int& nPaths, 
			const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions, int seed = -1);
		static const std::shared_ptr<double> price(const double& discountRate, const std::shared_ptr<std::vector<double>>& simulatedUnderlyingPrice,
			const std::shared_ptr<instruments::VanillaOption>& vanillaOption);

	private:
		std::shared_ptr<models::IMonteCarloModel> m_model;
	};
}

#endif // !__MONTECARLOPRICER_H__
