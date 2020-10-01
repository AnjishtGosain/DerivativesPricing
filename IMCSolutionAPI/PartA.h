#ifndef __PARTA_H__
#define __PARTA_H__

#include <vector>
#include <memory>
#include "../Instruments/VanillaOption.h"
#include "../Models/TreeModelUtilities/ITreeModel.h" 


class PartA
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

private:
	PartA() {};

	static std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> constructVanillaOptionsPtr(const double & strike, 
		const bool & is_call, const double & T);

	static double constructTreePrice(const std::shared_ptr<models::ITreeModel>& model, const double& timeStepSize,
		const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>& vanillaOptionsPtr);
};

#endif // !__PARTA_H__
