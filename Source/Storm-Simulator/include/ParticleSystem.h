#pragma once

#include "NeighborParticleInfo.h"
#include "ParticleSystemContainer.h"

namespace Storm
{
	class IBlower;

	class ParticleSystem
	{
	public:
		ParticleSystem(unsigned int particleSystemIndex, std::vector<Storm::Vector3> &&worldPositions);
		ParticleSystem(unsigned int particleSystemIndex, const std::size_t particleCount);
		virtual ~ParticleSystem() = default;

	private:
		void initParticlesCount(const std::size_t particleCount);
		void resizeParticlesCount(const std::size_t particleCount);

	public:
		const std::vector<Storm::Vector3>& getPositions() const noexcept;
		std::vector<Storm::Vector3>& getPositions() noexcept;
		const std::vector<Storm::Vector3>& getVelocity() const noexcept;
		std::vector<Storm::Vector3>& getVelocity() noexcept;
		const std::vector<Storm::Vector3>& getForces() const noexcept;
		std::vector<Storm::Vector3>& getForces() noexcept;

		const std::vector<Storm::Vector3>& getTemporaryPressureForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryPressureForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryViscosityForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryViscosityForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryDragForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryDragForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryBernoulliDynamicPressureForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryBernoulliDynamicPressureForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryNoStickForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryNoStickForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryCoendaForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryCoendaForces() noexcept;
		const std::vector<Storm::Vector3>& getTemporaryPressureIntermediaryForces() const noexcept;
		std::vector<Storm::Vector3>& getTemporaryPressureIntermediaryForces() noexcept;

		const std::vector<Storm::ParticleNeighborhoodArray>& getNeighborhoodArrays() const noexcept;
		std::vector<Storm::ParticleNeighborhoodArray>& getNeighborhoodArrays() noexcept;

		const Storm::Vector3& getTotalForceNonPhysX() const noexcept;

		std::size_t getParticleCount() const noexcept;

		unsigned int getId() const noexcept;

		virtual bool isFluids() const noexcept = 0;
		virtual bool isStatic() const noexcept = 0;
		virtual bool isWall() const noexcept = 0;

		bool isDirty() const noexcept;
		void setIsDirty(bool dirty);

		// Only to be used with replaying feature.
		virtual void setPositions(std::vector<Storm::Vector3> &&positions) = 0;
		virtual void setVelocity(std::vector<Storm::Vector3> &&velocities) = 0;
		virtual void setForces(std::vector<Storm::Vector3> &&forces) = 0;
		virtual void setDensities(std::vector<float> &&densities) = 0;
		virtual void setPressures(std::vector<float> &&pressures) = 0;
		virtual void setVolumes(std::vector<float> &&volumes) = 0;
		virtual void setMasses(std::vector<float> &&masses) = 0;
		virtual void setNormals(std::vector<Storm::Vector3> &&normals) = 0;
		virtual void setTmpPressureForces(std::vector<Storm::Vector3> &&tmpPressureForces) = 0;
		virtual void setTmpViscosityForces(std::vector<Storm::Vector3> &&tmpViscoForces) = 0;
		virtual void setTmpDragForces(std::vector<Storm::Vector3> &&tmpDragForces) = 0;
		virtual void setTmpBernoulliDynamicPressureForces(std::vector<Storm::Vector3> &&tmpDynamicQForces) = 0;
		virtual void setTmpNoStickForces(std::vector<Storm::Vector3> &&tmpNoStickForces) = 0;
		virtual void setTmpCoendaForces(std::vector<Storm::Vector3> &&coendaForces) = 0;
		virtual void setTmpPressureIntermediaryForces(std::vector<Storm::Vector3> &&tmpPressuresIntermediaryForces) = 0;
		virtual void setParticleSystemPosition(const Storm::Vector3 &pSystemPosition) = 0;
		virtual void setParticleSystemTotalForce(const Storm::Vector3 &pSystemTotalForce) = 0;
		virtual void setParticleSystemWantedDensity(const float value) = 0;

		virtual void setParticleSystemTotalForceNonPhysX(const Storm::Vector3 &pSystemTotalForce);

	public:
		virtual void prepareSaving(const bool replayMode);

	public:
		void buildNeighborhood(const Storm::ParticleSystemContainer &allParticleSystems);

		// Debugging purpose. This method could be left trailing behind with recent neighborhood building modification done inside buildNeighborhood. The update would only be done when we need to debug!
		void buildSpecificParticleNeighborhood(const Storm::ParticleSystemContainer &allParticleSystems, const std::size_t pIndex);

	protected:
		virtual void buildNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength) = 0;

		// Debugging purpose. This method could be left trailing behind recent neighborhood building modification done inside buildNeighborhoodOnParticleSystemUsingSpacePartition (the real one because optimized). The update would only be done when we need to debug the feature!
		virtual void buildSpecificParticleNeighborhoodOnParticleSystemUsingSpacePartition(const Storm::ParticleSystemContainer &allParticleSystems, const std::size_t pIndex, const float kernelLength) = 0;

	public:
		virtual void updatePosition(float deltaTimeInSec, bool force) = 0;

	public:
		virtual void initializePreSimulation(const Storm::ParticleSystemContainer &allParticleSystems, const float kernelLength);

		virtual void onIterationStart();
		virtual void onSubIterationStart(const Storm::ParticleSystemContainer &allParticleSystems, const std::vector<std::unique_ptr<Storm::IBlower>> &blowers);
		virtual void onIterationEnd();

	public:
		virtual bool computeVelocityChange(float deltaTimeInSec, float highVelocityThresholdSquared) = 0;

	public:
		static float computeParticleDefaultVolume();

	public:
		static bool isElligibleNeighborParticle(const float kernelLengthSquared, const float normSquared);

	public:
		virtual void revertToCurrentTimestep(const std::vector<std::unique_ptr<Storm::IBlower>> &blowers) = 0;

	protected:
		virtual void resetParticleTemporaryForces(const std::size_t currentPIndex);

	protected:
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocity;
		std::vector<Storm::Vector3> _force;

		// Tmp force value only valid if we're selecting a force, or if we are recording.
		std::vector<Storm::Vector3> _tmpPressureForce;
		std::vector<Storm::Vector3> _tmpPressureIntermediaryForce; // For DFSPH first solver. This is not the pressure force but a record of a component of the final pressure force.
		std::vector<Storm::Vector3> _tmpViscosityForce;
		std::vector<Storm::Vector3> _tmpDragForce;
		std::vector<Storm::Vector3> _tmpBernoulliDynamicPressureForce;
		std::vector<Storm::Vector3> _tmpNoStickForce;
		std::vector<Storm::Vector3> _tmpCoendaForce;

		Storm::Vector3 _totalForceNonPhysX;

		// This contains the neighborhood per particle.
		// Note : For static rigid body, it does not contain the static particles neighborhood because we use it only one time (when initializing the volume) and this is a huge lost of computation time !
		// So when we would need to have the static neighborhood for static rigid body, we should add another array separated from this one and compute it only one time 
		// => Static rigid bodies doesn't move, so static particles from static rigid bodies would always be the same, at the same position. Thus recomputing it each timestep takes a lot of time for nothing (for getting the same result as the time step before). Therefore, storing the result used from this static neighborhood, instead of recomputing it, is wiser.
		std::vector<Storm::ParticleNeighborhoodArray> _neighborhood;

		unsigned int _particleSystemIndex;
		bool _isDirty;

		float _maxVelocityNorm;

	public:
		mutable std::mutex _mutex;
	};
}
