#include "ColorChecker.h"


bool Storm::ColorCheckerHelper::isInvalid(const Storm::Vector3 &color3) noexcept
{
	return 
		Storm::ColorCheckerHelper::channelIsInvalid(color3.x()) ||
		Storm::ColorCheckerHelper::channelIsInvalid(color3.y()) ||
		Storm::ColorCheckerHelper::channelIsInvalid(color3.z())
		;
}

bool Storm::ColorCheckerHelper::isInvalid(const Storm::Vector4 &color4) noexcept
{
	return
		Storm::ColorCheckerHelper::channelIsInvalid(color4.x()) ||
		Storm::ColorCheckerHelper::channelIsInvalid(color4.y()) ||
		Storm::ColorCheckerHelper::channelIsInvalid(color4.z()) ||
		Storm::ColorCheckerHelper::channelIsInvalid(color4.w())
		;
}
