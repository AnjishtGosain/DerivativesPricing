#ifndef __IMONTECARLOMODEL_H__ 
#define __IMONTECARLOMODEL_H__ 

#include <iostream>
#include <memory>
#include "../Enumerations/UnderlyingCode.h"

namespace models
{
	class IMonteCarloModel
	{
	public:
		virtual ~IMonteCarloModel() = default;

		// Function which returns a simulation of the model process at a future point in time. 
		virtual const enumerations::UnderlyingCode& getUnderlyingCode() const = 0;
		virtual const double& getDiscountRate() const = 0;
		virtual const std::shared_ptr<std::vector<double>> generateMonteCarloSimulations(const int& nPaths, const double& time, int seed = -1) = 0;
	};
}

#endif // !__IMONTECARLOMODEL_H__
