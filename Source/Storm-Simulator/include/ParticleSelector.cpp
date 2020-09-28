#include "ParticleSelector.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"

#include "ParticleSelectionMode.h"


Storm::ParticleSelector::ParticleSelector() :
	_currentParticleSelectionMode{ Storm::ParticleSelectionMode::ViscosityAndPressure }
{
	_selectedParticleData._selectedParticle = std::make_pair(std::numeric_limits<decltype(_selectedParticleData._selectedParticle.first)>::max(), 0);
}

bool Storm::ParticleSelector::hasSelectedParticle() const noexcept
{
	return _selectedParticleData._selectedParticle.first != std::numeric_limits<decltype(_selectedParticleData._selectedParticle.first)>::max();
}

bool Storm::ParticleSelector::setParticleSelection(unsigned int particleSystemId, std::size_t particleIndex)
{
	if (_selectedParticleData._selectedParticle.first != particleSystemId || _selectedParticleData._selectedParticle.second != particleIndex)
	{
		Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
		graphicMgr.safeSetSelectedParticle(particleSystemId, particleIndex);

		_selectedParticleData._selectedParticle.first = particleSystemId;
		_selectedParticleData._selectedParticle.second = particleIndex;

		return true;
	}

	return false;
}

bool Storm::ParticleSelector::clearParticleSelection()
{
	if (this->hasSelectedParticle())
	{
		Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
		graphicMgr.safeClearSelectedParticle();

		_selectedParticleData._selectedParticle.first = std::numeric_limits<decltype(_selectedParticleData._selectedParticle.first)>::max();

		return true;
	}

	return false;
}

void Storm::ParticleSelector::cycleParticleSelection()
{
	const uint8_t cycledValue = (static_cast<uint8_t>(_currentParticleSelectionMode) + 1) % static_cast<uint8_t>(Storm::ParticleSelectionMode::SelectionModeCount);
	_currentParticleSelectionMode = static_cast<Storm::ParticleSelectionMode>(cycledValue);
}

void Storm::ParticleSelector::setSelectedParticlePressureForce(const Storm::Vector3 &pressureForce)
{
	_selectedParticleData._pressureForce = pressureForce;
}

void Storm::ParticleSelector::setSelectedParticleViscosityForce(const Storm::Vector3 &viscoForce)
{
	_selectedParticleData._viscosityForce = viscoForce;
}

void Storm::ParticleSelector::setSelectedParticleSumForce(const Storm::Vector3 &sumForce)
{
	_selectedParticleData._externalSumForces = sumForce;
}

const Storm::Vector3& Storm::ParticleSelector::getSelectedParticleForceToDisplay() const
{
	switch (_currentParticleSelectionMode)
	{
	case Storm::ParticleSelectionMode::Pressure:				return _selectedParticleData._pressureForce;
	case Storm::ParticleSelectionMode::Viscosity:				return _selectedParticleData._viscosityForce;
	case Storm::ParticleSelectionMode::ViscosityAndPressure:	return _selectedParticleData._externalSumForces;

	case Storm::ParticleSelectionMode::SelectionModeCount:
	default:
		Storm::throwException<std::exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(_currentParticleSelectionMode));
		break;
	}
}

unsigned int Storm::ParticleSelector::getSelectedParticleSystemId() const noexcept
{
	return _selectedParticleData._selectedParticle.first;
}

std::size_t Storm::ParticleSelector::getSelectedParticleIndex() const noexcept
{
	return _selectedParticleData._selectedParticle.second;
}

