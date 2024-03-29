#include "SpeedProfileHandler.h"

#include "UIField.h"


Storm::SpeedProfileHandler::SpeedProfileHandler(const std::wstring_view &name) :
	_name{ name },
	_pushStep{ 0 },
	_allTimesAccumulated{ 0.f }
{
	_profileDataField
		.bindFieldW(_name, _speedProfile.computationSpeed())
		;
}

Storm::SpeedProfileHandler::~SpeedProfileHandler() = default;

void Storm::SpeedProfileHandler::addProfileData(STORM_MAY_BE_UNUSED const std::wstring_view &name)
{
	assert(_name == name && "Speed profiling is specific to the Simulator loop and only one should exist!");
	_speedProfile.startTime();
}

void Storm::SpeedProfileHandler::removeProfileData(STORM_MAY_BE_UNUSED const std::wstring_view &name)
{
	_speedProfile.stopTime();

	_allTimesAccumulated += this->getCurrentSpeed();

	assert(_name == name && "Speed profiling is specific to the Simulator loop and only one should exist!");

	if (++_pushStep % 10 == 0)
	{
		_profileDataField.pushFieldW(_name);
	}
}

float Storm::SpeedProfileHandler::getAccumulatedTime() const
{
	return _allTimesAccumulated;
}

float Storm::SpeedProfileHandler::getCurrentSpeed() const
{
	return _speedProfile.computationSpeed();
}
