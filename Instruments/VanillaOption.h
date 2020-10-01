#ifndef __VANILLAOPTION_H__
#define __VANILLAOPTION_H__

#include <memory>
#include "../Enumerations/ExerciseType.h"
#include "../Enumerations/OptionRight.h"
#include "../Enumerations/UnderlyingCode.h"

namespace instruments
{
	class VanillaOption
	{
	public:
		VanillaOption(const double& strike, const double& timeToExpiry, const enumerations::ExerciseType& exerciseType, 
			const enumerations::OptionRight& optionRight, const enumerations::UnderlyingCode& underlyingCode);
		VanillaOption() = default;
		~VanillaOption() = default;

		// Getters
		const double& getStrike() const { return m_strike; }
		const double& getTimeToExpiry() const { return m_timeToExpiry; }
		const enumerations::ExerciseType& getExerciseType() const { return m_exerciseType; }
		const enumerations::OptionRight& getOptionRight() const { return m_optionRight; }
		const enumerations::UnderlyingCode& getUnderlyingCode() const { return m_underlyingCode; }

		// Setters
		void setStrike(const double& value);
		void setTimeToExpiry(const double& value);
		void setExerciseType(const enumerations::ExerciseType& value) { m_exerciseType = value; }
		void setOptionRight(const enumerations::OptionRight& value) { m_optionRight = value; }
		void setUnderlyingCode(const enumerations::UnderlyingCode& value) { m_underlyingCode = value; }

		// Delete the = operator and the copy constructor to ensure that copies are not inadvertently made
		VanillaOption& operator = (VanillaOption const&) = delete;
		VanillaOption(VanillaOption const&) = delete;

		// Other member functions
		const std::shared_ptr<double> valueAtTreeNode(const double& forwardValue, const double& underlyingPrice) const;
		const std::shared_ptr<double> intrinsicValue (const double& underlyingPrice) const;

	private:
		double m_strike;
		double m_timeToExpiry;
		enumerations::ExerciseType m_exerciseType;
		enumerations::OptionRight m_optionRight;
		enumerations::UnderlyingCode m_underlyingCode;
	};
}

#endif // !__VANILLAOPTION_H__
