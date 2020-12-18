#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		FluidParticleSystem(unsigned int particleSystemIndex, std::size_t particleCount);

	public:
		void onIterationStart() final override;
		void onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) final override;

	public:
		bool isFluids() const noexcept final override;
		bool isStatic() const noexcept final override;
		bool isWall() const noexcept final override;

		void setPositions(std::vector<Storm::Vector3> &&positions) final override;
		void setVelocity(std::vector<Storm::Vector3> &&velocities) final override;
		void setForces(std::vector<Storm::Vector3> &&forces) final override;
		void setDensities(std::vector<float> &&densities) final override;
		void setPressures(std::vector<float> &&pressures) final override;
		void setVolumes(std::vector<float> &&volumes) final override;
		void setMasses(std::vector<float> &&masses) final override;
		void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) final override;
		void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) final override;
		void setParticleSystemPosition(const Storm::Vector3 &pSystemPosition) final override;
		void setParticleSystemTotalForce(const Storm::Vector3 &pSystemTotalForce) final override;

		void prepareSaving(const bool replayMode) final override;

	public:
		// Accessible by dynamic casting.
		float getRestDensity() const noexcept;
		float getParticleVolume() const noexcept;

		std::vector<float>& getMasses() noexcept;
		const std::vector<float>& getMasses() const noexcept;
		std::vector<float>& getDensities() noexcept;
		const std::vector<float>& getDensities() const noexcept;
		std::vector<float>& getPressures() noexcept;
		const std::vector<float>& getPressures() const noexcept;

	public:
		void buildNeighborhoodOnParticleSystem(const Storm::ParticleSystem &otherParticleSystem, const float kernelLengthSquared) final override;
		void buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLengthSquared) final override;

	public:
		bool computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared) final override;
		void updatePosition(float deltaTimeInSec, bool force) final override;

	public:
		void revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) final override;

	private:
		void internalInitializeForce(const Storm::Vector3 &gravityAccel, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers, Storm::Vector3 &force, const std::size_t currentPIndex);

	private:
		std::vector<float> _masses;
		std::vector<float> _densities;
		std::vector<float> _pressure;

		std::vector<Storm::Vector3> _velocityPreTimestep;

		float _restDensity;
		float _particleVolume;
	};
}
