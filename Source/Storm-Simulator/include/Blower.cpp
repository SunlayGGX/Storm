#include "SceneBlowerConfig.h"

#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"

#include "BlowerType.h"

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
	Storm::throwException<Storm::Exception>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__ " blowers!")		\


Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase(const Storm::SceneBlowerConfig &blowerConfig) :
	_startTime{ blowerConfig._startTimeInSeconds },
	_stopTime{ blowerConfig._stopTimeInSeconds },
	_currentTime{ 0.f }
{

}

Storm::BlowerPulseTimeHandler::BlowerPulseTimeHandler(const Storm::SceneBlowerConfig &blowerConfig) :
	_startTime{ blowerConfig._startTimeInSeconds },
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

Storm::FadeInTimeHandler::FadeInTimeHandler(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerConfig },
	Storm::FadeInTimeHandler::UnderlyingFadeInType{ blowerConfig._startTimeInSeconds, blowerConfig._fadeInTimeInSeconds }
{

}

Storm::FadeOutTimeHandler::FadeOutTimeHandler(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerConfig },
	Storm::FadeOutTimeHandler::UnderlyingFadeOutType{ blowerConfig._stopTimeInSeconds - blowerConfig._fadeOutTimeInSeconds, blowerConfig._fadeOutTimeInSeconds }
{

}

Storm::FadeInOutTimeHandler::FadeInOutTimeHandler(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerTimeHandlerBase::BlowerTimeHandlerBase{ blowerConfig },
	Storm::FadeInOutTimeHandler::UnderlyingFadeInType{ blowerConfig._startTimeInSeconds, blowerConfig._fadeInTimeInSeconds },
	Storm::FadeInOutTimeHandler::UnderlyingFadeOutType{ blowerConfig._stopTimeInSeconds - blowerConfig._fadeOutTimeInSeconds, blowerConfig._fadeOutTimeInSeconds }
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


void Storm::BlowerGradualDirectionalCubeArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float forceNorm, Storm::Vector3 &tmp) const
{
	// This distanceCoeff variable should be 1.f if the particle is on the center plane. 0.f if it is on the max distance from the center plane.
	const float distanceCoeff = (1.f - std::fabs(_planeDirectionVect.dot(tmp)) / _maxDistanceToCenterPlane);
	tmp *= (forceNorm / tmp.norm() * distanceCoeff);
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

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig, int) :
	_dimension{ blowerConfig._blowerDimension / 2.f }
{
}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCubeArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cube);
}

Storm::BlowerGradualDirectionalCubeArea::BlowerGradualDirectionalCubeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCubeArea{ blowerConfig, 0 },
	_planeDirectionVect{ blowerConfig._blowerForce.normalized() } // For now, blowers don't have rotation. But when it would have one, use the rotation instead.
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::CubeGradualDirectional);

	// This line is like : what is the farthest corner of the cube from the center plane ?
	// And the farthest corner is the one aligned with the direction, therefore with a dot product positive on each element
	// (since this is a cube, there is always one valid corner which respect all of those assumptions).
	_maxDistanceToCenterPlane =
		std::fabs(_planeDirectionVect.x() * _dimension.x()) +
		std::fabs(_planeDirectionVect.y() * _dimension.y()) +
		std::fabs(_planeDirectionVect.z() * _dimension.z());
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Sphere);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::SceneBlowerConfig &blowerConfig, int)
{
	_radiusSquared = blowerConfig._radius * blowerConfig._radius;
}

Storm::BlowerRepulsionSphereArea::BlowerRepulsionSphereArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::RepulsionSphere);
}

Storm::BlowerExplosionSphereArea::BlowerExplosionSphereArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 },
	_radius{ blowerConfig._radius }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere);
}

Storm::BlowerCylinderArea::BlowerCylinderArea(const Storm::SceneBlowerConfig &blowerConfig) :
	_radiusSquared{ blowerConfig._radius * blowerConfig._radius },
	_midHeight{ blowerConfig._height / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cylinder);
}

Storm::BlowerConeArea::BlowerConeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	_downRadiusSquared{ blowerConfig._downRadius * blowerConfig._downRadius },
	_midHeight{ blowerConfig._height / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_RIGHT_SETTING(blowerConfig, Storm::BlowerType::Cone);

	const float upRadiusSquared = blowerConfig._upRadius * blowerConfig._upRadius;
	_diffRadiusSquared = upRadiusSquared - _downRadiusSquared;
}

Storm::BlowerCubeArea::~BlowerCubeArea() = default;
Storm::BlowerGradualDirectionalCubeArea::~BlowerGradualDirectionalCubeArea() = default;
Storm::BlowerSphereArea::~BlowerSphereArea() = default;
Storm::BlowerRepulsionSphereArea::~BlowerRepulsionSphereArea() = default;
Storm::BlowerExplosionSphereArea::~BlowerExplosionSphereArea() = default;
Storm::BlowerCylinderArea::~BlowerCylinderArea() = default;
Storm::BlowerConeArea::~BlowerConeArea() = default;
