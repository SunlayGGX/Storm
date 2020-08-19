#pragma once


namespace Storm
{
	struct BlowerData;

	class BlowerTimeHandlerBase
	{
	protected:
		BlowerTimeHandlerBase(const Storm::BlowerData &blowerDataConfig);

	public:
		bool advanceTime(const float deltaTimeInSeconds);

	protected:
		float _currentTime;
		const float _startTime;
		const float _stopTime;
	};

	template<int>
	class FadeTimeHandler
	{
	protected:
		FadeTimeHandler(float startFadeTime, float fadeDuration) :
			_startFadeTimeInSeconds{ startFadeTime },
			_fadeDurationInSeconds{ fadeDuration }
		{}

	protected:
		bool isInsideTimespan(float &fadeCoefficient, const float currentTimeInSeconds) const
		{
			fadeCoefficient = (currentTimeInSeconds - _startFadeTimeInSeconds) / _fadeDurationInSeconds;
			return fadeCoefficient >= 0.f && fadeCoefficient < 1.f;
		}

	protected:
		const float _startFadeTimeInSeconds;
		const float _fadeDurationInSeconds;
	};

	class FadeInTimeHandler :
		protected Storm::BlowerTimeHandlerBase,
		protected Storm::FadeTimeHandler<0>
	{
	protected:
		using UnderlyingFadeInType = Storm::FadeTimeHandler<0>;

	public:
		FadeInTimeHandler(const Storm::BlowerData &blowerDataConfig);

	protected:
		constexpr static bool hasFadeIn() { return true; }

	protected:
		bool shouldFadeIn(float &outFadeCoefficient) const;
	};

	class FadeOutTimeHandler :
		protected Storm::BlowerTimeHandlerBase,
		protected Storm::FadeTimeHandler<0>
	{
	protected:
		using UnderlyingFadeOutType = Storm::FadeTimeHandler<0>;

	public:
		FadeOutTimeHandler(const Storm::BlowerData &blowerDataConfig);

	protected:
		constexpr static bool hasFadeOut() { return true; }

	public:
		bool shouldFadeOut(float &outFadeCoefficient) const;
	};

	class FadeInOutTimeHandler :
		protected Storm::BlowerTimeHandlerBase,
		protected Storm::FadeTimeHandler<0>,
		protected Storm::FadeTimeHandler<1>
	{
	protected:
		using UnderlyingFadeInType = Storm::FadeTimeHandler<0>;
		using UnderlyingFadeOutType = Storm::FadeTimeHandler<1>;

	public:
		FadeInOutTimeHandler(const Storm::BlowerData &blowerDataConfig);

	protected:
		constexpr static bool hasFadeIn() { return true; }
		constexpr static bool hasFadeOut() { return true; }

	public:
		bool shouldFadeIn(float &outFadeCoefficient) const;
		bool shouldFadeOut(float &outFadeCoefficient) const;
	};
}
