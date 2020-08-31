#include "BlowerData.h"

#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"

#include "BlowerType.h"

#include "ThrowException.h"


namespace
{
	inline bool computeFadeInCoefficient(const float currentTime, const float startFadeInTime, const float fadeInDurationTime, float &outFadeCoefficient)
	{
		outFadeCoefficient = (currentTime - startFadeInTime) / fadeInDurationTime;
		return outFadeCoefficient >= 0.f && outFadeCoefficient < 1.f;
	}

	inline bool computeFadeOutCoefficient(const float currentTime, const float startFadeOutTime, const float fadeInDurationTime, float &outFadeCoefficient)
	{
		outFadeCoefficient = 1.f - ((currentTime - startFadeOutTime) / fadeInDurationTime);
		return outFadeCoefficient >= 0.f && outFadeCoefficient < 1.f;
	}
}

#define STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(BlowerDataVariable, Setting)									\
if (BlowerDataVariable._blowerType != Storm::Setting)															\
	Storm::throwException<std::exception>(__FUNCTION__ " is intended to be used for " #Setting " blowers!")		\


Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase(const Storm::BlowerData &blowerDataConfig) :
	_startTime{ blowerDataConfig._startTimeInSeconds },
	_stopTime{ blowerDataConfig._stopTimeInSeconds },
	_currentTime{ 0.f }
{

}

bool Storm::BlowerTimeHandlerBase::advanceTime(const float deltaTimeInSeconds)
{
	_currentTime += deltaTimeInSeconds;
	return _currentTime >= _startTime && (_stopTime == -1.f || _currentTime < _stopTime);
}

Storm::FadeInTimeHandler::FadeInTimeHandler(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerDataConfig },
	Storm::FadeInTimeHandler::UnderlyingFadeInType{ blowerDataConfig._startTimeInSeconds, blowerDataConfig._fadeInTimeInSeconds }
{

}

Storm::FadeOutTimeHandler::FadeOutTimeHandler(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerDataConfig },
	Storm::FadeOutTimeHandler::UnderlyingFadeOutType{ blowerDataConfig._stopTimeInSeconds - blowerDataConfig._fadeOutTimeInSeconds, blowerDataConfig._fadeOutTimeInSeconds }
{

}

Storm::FadeInOutTimeHandler::FadeInOutTimeHandler(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerDataConfig },
	Storm::FadeInOutTimeHandler::UnderlyingFadeInType{ blowerDataConfig._startTimeInSeconds, blowerDataConfig._fadeInTimeInSeconds },
	Storm::FadeInOutTimeHandler::UnderlyingFadeOutType{ blowerDataConfig._stopTimeInSeconds - blowerDataConfig._fadeOutTimeInSeconds, blowerDataConfig._fadeOutTimeInSeconds }
{

}

bool Storm::FadeInTimeHandler::shouldFadeIn(float &outFadeCoefficient) const
{
	using ThisType = Storm::FadeInTimeHandler;
	return computeFadeInCoefficient(_currentTime, ThisType::UnderlyingFadeInType::_startFadeTimeInSeconds, ThisType::UnderlyingFadeInType::_fadeDurationInSeconds, outFadeCoefficient);
}

bool Storm::FadeOutTimeHandler::shouldFadeOut(float &outFadeCoefficient) const
{
	using ThisType = Storm::FadeOutTimeHandler;
	return computeFadeOutCoefficient(_currentTime, ThisType::UnderlyingFadeOutType::_startFadeTimeInSeconds, ThisType::UnderlyingFadeOutType::_fadeDurationInSeconds, outFadeCoefficient);
}

bool Storm::FadeInOutTimeHandler::shouldFadeIn(float &outFadeCoefficient) const
{
	using ThisType = Storm::FadeInOutTimeHandler;
	return computeFadeInCoefficient(_currentTime, ThisType::UnderlyingFadeInType::_startFadeTimeInSeconds, ThisType::UnderlyingFadeInType::_fadeDurationInSeconds, outFadeCoefficient);
}

bool Storm::FadeInOutTimeHandler::shouldFadeOut(float &outFadeCoefficient) const
{
	using ThisType = Storm::FadeInOutTimeHandler;
	return computeFadeOutCoefficient(_currentTime, ThisType::UnderlyingFadeOutType::_startFadeTimeInSeconds, ThisType::UnderlyingFadeOutType::_fadeDurationInSeconds, outFadeCoefficient);
}

void Storm::BlowerRepulsionSphereArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const
{
	const float distNorm = tmp.squaredNorm();
	if (distNorm > 0.0000001f)
	{
		tmp *= (forceNorm / std::sqrtf(distNorm));
	}
	else
	{
		tmp = force;
	}
}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::BlowerData &blowerDataConfig) :
	_dimension{ blowerDataConfig._blowerDimension / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, BlowerType::Cube);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerSphereArea{ blowerDataConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, BlowerType::Sphere);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::BlowerData &blowerDataConfig, int)
{
	_radiusSquared = blowerDataConfig._radius * blowerDataConfig._radius;
}

Storm::BlowerRepulsionSphereArea::BlowerRepulsionSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerSphereArea{ blowerDataConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, BlowerType::RepulsionSphere);
}
