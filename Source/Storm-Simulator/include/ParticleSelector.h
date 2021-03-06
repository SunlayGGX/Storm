#pragma once


namespace Storm
{
	enum class ParticleSelectionMode : uint8_t;
	class UIFieldContainer;
	struct SelectedParticleData;

	class ParticleSelector
	{
	public:
		ParticleSelector();
		~ParticleSelector();

	public:
		void initialize();

	public:
		bool hasSelectedParticle() const noexcept;
		bool setParticleSelection(unsigned int particleSystemId, std::size_t particleIndex);
		bool clearParticleSelection();

	public:
		void setParticleSelectionDisplayMode(const Storm::ParticleSelectionMode newMode);
		void cycleParticleSelectionDisplayMode();

	public:
		void setSelectedParticleVelocity(const Storm::Vector3 &velocity);
		void setSelectedParticlePressureForce(const Storm::Vector3 &pressureForce);
		void setSelectedParticleViscosityForce(const Storm::Vector3 &viscoForce);
		void setSelectedParticleDragForce(const Storm::Vector3 &dragForce);
		void setSelectedParticleSumForce(const Storm::Vector3 &sumForce);
		void setRbPosition(const Storm::Vector3 &position);
		void setRbTotalForce(const Storm::Vector3 &totalForce);
		void clearRbTotalForce();

	public:
		const Storm::Vector3& getSelectedVectorToDisplay() const;
		const Storm::Vector3& getSelectedVectorPosition(const Storm::Vector3 &particlePosition) const;
		unsigned int getSelectedParticleSystemId() const noexcept;
		std::size_t getSelectedParticleIndex() const noexcept;

	public:
		void logForceComponents() const;

	private:
		std::unique_ptr<Storm::SelectedParticleData> _selectedParticleData;

		Storm::ParticleSelectionMode _currentParticleSelectionMode;

		std::wstring _selectionModeStr;
		std::unique_ptr<Storm::UIFieldContainer> _fields;
	};
}
