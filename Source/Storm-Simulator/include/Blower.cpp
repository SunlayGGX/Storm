#include "SceneBlowerConfig.h"

#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"
#include "BlowerVorticeArea.h"

#include "BlowerType.h"

#include "CorrectSettingChecker.h"

#include "StaticAssertionsMacros.h"
#include "StormMacro.h"


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

#define STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(BlowerDataVariable, ...)											\
if (!CorrectSettingChecker<Storm::BlowerType>::check<__VA_ARGS__>(BlowerDataVariable._blowerType))						\
	Storm::throwException<Storm::Exception>(__FUNCTION__ " is intended to be used for " #__VA_ARGS__ " blowers!")		\

#define STORM_CHECK_CORRECT_INHERITANCE_CONSTRUCT(ParentBlowerType)																						\
using ThisType = std::remove_reference_t<decltype(*this)>;																								\
STORM_STATIC_ASSERT(STORM_MAKE_PACKED_PARAMETER(std::is_base_of_v<ParentBlowerType, ThisType> && !std::is_convertible_v<ThisType, ParentBlowerType>),	\
"The current BlowerType should inherit privately from " STORM_STRINGIFY(ParentBlowerType))


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
	return this->forceSetTime(_currentTime + deltaTimeInSeconds);
}

bool Storm::BlowerPulseTimeHandler::forceSetTime(const float timeInSeconds)
{
	_currentTime = timeInSeconds;
	if (_currentTime >= _startTime)
	{
		_enabled = false;
		return true;
	}
	else STORM_LIKELY
	{
		_enabled = true;
		return false;
	}
}

bool Storm::BlowerTimeHandlerBase::forceSetTime(const float timeInSeconds)
{
	_currentTime = timeInSeconds;
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


void Storm::BlowerGradualDirectionalCubeArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float /*forceNorm*/, Storm::Vector3 &tmp) const
{
	// This distanceCoeff variable should be 1.f if the particle is on the center plane. 0.f if it is on the max distance from the center plane.
	const float distanceCoeff = (1.f - std::fabs(_planeDirectionVect.dot(tmp)) / _maxDistanceToCenterPlane);
	tmp = force * distanceCoeff;
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

void Storm::BlowerSpherePlanarGradualArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float /*forceNorm*/, Storm::Vector3 &tmp) const
{
	const float distanceCoeff = 1.f - std::fabs(_planeDirectionVect.dot(tmp) / _radius);
	tmp = force * distanceCoeff;
}

void Storm::BlowerCylinderGradualMidPlanarArea::applyDistanceEffectToTemporary(const Storm::Vector3 &force, const float /*forceNorm*/, Storm::Vector3 &tmp) const
{
	const float distanceCoeff = 1.f - std::fabs(tmp.y()) / _midHeight;
	tmp = force * distanceCoeff;
}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig, int) :
	_dimension{ blowerConfig._blowerDimension / 2.f }
{
}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCubeArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::Cube);
}

Storm::BlowerGradualDirectionalCubeArea::BlowerGradualDirectionalCubeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCubeArea{ blowerConfig, 0 },
	_planeDirectionVect{ blowerConfig._blowerForce.normalized() } // For now, blowers don't have rotation. But when it would have one, use the rotation instead.
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::CubeGradualDirectional);
	STORM_CHECK_CORRECT_INHERITANCE_CONSTRUCT(Storm::BlowerCubeArea);

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
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::Sphere);
}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::SceneBlowerConfig &blowerConfig, int)
{
	_radiusSquared = blowerConfig._radius * blowerConfig._radius;
}

Storm::BlowerRepulsionSphereArea::BlowerRepulsionSphereArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::RepulsionSphere);
}

Storm::BlowerExplosionSphereArea::BlowerExplosionSphereArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 },
	_radius{ blowerConfig._radius }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::ExplosionSphere, Storm::BlowerType::PulseExplosionSphere);
	STORM_CHECK_CORRECT_INHERITANCE_CONSTRUCT(Storm::BlowerSphereArea);
}

Storm::BlowerSpherePlanarGradualArea::BlowerSpherePlanarGradualArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerSphereArea{ blowerConfig, 0 },
	_planeDirectionVect{ blowerConfig._blowerForce.normalized() },
	_radius{ blowerConfig._radius }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::SpherePlanarGradual);
	STORM_CHECK_CORRECT_INHERITANCE_CONSTRUCT(Storm::BlowerSphereArea);
}

Storm::BlowerCylinderArea::BlowerCylinderArea(const Storm::SceneBlowerConfig &blowerConfig, int) :
	_radiusSquared{ blowerConfig._radius * blowerConfig._radius },
	_midHeight{ blowerConfig._height / 2.f }
{

}

Storm::BlowerCylinderArea::BlowerCylinderArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCylinderArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::Cylinder);
}

Storm::BlowerCylinderGradualMidPlanarArea::BlowerCylinderGradualMidPlanarArea(const Storm::SceneBlowerConfig &blowerConfig) :
	Storm::BlowerCylinderArea{ blowerConfig, 0 }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::CylinderGradualMidPlanar);
	STORM_CHECK_CORRECT_INHERITANCE_CONSTRUCT(Storm::BlowerCylinderArea);
}

Storm::BlowerConeArea::BlowerConeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	_downRadiusSquared{ blowerConfig._downRadius * blowerConfig._downRadius },
	_midHeight{ blowerConfig._height / 2.f }
{
	STORM_ENSURE_CONSTRUCTED_ON_CORRECT_SETTINGS(blowerConfig, Storm::BlowerType::Cone);

	const float upRadiusSquared = blowerConfig._upRadius * blowerConfig._upRadius;
	_diffRadiusSquared = upRadiusSquared - _downRadiusSquared;
}

Storm::NoVortice::NoVortice(const Storm::SceneBlowerConfig &) {}

Storm::DefaultVorticeArea::DefaultVorticeArea(const Storm::SceneBlowerConfig &blowerConfig) :
	_coeff{ blowerConfig._vorticeCoeff }
{

}

Storm::Vector3 Storm::DefaultVorticeArea::applyVortice(const Storm::Vector3 &force, const float forceNormSquared, const Storm::Vector3 &posDiff) const
{
	return (posDiff - posDiff.dot(force) / forceNormSquared * force).cross(force);
}
