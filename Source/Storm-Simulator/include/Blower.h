#pragma once

#include "IBlower.h"
#include "BlowerState.h"


namespace Storm
{
	enum class BlowerType;

	template<Storm::BlowerType blowerType, class BlowerEffectArea, class BlowerTimeHandler, class BlowerCallbacks>
	class Blower :
		public Storm::IBlower,
		private BlowerEffectArea,
		private BlowerTimeHandler,
		private BlowerCallbacks
	{
	private:
		using ThisType = Storm::Blower<blowerType, BlowerEffectArea, BlowerTimeHandler, BlowerCallbacks>;

	public:
		Blower(const Storm::SceneBlowerConfig &blowerConfig) :
			BlowerEffectArea{ blowerConfig },
			BlowerTimeHandler{ blowerConfig },
			_id{ blowerConfig._blowerId },
			_enabled{ true },
			_srcForce{ blowerConfig._blowerForce },
			_force{ Vector3::Zero() },
			_forceNorm{ 0.f },
			_blowerPosition{ blowerConfig._blowerPosition },
			_state{ Storm::BlowerState::NotWorking }
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

		void advanceTime(float deltaTimeSec) final override
		{
			if (_enabled)
			{
				if (BlowerTimeHandler::advanceTime(deltaTimeSec))
				{
					if constexpr (ThisType::hasFadeIn<BlowerTimeHandler>(0))
					{
						float fadeInCoefficient;
						if (BlowerTimeHandler::shouldFadeIn(fadeInCoefficient))
						{
							if (_state != Storm::BlowerState::Fading)
							{
								this->setAndNotifyStateChanged<Storm::BlowerState::Fading>();
							}

							this->setActualForce(_srcForce * fadeInCoefficient);
							return;
						}
					}

					if constexpr (ThisType::hasFadeOut<BlowerTimeHandler>(0))
					{
						float fadeOutCoefficient;
						if (BlowerTimeHandler::shouldFadeOut(fadeOutCoefficient))
						{
							if (_state != Storm::BlowerState::Fading)
							{
								this->setAndNotifyStateChanged<Storm::BlowerState::Fading>();
							}

							this->setActualForce(_srcForce * fadeOutCoefficient);
							return;
						}
					}

					if (_state != Storm::BlowerState::FullyFonctional)
					{
						this->setActualForce(_srcForce);
						this->setAndNotifyStateChanged<Storm::BlowerState::FullyFonctional>();
					}
				}
				else if (_state != Storm::BlowerState::NotWorking)
				{
					this->setAndNotifyStateChanged<Storm::BlowerState::NotWorking>();
				}
			}
		}

		void setTime(float timeSec) final override
		{
			if (BlowerTimeHandler::forceSetTime(timeSec))
			{
				float coeff;
				if constexpr (ThisType::hasFadeIn<BlowerTimeHandler>(0))
				{
					if (BlowerTimeHandler::shouldFadeIn(coeff))
					{
						if (_state != Storm::BlowerState::Fading)
						{
							this->setAndNotifyStateChanged<Storm::BlowerState::Fading>();
							this->setActualForce(_srcForce * coeff);
						}
						return;
					}
				}

				if constexpr (ThisType::hasFadeOut<BlowerTimeHandler>(0))
				{
					if (BlowerTimeHandler::shouldFadeOut(coeff))
					{
						if (_state != Storm::BlowerState::Fading)
						{
							this->setAndNotifyStateChanged<Storm::BlowerState::Fading>();
							this->setActualForce(_srcForce * coeff);
						}
						return;
					}
				}

				if (_state != Storm::BlowerState::FullyFonctional)
				{
					this->setActualForce(_srcForce);
					this->setAndNotifyStateChanged<Storm::BlowerState::FullyFonctional>();
				}
			}
			else if (_state != Storm::BlowerState::NotWorking)
			{
				this->setAndNotifyStateChanged<Storm::BlowerState::NotWorking>();
			}
		}

	private:
		void applyForceInternal(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const
		{
			// tmp is the position diff
			Storm::Vector3 tmp = inParticlePosition - _blowerPosition;
			if (BlowerEffectArea::isInside(tmp))
			{
				if constexpr (ThisType::hasDistanceEffect<BlowerEffectArea>(0))
				{
					// tmp is the position diff before entering the method, it is a force scaled by the distance effect after leaving the method (to optimize, we wouldn't create another temporary Vect3)...
					BlowerEffectArea::applyDistanceEffectToTemporary(_force, _forceNorm, tmp);
					inOutParticleForce += tmp;
				}
				else
				{
					inOutParticleForce += _force;
				}
			}
		}

		template<Storm::BlowerState newState>
		void setAndNotifyStateChanged()
		{
			BlowerCallbacks::notifyStateChanged(_id, newState);
			_state = newState;
		}

		void setActualForce(const Storm::Vector3 &actualForce)
		{
			_force = actualForce;
			if constexpr (ThisType::hasDistanceEffect<BlowerEffectArea>(0))
			{
				_forceNorm = _force.norm();
			}
		}

	public:
		void applyForce(const Storm::Vector3 &inParticlePosition, Storm::Vector3 &inOutParticleForce) const final override
		{
			if (_state != Storm::BlowerState::NotWorking)
			{
				this->applyForceInternal(inParticlePosition, inOutParticleForce);
			}
		}

	public:
		void tweakEnabling() final override
		{
			if (_enabled)
			{
				this->setAndNotifyStateChanged<Storm::BlowerState::NotWorking>();
				_enabled = false;
			}
			else
			{
				_enabled = true;
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

		bool _enabled;

		Storm::Vector3 _force;
		const Storm::Vector3 _blowerPosition;
		const Storm::Vector3 _srcForce;

		float _forceNorm;

		Storm::BlowerState _state;
	};
}
