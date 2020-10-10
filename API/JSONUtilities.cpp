#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include "../Enumerations/OptionRight.h"
#include "JSONUtilities.h"

namespace pt = boost::property_tree;
using namespace std;
using namespace instruments;
using namespace enumerations;
using namespace models;
using boost::lexical_cast;

std::shared_ptr<std::unordered_map<int, 
	std::tuple<
	std::shared_ptr<models::BlackScholesDoubleNormalJump>, 
	std::shared_ptr<instruments::VanillaOption> 
	>>> VanillaOptionBlackScholesDoubleNormalJSONReader(const std::string& jsonFileLocation)
{
	// Read in the American option data from JSON file
	pt::ptree root;
	// Load the json file in this ptree
	pt::read_json(jsonFileLocation, root);

	// Read in the data into unordered maps - in a map, every time a key is added, the map is automarically resorted. 
	// I do this manually in the next step to preserve the indexing of the contracts from the json file

	unordered_map<int, double> unorderedDividendAmount;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("D"))
		unorderedDividendAmount.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, ExerciseType> unorderedExerciseStyle;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("exercise_style"))
		unorderedExerciseStyle.emplace(lexical_cast<int>(string(point.first.data())), 
			string(point.second.data()) == "AMERICAN" ? ExerciseType::american : ExerciseType::european);

	unordered_map<int, double> unorderedNormalShift;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("k"))
		unorderedNormalShift.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, OptionRight> unorderedKind;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("kind"))
		unorderedKind.emplace(lexical_cast<int>(string(point.first.data())), 
			string(point.second.data()) == "PUT" ? OptionRight::put : OptionRight::call);

	unordered_map<int, double> unorderedBernoulliProbability;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("p"))
		unorderedBernoulliProbability.emplace(
			lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedCostOfCarry;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("q"))
		unorderedCostOfCarry.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedDiscountRate;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("r"))
		unorderedDiscountRate.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedImpliedVolatility;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("sigma"))
		unorderedImpliedVolatility.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedJumpVolatility;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("sigma_ea"))
		unorderedJumpVolatility.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedStrike;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("strike"))
		unorderedStrike.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedDividendTime;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("t_div"))
		unorderedDividendTime.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedJumpTime;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("t_ea"))
		unorderedJumpTime.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedTimeToExpiry;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("time_to_expiry"))
		unorderedTimeToExpiry.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	// Construct the vanilla option and model pairs
	unordered_map<int, tuple<shared_ptr<BlackScholesDoubleNormalJump>, shared_ptr<VanillaOption>>> modelOptionPairs;
	auto n = unorderedDividendTime.size();
	modelOptionPairs.reserve(n);
	for (auto it = unorderedTimeToExpiry.begin(); it != unorderedTimeToExpiry.end(); ++it)
	{
		auto key = it->first;
		auto modelPtr = make_shared<BlackScholesDoubleNormalJump>(
			unorderedCostOfCarry[key],
			unorderedDiscountRate[key],
			unorderedImpliedVolatility[key],
			0.0, // the underlying price is currently hard set to 0, overriden later
			UnderlyingCode::BHP,
			unorderedDividendTime[key],
			unorderedDividendAmount[key],
			unorderedJumpTime[key],
			unorderedNormalShift[key], // assuming that \mu = 0.0
			unorderedJumpVolatility[key],
			-1.0 * unorderedNormalShift[key],
			unorderedJumpVolatility[key],
			unorderedBernoulliProbability[key]
			);

		auto optionPtr = make_shared<VanillaOption>(unorderedStrike[key], unorderedTimeToExpiry[key], unorderedExerciseStyle[key], 
			unorderedKind[key], UnderlyingCode::BHP);
		auto modelOptionTuple = make_tuple(move(modelPtr), move(optionPtr));
		// Add the option to options, and the price to optionPrices
		modelOptionPairs.insert({ key, (move(modelOptionTuple)) });
	}
	auto modelOptionPairsPtr = make_shared<unordered_map<int, tuple<shared_ptr<BlackScholesDoubleNormalJump>, 
		shared_ptr<VanillaOption>>>>(move(modelOptionPairs));
	return modelOptionPairsPtr;
}

