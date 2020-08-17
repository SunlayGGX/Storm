#include "SpeedProfileHandler.h"

#include "UIField.h"


Storm::SpeedProfileHandler::SpeedProfileHandler(const std::wstring_view &name) :
	_name{ name },
	_pushStep{ 0 }
{
	_profileDataField
		.bindFieldW(_name, _speedProfile.computationSpeed())
		;
}

Storm::SpeedProfileHandler::~SpeedProfileHandler() = default;

void Storm::SpeedProfileHandler::addProfileData(const std::wstring_view &name)
{
	assert(_name == name && "Speed profiling is specific to the Simulator loop and only one should exist!");
	_speedProfile.startTime();
}

void Storm::SpeedProfileHandler::removeProfileData(const std::wstring_view &name)
{
	_speedProfile.stopTime();

	assert(_name == name && "Speed profiling is specific to the Simulator loop and only one should exist!");

	if (++_pushStep % 10 == 0)
	{
		_profileDataField.pushFieldW(_name);
	}
}
