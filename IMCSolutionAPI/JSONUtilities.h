#ifndef __JSONUTILITIES_H__
#define __JSONUTILITIES_H__

#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../Instruments/VanillaOption.h"
#include "../Models/BlackScholesDoubleNormalJump.h"

std::shared_ptr<std::unordered_map<int, 
	std::tuple<
	std::shared_ptr<models::BlackScholesDoubleNormalJump>, 
	std::shared_ptr<instruments::VanillaOption> 
	>>> VanillaOptionBlackScholesDoubleNormalJSONReader(const std::string& jsonFileLocation);

void VanillaOptionBlackScholesDoubleNormalJSONWriter(
	const std::shared_ptr<std::unordered_map<int, std::shared_ptr<double>>> prices,
	const std::shared_ptr<std::unordered_map<int,
	std::tuple<
	std::shared_ptr<models::BlackScholesDoubleNormalJump>,
	std::shared_ptr<instruments::VanillaOption>>>> options,
	const std::string& jsonFileLocation);

std::tuple<std::shared_ptr<std::vector<std::shared_ptr<double>>>, std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>>> 
	AmericanOptionJSONReader
	(const std::string& jsonFileLocation);

void AmericanOptionJSONWriter(const std::shared_ptr<std::vector<std::shared_ptr<double>>> prices,
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> options, const std::string& jsonFileLocation);

#endif // !__JSONUTILITIES_H__