void VanillaOptionBlackScholesDoubleNormalJSONWriter(
	const std::shared_ptr<std::unordered_map<int, std::shared_ptr<double>>> prices,
	const std::shared_ptr<std::unordered_map<int, 
		std::tuple<
		std::shared_ptr<models::BlackScholesDoubleNormalJump>, 
		std::shared_ptr<instruments::VanillaOption>>>> options,
	const std::string& jsonFileLocation)
{
	// Initialise the object
	pt::ptree root;

	// Add the prices
	pt::ptree dividendAmountNodes;
	pt::ptree exerciseTypeNodes;
	pt::ptree shiftAmountNodes;
	pt::ptree optionRightNodes;
	pt::ptree bernoulliProbabilityNodes;
	pt::ptree costOfCarryNodes;
	pt::ptree discountRateNodes;
	pt::ptree impliedVolatilityNodes;
	pt::ptree jumpVolatilityNodes;
	pt::ptree strikeNodes;
	pt::ptree dividendTimeNodes;
	pt::ptree jumpTimeNodes;
	pt::ptree timeToExpiryNodes;
	pt::ptree priceNodes;
	for (auto it = prices->begin(); it != prices->end(); it++)
	{
		auto key = it->first; 
		auto price = it->second; 
		string keyString(lexical_cast<string>(key));
		dividendAmountNodes.put(keyString, get<0>(options->at(key))->getDividendAmount());
		exerciseTypeNodes.put(keyString, get<1>(options->at(key))->getExerciseType() == ExerciseType::american ? "AMERICAN" : "EUROPEAN");
		shiftAmountNodes.put(keyString, get<0>(options->at(key))->getJumpMean1());
		optionRightNodes.put(keyString, get<1>(options->at(key))->getOptionRight() == OptionRight::call ? "CALL" : "PUT");
		bernoulliProbabilityNodes.put(keyString, get<0>(options->at(key))->getBernoulliProbability());
		costOfCarryNodes.put(keyString, get<0>(options->at(key))->getCostOfCarry());
		discountRateNodes.put(keyString, get<0>(options->at(key))->getDiscountRate());
		impliedVolatilityNodes.put(keyString, get<0>(options->at(key))->getImpliedVolatility());
		jumpVolatilityNodes.put(keyString, get<0>(options->at(key))->getJumpVolatility1());
		strikeNodes.put(keyString, get<1>(options->at(key))->getStrike());
		dividendTimeNodes.put(keyString, get<0>(options->at(key))->getDividendTime());
		jumpTimeNodes.put(keyString, get<0>(options->at(key))->getJumpTime());
		timeToExpiryNodes.put(keyString, get<1>(options->at(key))->getTimeToExpiry());
		priceNodes.put(keyString, *price);
	}
	root.add_child("D", dividendAmountNodes);
	root.add_child("exercise_style", exerciseTypeNodes);
	root.add_child("k", shiftAmountNodes);
	root.add_child("kind", optionRightNodes);
	root.add_child("p", bernoulliProbabilityNodes);
	root.add_child("q", costOfCarryNodes);
	root.add_child("r", discountRateNodes);
	root.add_child("sigma", impliedVolatilityNodes);
	root.add_child("sigma_ea", jumpVolatilityNodes);
	root.add_child("strike", strikeNodes);
	root.add_child("t_div", dividendTimeNodes);
	root.add_child("t_ea", jumpTimeNodes);
	root.add_child("time_to_expiry", timeToExpiryNodes);
	root.add_child("price", priceNodes);

	// Write out the tree
	pt::write_json(jsonFileLocation, root);
	return;
}


