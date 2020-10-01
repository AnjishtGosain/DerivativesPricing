#ifndef __ANALYTICPRICER_H__
#define __ANALYTICPRICER_H__

#include <iostream>
#include <memory>
#include <vector>
#include "../Models/AnalyticModelUtilities/IAnalyticModel.h"
#include "../Instruments/VanillaOption.h"

namespace pricers
{
	//// Pricing of vanilla options based on analytic methods. For example, the Black Scholes closed-form solution for the price of European options.
	class AnalyticPricer
	{
	public:
		AnalyticPricer(const std::shared_ptr<models::IAnalyticModel>& model);
		AnalyticPricer() = default;
		~AnalyticPricer() = default;

		// Getters
		const std::shared_ptr<models::IAnalyticModel> getModel() const { return m_model; }

		// Setters
		void setModel(const std::shared_ptr<models::IAnalyticModel>& value) { m_model = value; }

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		AnalyticPricer& operator = (AnalyticPricer const&) = delete;
		AnalyticPricer(AnalyticPricer const&) = delete;

		// Pricing functions
		const std::shared_ptr<std::vector<std::shared_ptr<double>>> price( 
			const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions);
		const std::shared_ptr<double> price(const std::shared_ptr<instruments::VanillaOption>& vanillaOption);

	private:
		std::shared_ptr<models::IAnalyticModel> m_model;
	};
}

#endif // !__ANALYTICPRICER_H__
