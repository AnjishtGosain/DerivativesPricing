#ifndef __BLACKSCHOLESDOUBLENORMALJUMP_H__ 
#define __BLACKSCHOLESDOUBLENORMALJUMP_H__

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
	class BlackScholesDoubleNormalJump : public IMonteCarloModel, public ITreeModel
	{
	public:
		BlackScholesDoubleNormalJump(const double& costOfCarry, const double& discountRate, const double& impliedVolatility, 
			const double& initialUnderlyingPrice, const enumerations::UnderlyingCode& underlyingCode, const double& dividendTime, 
			const double& dividendAmount, const double& jumpTime, const double& jumpMean1, const double& jumpVolatility1, 
			const double& jumpMean2, const double& jumpVolatility2, const double& bernoulliProbability);
		BlackScholesDoubleNormalJump() = default;
		~BlackScholesDoubleNormalJump() = default;

		// Getters
		const double& getCostOfCarry() const { return m_costOfCarry; }
		const double& getDiscountRate() const { return m_discountRate; }
		const double& getImpliedVolatility() const { return m_impliedVolatility; }
		const double& getInitialUnderlyingPrice() const { return m_initialUnderlyingPrice; }
		const enumerations::UnderlyingCode& getUnderlyingCode() const { return m_underlyingCode; }
		const double& getDividendTime() const { return m_dividendTime; }
		const double& getDividendAmount() const { return m_dividendAmount; }
		const double& getJumpTime() const { return m_jumpTime; }
		const double& getJumpMean1() const { return m_jumpMean1; }
		const double& getJumpVolatility1() const { return m_jumpVolatility1; }
		const double& getJumpMean2() const { return m_jumpMean2; }
		const double& getJumpVolatility2() const { return m_jumpVolatility2; }
		const double& getBernoulliProbability() const { return m_bernoulliProbability; }

		// Setters
		void setCostOfCarry(const double& value) { m_costOfCarry = value; }
		void setDiscountRate(const double& value) { m_discountRate = value; }
		void setUnderlyingCode(const enumerations::UnderlyingCode& value) { m_underlyingCode = value; }
		void setImpliedVolatility(const double& value);
		void setInitialUnderlyingPrice(const double& value);
		void setDividendTime(const double& value);
		void setDividendAmount(const double &value);
		void setJumpTime(const double& value);
		void setJumpMean1(const double& value) { m_jumpMean1 = value; }
		void setJumpVolatility1(const double& value);
		void setJumpMean2(const double& value) { m_jumpMean2 = value; }
		void setJumpVolatility2(const double& value);
		void setBernoulliProbability(const double& value);

		// Monte Carlo Pricing functions
		const std::shared_ptr<std::vector<double>> generateMonteCarloSimulations(const int& nPaths, const double& time, int seed = -1);

		// Tree Pricing functions
		const std::shared_ptr<models::Tree> constructTree(const int& nTimeSteps, const double& timeToExpiry, 
			const enumerations::Implementation implementation, const double upperLimitStandardDeviation, const double lowerLimitStandardDeviation);

		const std::shared_ptr<double> smoothedValueAtTreeNode(const double underlyingPrice,
			const shared_ptr<instruments::VanillaOption> vanillaOption, const double timeStepSize);
		const bool supportsVanillaOptionSmoothing(const double& timeStart, const double& timeEnd);

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		BlackScholesDoubleNormalJump& operator = (BlackScholesDoubleNormalJump const&) = delete;
		BlackScholesDoubleNormalJump(BlackScholesDoubleNormalJump const&) = delete;

	private:
		double m_costOfCarry;
		double m_discountRate;
		double m_impliedVolatility; // no surface is constructed as the solution to the take home only requires a flat volatility.
		double m_initialUnderlyingPrice;
		enumerations::UnderlyingCode m_underlyingCode;
		double m_dividendTime;
		double m_dividendAmount;
		double m_jumpTime;
		double m_jumpMean1;
		double m_jumpVolatility1;
		double m_jumpMean2;
		double m_jumpVolatility2;
		double m_bernoulliProbability;
		const static bool m_supportsVanillaOptionSmoothing = true;

	};
}

#endif // !__BLACKSCHOLESDOUBLENORMALJUMP_H__
