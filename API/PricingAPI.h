#ifndef __PRICINGAPI_H__
#define __PRICINGAPI_H__

#include <vector>
#include <memory>
#include "JSONUtilities.h"
#include "../Instruments/VanillaOption.h"
#include "../Models/TreeModelUtilities/ITreeModel.h" 


class PricingAPI
{
public:
	static double price(const double& strike, const bool& is_call, const double& s_0, const double& r, const double& q, const double& T, 
		const double& sigma, const double& timeStepSize);

	static double price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T,
		const double & sigma, const double& t_div, const double& div, const double& timeStepSize);

	static double price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T,
		const double & sigma, const double& t_div, const double& div, const double& jumpTime, const double& jumpMean, 
		const double& jumpVol, const double& timeStepSize);

	static double price(const double & strike, const bool & is_call, const double & s_0, const double & r, const double & q, const double & T,
		const double & sigma, const double& t_div, const double& div, const double& jumpTime, const double& jumpMean1, const double& jumpVol1,
		const double& jumpMean2, const double& jumpVol2, const double& bernoulliProbability, const double& timeStepSize);

	// Functions for Pricing Problem 1

	static void priceVanillaOptionsBlackScholesDoubleNormalJumpModel(const std::string & jsonInputFilePath,
		const std::string & jsonOutputFilePath, const double & initialUnderlyingPrice, const double & jumpMean, const int & nTimeSteps);

	static void priceAmericanOptionsBlackScholesSingleNormalJumpModel(const std::string& jsonInputFilePath, const std::string& jsonOutputFilePath,
		const double& costOfCarry, const double& discountRate, const double& impliedVolatility, const double& initialUnderlyingPrice, 
		const double& dividendTime, const double& dividendAmount, const double& jumpTime, const double& jumpMean, const double& jumpVolatility,
		const int& nTimeSteps);



private:
	PricingAPI() {};

	static std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> constructVanillaOptionsPtr(const double & strike, 
		const bool & is_call, const double & T);

	static double constructTreePrice(const std::shared_ptr<models::ITreeModel>& model, const double& timeStepSize,
		const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptionsPtr);
};

#endif // !__PRICINGAPI_H__
