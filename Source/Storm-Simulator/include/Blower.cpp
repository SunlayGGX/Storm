#include "BlowerData.h"

#include "BlowerTimeHandler.h"
#include "BlowerEffectArea.h"


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

Storm::BlowerEffectAreaBase::BlowerEffectAreaBase(const Storm::BlowerData &blowerDataConfig) :
	_dimension{ blowerDataConfig._blowerDimension }
{

}

Storm::BlowerCubeArea::BlowerCubeArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerEffectAreaBase::BlowerEffectAreaBase{ blowerDataConfig }
{

}

Storm::BlowerSphereArea::BlowerSphereArea(const Storm::BlowerData &blowerDataConfig) :
	Storm::BlowerEffectAreaBase::BlowerEffectAreaBase{ blowerDataConfig }
{
	_radiusSquared = _dimension.x() * _dimension.x();
}
