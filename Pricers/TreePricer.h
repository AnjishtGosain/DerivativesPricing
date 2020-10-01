#ifndef __TREEPRICER_H__
#define __TREEPRICER_H__

#include <iostream>
#include <memory>
#include <vector>
#include "../Models/TreeModelUtilities/ITreeModel.h"
#include "../Models/TreeModelUtilities/Tree.h"
#include "../Instruments/VanillaOption.h"
#include "../Enumerations/Implementation.h"

namespace pricers
{
	//// Pricing of vanilla options based on tree methods. e.g. binomial trees
	class TreePricer
	{
	public:
		TreePricer(const std::shared_ptr<models::ITreeModel>& model);
		TreePricer() = default;
		~TreePricer() = default;

		// Getters
		const std::shared_ptr<models::ITreeModel> getModel() const { return m_model; }

		// Setters
		void setModel(const std::shared_ptr<models::ITreeModel>& value) { m_model = value; }

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		TreePricer& operator = (TreePricer const&) = delete;
		TreePricer(TreePricer const&) = delete;

		// Pricing functions
		const std::shared_ptr<std::vector<std::shared_ptr<double>>> price(const int nTimeSteps,
			const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> vanillaOptions, const bool useVanillaOptionSmoothing,
			const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation);

		const std::shared_ptr<std::vector<std::shared_ptr<double>>> priceWithRichardsonExtrapolation(const int& nTimeSteps,
			const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptions, const bool& useVanillaOptionSmoothing,
			const enumerations::Implementation& implementation, const double& upperLimitStandardDeviation, const double& lowerLimitStandardDeviation);

		const std::shared_ptr<double> price(const double discountRate, const std::shared_ptr<models::Tree> tree,
			const std::shared_ptr<instruments::VanillaOption> vanillaOption, const bool useVanillaOptionSmoothing);

	private:
		std::shared_ptr<models::ITreeModel> m_model;
	};
}

#endif // !__TREEPRICER_H__
