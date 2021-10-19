#pragma once

#include "ParticleSystem.h"


namespace Storm
{
	class UIFieldContainer;

	class FluidParticleSystem : public Storm::ParticleSystem
	{
	public:
		FluidParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		FluidParticleSystem(unsigned int particleSystemIndex, std::size_t particleCount);
		~FluidParticleSystem();

	public:
		void onIterationStart() final override;
		void onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) final override;
		void onIterationEnd() final override;

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
		void setNormals(std::vector<Storm::Vector3> &&normals) final override;
		void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) final override;
		void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) final override;
		void setTmpDragForces(std::vector<Storm::Vector3> &&tmpDragForces) final override;
		void setTmpBernoulliDynamicPressureForces(std::vector<Storm::Vector3> &&tmpDynamicQForces) final override;
		void setTmpNoStickForces(std::vector<Storm::Vector3> &&tmpNoStickForces) final override;
		void setTmpCoendaForces(std::vector<Storm::Vector3> &&tmpCoendaForces) final override;
		void setTmpPressureDensityIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces) final override;
		void setTmpPressureVelocityIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces) final override;
		void setTmpBlowerForces(std::vector<Storm::Vector3> &&tmpBlowerForces) final override;
		void setParticleSystemPosition(const Storm::Vector3 &pSystemPosition) final override;
		void setParticleSystemTotalForce(const Storm::Vector3 &pSystemTotalForce) final override;
		void setParticleSystemWantedDensity(const float value) final override;

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

		std::vector<Storm::Vector3>& getVelocityPreTimestep() noexcept;
		const std::vector<Storm::Vector3>& getVelocityPreTimestep() const noexcept;

		std::vector<Storm::Vector3>& getTmpBlowerForces() noexcept;
		const std::vector<Storm::Vector3>& getTmpBlowerForces() const noexcept;

		void setGravityEnabled(bool enabled) noexcept;

	public:
		void buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength) final override;
		void buildSpecificParticleNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const std::size_t particleIndex, const float kernelLength) final override;

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
		std::vector<Storm::Vector3> _tmpBlowerForces;

		float _restDensity;
		float _wantedDensity;
		float _particleVolume;

		bool _gravityEnabled;
		
		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
