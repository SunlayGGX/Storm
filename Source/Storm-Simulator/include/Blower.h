#pragma once

#include "IBlower.h"


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType blowerType, class BlowerEffectArea, class BlowerTimeHandler>
	class Blower :
		public Storm::IBlower,
		private BlowerEffectArea,
		private BlowerTimeHandler
	{
	private:
		enum class BlowerState : char
		{
			NotWorking,
			Fading,
			FullyFonctional,
		};

	public:
		Blower(const Storm::BlowerData &blowerDataConfig) :
			BlowerEffectArea{ blowerDataConfig },
			BlowerTimeHandler{ blowerDataConfig },
			_id{ blowerDataConfig._id },
			_srcForce{ blowerDataConfig._blowerForce },
			_force{ Vector3::Zero() },
			_blowerPosition{ blowerDataConfig._blowerPosition },
			_state{ BlowerState::NotWorking }
		{}

	private:
		template<class Type>
		constexpr static auto hasFadeIn(int) -> decltype(Type::hasFadeIn())
		{
			return Type::hasFadeIn();
		}

		template<class Type>
		constexpr static bool hasFadeIn(void*)
		{
			return false;
		}

		template<class Type>
		constexpr static auto hasFadeOut(int) -> decltype(Type::hasFadeOut())
		{
			return Type::hasFadeOut();
		}

		template<class Type>
		constexpr static bool hasFadeOut(void*)
		{
			return false;
		}

		template<class Type>
		constexpr static auto hasDistanceEffect(int) -> decltype(Type::hasDistanceEffect())
		{
			return Type::hasDistanceEffect();
		}

		template<class Type>
		constexpr static bool hasDistanceEffect(void*)
		{
			return false;
		}

	public:
		constexpr Storm::BlowerType getType() const final override
		{
			return blowerType;
		}

		void advanceTime(float deltaTime) final override
		{
			using ThisType = Storm::Blower<blowerType, BlowerEffectArea, BlowerTimeHandler>;

			if (BlowerTimeHandler::advanceTime(deltaTime))
			{
				if constexpr (ThisType::hasFadeIn<BlowerTimeHandler>(0))
				{
					float fadeInCoefficient;
					if (BlowerTimeHandler::shouldFadeIn(fadeInCoefficient))
					{
						_force = _srcForce * fadeInCoefficient;
						_state = BlowerState::Fading;
						return;
					}
				}

				if constexpr (ThisType::hasFadeOut<BlowerTimeHandler>(0))
				{
					float fadeOutCoefficient;
					if (BlowerTimeHandler::shouldFadeOut(fadeOutCoefficient))
					{
						_force = _srcForce * fadeOutCoefficient;
						_state = BlowerState::Fading;
						return;
					}
				}

				if (_state != BlowerState::FullyFonctional)
				{
					_force = _srcForce;
					_state = BlowerState::FullyFonctional;
				}
			}
			else
			{
				_state = BlowerState::NotWorking;
			}
		}

	private:
		void applyForceInternal(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const
		{
			using ThisType = Storm::Blower<blowerType, BlowerEffectArea, BlowerTimeHandler>;

			// tmp is the position diff
			Storm::Vector3 tmp = inParticlePosition - _blowerPosition;
			if (BlowerEffectArea::isInside(tmp))
			{
				if constexpr (ThisType::hasDistanceEffect<BlowerEffectArea>(0))
				{
					// tmp is the position diff before entering the method, it is a force scaled by the distance effect after leaving the method (to optimize, we wouldn't create another temporary Vect3)...
					BlowerEffectArea::applyDistanceEffectToTemporary(_force, tmp);
					inOutParticleForce += tmp;
				}
				else
				{
					inOutParticleForce += _force;
				}
			}
		}

	public:
		void applyForce(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const final override
		{
			if (_state != BlowerState::NotWorking)
			{
				this->applyForceInternal(inParticlePosition, inOutParticleForce);
			}
		}

	public:
		bool operator==(const std::size_t id) const final override
		{
			return _id == id;
		}

		bool operator<(const std::size_t id) const final override
		{
			return _id < id;
		}

	private:
		std::size_t _id;

		Storm::Vector3 _force;
		const Storm::Vector3 _blowerPosition;
		const Storm::Vector3 _srcForce;

		BlowerState _state;
	};
}
