#ifndef __IANALYTICMODEL_H__ 
#define __IANALYTICMODEL_H__ 

#include <memory>
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Enumerations/ExerciseType.h"

namespace models
{
	class IAnalyticModel
	{
	public:
		virtual ~IAnalyticModel() = default;
		virtual const enumerations::UnderlyingCode& getUnderlyingCode() const = 0;
		virtual const std::shared_ptr<double> calculateAnalyticSolution(const double& strike, const double& timeToExpiry,
			const enumerations::OptionRight& optionRight, const enumerations::ExerciseType& exerciseType) = 0;
	};
}

#endif // !__IANALYTICMODEL_H__
