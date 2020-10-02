#pragma once

#include "SpeedProfileData.h"
#include "UIFieldContainer.h"


namespace Storm
{
	class SpeedProfileHandler
	{
	public:
		SpeedProfileHandler(const std::wstring_view &name);
		~SpeedProfileHandler();

	public:
		void addProfileData(const std::wstring_view &name);
		void removeProfileData(const std::wstring_view &name);

		float getAccumulatedTime() const;

	private:
		const std::wstring_view _name;
		Storm::SpeedProfileData _speedProfile;

		float _allTimesAccumulated;

		unsigned int _pushStep;
		Storm::UIFieldContainer _profileDataField;
	};
}
