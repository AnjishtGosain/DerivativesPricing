#ifndef __ITREEMODEL_H__ 
#define __ITREEMODEL_H__ 

#include <vector>
#include <memory>
#include "Tree.h"
#include "../../Enumerations/UnderlyingCode.h"
#include "../../Enumerations/Implementation.h"
#include "../../Instruments/VanillaOption.h"

namespace models
{
	class ITreeModel
	{
	public:
		virtual ~ITreeModel() = default;
		virtual const enumerations::UnderlyingCode& getUnderlyingCode() const = 0;
		virtual const double& getDiscountRate() const = 0;

		virtual const std::shared_ptr<models::Tree> constructTree(const int& nTimeSteps, const double& timeToExpiry, 
			const enumerations::Implementation implementation, const double upperLimitStandardDeviation, 
			const double lowerLimitStandardDeviation) = 0; // Implementation is used to select different tree constructions
		virtual const std::shared_ptr<double> smoothedValueAtTreeNode(const double underlyingPrice,
			const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize) = 0;
		virtual const bool supportsVanillaOptionSmoothing(const double & timeStart, const double & timeEnd) = 0;
	private:
		bool m_supportsVanillaOptionSmoothing; 
	};
}

#endif // !__ITREEMODEL_H__