std::tuple<
	std::shared_ptr<std::vector<std::shared_ptr<double>>>, // prices
	std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> // option
	> 
	AmericanOptionJSONReader
	(const std::string& jsonFileLocation)
{
	// Read in the American option data from JSON file
	pt::ptree root;
	// Load the json file in this ptree
	pt::read_json(jsonFileLocation, root);

	// Read in the data into unordered maps - in a map, every time a key is added, the map is automarically resorted. 
	// I do this manually in the next step to preserve the indexing of the contracts from the json file
	unordered_map<int, double> unorderedTimeToExpiry;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("time_to_expiry"))
		unorderedTimeToExpiry.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, double> unorderedStrike;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("strike"))
		unorderedStrike.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	unordered_map<int, OptionRight> unorderedKind;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("kind"))
		unorderedKind.emplace(lexical_cast<int>(string(point.first.data())), string(point.second.data()) == "PUT" ? OptionRight::put : OptionRight::call);

	unordered_map<int, double> unorderedPrice;
	BOOST_FOREACH(pt::ptree::value_type &point, root.get_child("price"))
		unorderedPrice.emplace(lexical_cast<int>(string(point.first.data())), lexical_cast<double>(string(point.second.data())));

	// construct ordered maps to preserve indexing from the json file
	map<int, double> timeToExpiry(unorderedTimeToExpiry.begin(), unorderedTimeToExpiry.end());
	map<int, double> strike(unorderedStrike.begin(), unorderedStrike.end());
	map<int, OptionRight> kind(unorderedKind.begin(), unorderedKind.end());
	map<int, double> price(unorderedPrice.begin(), unorderedPrice.end());

	// Construct the American options
	vector<shared_ptr<VanillaOption>> options;
	vector<shared_ptr<double>> optionPrices;
	auto n = timeToExpiry.size();
	options.reserve(n);
	optionPrices.reserve(n);
	for (auto it = timeToExpiry.begin(); it != timeToExpiry.end(); ++it)
	{
		auto key = it->first;
		auto optionPtr = make_shared<VanillaOption>(strike[key], timeToExpiry[key], ExerciseType::american, kind[key], UnderlyingCode::BHP);
		auto optionPrice = price[key];
		// Add the option to options, and the price to optionPrices
		options.push_back(move(optionPtr));
		optionPrices.push_back(make_shared<double>(move(optionPrice)));
	}
	auto optionPricesPtr = make_shared<vector<shared_ptr<double>>>(move(optionPrices));
	auto optionsPtr = make_shared<vector<shared_ptr<VanillaOption>>>(move(options));

	return { optionPricesPtr, optionsPtr };
}

void AmericanOptionJSONWriter(
	const std::shared_ptr<std::vector<std::shared_ptr<double>>> prices,
	const std::shared_ptr<std::vector<std::shared_ptr<instruments::VanillaOption>>> options, 
	const std::string& jsonFileLocation)
{
	// Initialise the object
	pt::ptree root;

	// Add the prices
	pt::ptree priceNodes;
	pt::ptree strikeNodes;
	pt::ptree kindNodes;
	pt::ptree timeToExpiryNodes;
	for(int i = 0; i < prices->size(); i++)
	{
		string val(lexical_cast<string>(i));
		priceNodes.put(val, *prices->at(i));
		strikeNodes.put(val, options->at(i)->getStrike());
		kindNodes.put(val, options->at(i)->getOptionRight() == OptionRight::call ? "CALL" : "PUT");
		timeToExpiryNodes.put(val, options->at(i)->getTimeToExpiry());
	}
	root.add_child("time_to_expiry", timeToExpiryNodes);
	root.add_child("strike", strikeNodes);
	root.add_child("kind", kindNodes);
	root.add_child("price", priceNodes);

	// Write out the tree
	pt::write_json(jsonFileLocation, root);
	return;
}