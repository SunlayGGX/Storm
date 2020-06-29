#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);

	public:
		void initializeIteration() final override;

	public:
		bool isFluids() const noexcept final override;

		const std::vector<float>& getPressures() const noexcept final override;
		std::vector<float>& getPressures() noexcept final override;

		const std::vector<float>& getPredictedDensities() const noexcept final override;
		std::vector<float>& getPredictedDensities() noexcept final override;
		const std::vector<Storm::Vector3>& getPredictedPositions() const noexcept final override;
		std::vector<Storm::Vector3>& getPredictedPositions() noexcept final override;
		const std::vector<Storm::Vector3>& getPredictedPressureForces() const noexcept final override;
		std::vector<Storm::Vector3>& getPredictedPressureForces() noexcept final override;

	public:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;

	public:
		void updatePosition(float deltaTimeInSec) final override;

	public:
		void applyPredictedPressureToTotalForce() final override;

	private:
		// "Predictive" SPH
		std::vector<float> _predictedDensity;
		std::vector<Storm::Vector3> _predictedPositions;
		std::vector<float> _pressures;
		std::vector<Storm::Vector3> _pressureForce;
	};
}
