#ifndef __BLACKSCHOLESSINGLENORMALJUMP_H__ 
#define __BLACKSCHOLESSINGLENORMALJUMP_H__

#include <iostream>
#include <vector>
#include <memory>
#include "AnalyticModelUtilities/IAnalyticModel.h"
#include "MonteCarloModelUtilities/IMonteCarloModel.h"
#include "TreeModelUtilities/ITreeModel.h"
#include "TreeModelUtilities/Tree.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/Implementation.h"

namespace models
{
	class BlackScholesSingleNormalJump : public IMonteCarloModel, public ITreeModel
	{
	public:
		BlackScholesSingleNormalJump(const double& costOfCarry, const double& discountRate, const double& impliedVolatility, 
			const double& initialUnderlyingPrice, const enumerations::UnderlyingCode& underlyingCode, const double& dividendTime, 
			const double& dividendAmount, const double& jumpTime, const double& jumpMean, const double& jumpVolatility);
		BlackScholesSingleNormalJump() = default;
		~BlackScholesSingleNormalJump() = default;

		// Getters
		const double& getCostOfCarry() const { return m_costOfCarry; }
		const double& getDiscountRate() const { return m_discountRate; }
		const double& getImpliedVolatility() const { return m_impliedVolatility; }
		const double& getInitialUnderlyingPrice() const { return m_initialUnderlyingPrice; }
		const enumerations::UnderlyingCode& getUnderlyingCode() const { return m_underlyingCode; }
		const double& getDividendTime() const { return m_dividendTime; }
		const double& getDividendAmount() const { return m_dividendAmount; }
		const double& getJumpTime() const { return m_jumpTime; }
		const double& getJumpMean() const { return m_jumpMean; }
		const double& getJumpVolatility() const { return m_jumpVolatility; }

		// Setters
		void setCostOfCarry(const double& value) { m_costOfCarry = value; }
		void setDiscountRate(const double& value) { m_discountRate = value; }
		void setUnderlyingCode(const enumerations::UnderlyingCode& value) { m_underlyingCode = value; }
		void setImpliedVolatility(const double& value);
		void setInitialUnderlyingPrice(const double& value);
		void setDividendTime(const double& value);
		void setDividendAmount(const double &value);
		void setJumpTime(const double& value);
		void setJumpMean(const double& value) { m_jumpMean = value; }
		void setJumpVolatility(const double& value);

		// Monte Carlo Pricing functions
		const std::shared_ptr<std::vector<double>> generateMonteCarloSimulations(const int& nPaths, const double& time, int seed = -1);

		// Tree Pricing functions
		const std::shared_ptr<models::Tree> constructTree(const int& nTimeSteps, const double& timeToExpiry, 
			const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation);

		const std::shared_ptr<double> smoothedValueAtTreeNode(const double underlyingPrice,
			const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize);
		const bool supportsVanillaOptionSmoothing(const double& timeStart, const double& timeEnd);

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		BlackScholesSingleNormalJump& operator = (BlackScholesSingleNormalJump const&) = delete;
		BlackScholesSingleNormalJump(BlackScholesSingleNormalJump const&) = delete;

	private:
		double m_costOfCarry;
		double m_discountRate;
		double m_impliedVolatility; // no surface is constructed as the solution to the take home only requires a flat volatility.
		double m_initialUnderlyingPrice;
		enumerations::UnderlyingCode m_underlyingCode;
		double m_dividendTime;
		double m_dividendAmount;
		double m_jumpTime;
		double m_jumpMean;
		double m_jumpVolatility;
		const static bool m_supportsVanillaOptionSmoothing = true;

	};
}

#endif // !__BLACKSCHOLESSINGLENORMALJUMP_H__
