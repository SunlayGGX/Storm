#pragma once


namespace Storm
{
	enum class ParticleSelectionMode : uint8_t;

	class ParticleSelector
	{
	public:
		struct SelectedParticleData
		{
			std::pair<unsigned int, std::size_t> _selectedParticle;
			Storm::Vector3 _pressureForce;
			Storm::Vector3 _viscosityForce;
			Storm::Vector3 _externalSumForces;
		};

	public:
		ParticleSelector();

	public:
		bool hasSelectedParticle() const noexcept;
		bool setParticleSelection(unsigned int particleSystemId, std::size_t particleIndex);
		bool clearParticleSelection();

	public:
		void cycleParticleSelection();

	public:
		void setSelectedParticlePressureForce(const Storm::Vector3 &pressureForce);
		void setSelectedParticleViscosityForce(const Storm::Vector3 &viscoForce);
		void setSelectedParticleSumForce(const Storm::Vector3 &sumForce);

	public:
		const Storm::Vector3& getSelectedParticleForceToDisplay() const;
		unsigned int getSelectedParticleSystemId() const noexcept;
		std::size_t getSelectedParticleIndex() const noexcept;

	private:
		SelectedParticleData _selectedParticleData;

		Storm::ParticleSelectionMode _currentParticleSelectionMode;
	};
}
