#include "BlowerData.h"

#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"

#include "BlowerType.h"

#include "ThrowException.h"
#include "CorrectSettingChecker.h"


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

#define STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(BlowerDataVariable, ...)											\
if (!CorrectSettingChecker<Storm::BlowerType>::check<__VA_ARGS__>(BlowerDataVariable._blowerType))					\
	Storm::throwException<std::exception>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__ " blowers!")		\


Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase(const Storm::BlowerData &blowerDataConfig) :
	_startTime{ blowerDataConfig._startTimeInSeconds },
	_stopTime{ blowerDataConfig._stopTimeInSeconds },
	_currentTime{ 0.f }
{

}

Storm::BlowerPulseTimeHandler::BlowerPulseTimeHandler(const Storm::BlowerData &blowerDataConfig) :
	_startTime{ blowerDataConfig._startTimeInSeconds },
	_currentTime{ 0.f },
	_enabled{ true }
{

}

bool Storm::BlowerPulseTimeHandler::advanceTime(const float deltaTimeInSeconds)
{
	if (_enabled)
	{
		_currentTime += deltaTimeInSeconds;
		if (_currentTime >= _startTime)
		{
			_enabled = false;
			return true;
		}
	}

	return false;
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
	const float distNormSquared = tmp.squaredNorm();
	if (distNormSquared > 0.0000001f)
	{
		tmp *= (forceNorm / std::sqrtf(distNormSquared));
	}
	else
	{
		tmp = force;
	}
}

void Storm::BlowerExplosionSphereArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const
{
	const float distNormSquared = tmp.squaredNorm();
	if (distNormSquared > 0.0000001f)
	{
		tmp *= (_radius * forceNorm / distNormSquared);
	}
	else
	{
		tmp = force;
	}
}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::BlowerData &blowerDataConfig) :
	_dimension{ blowerDataConfig._blowerDimension / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::Cube);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerSphereArea{ blowerDataConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::Sphere);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::BlowerData &blowerDataConfig, int)
{
	_radiusSquared = blowerDataConfig._radius * blowerDataConfig._radius;
}

Storm::BlowerRepulsionSphereArea::BlowerRepulsionSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerSphereArea{ blowerDataConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::RepulsionSphere);
}

Storm::BlowerExplosionSphereArea::BlowerExplosionSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerSphereArea{ blowerDataConfig, 0 },
	_radius{ blowerDataConfig._radius }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere);
}

Storm::BlowerCylinderArea::BlowerCylinderArea(const Storm::BlowerData &blowerDataConfig) :
	_radiusSquared{ blowerDataConfig._radius * blowerDataConfig._radius },
	_height{ blowerDataConfig._height }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::Cylinder);
}

Storm::BlowerConeArea::BlowerConeArea(const Storm::BlowerData &blowerDataConfig) :
	_downRadiusSquared{ blowerDataConfig._downRadius * blowerDataConfig._downRadius },
	_midHeight{ blowerDataConfig._height / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerDataConfig, Storm::BlowerType::Cone);

	const float upRadiusSquared = blowerDataConfig._upRadius * blowerDataConfig._upRadius;
	_diffRadiusSquared = upRadiusSquared - _downRadiusSquared;
}
