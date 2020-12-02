#include "ParticleSelector.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"

#include "ParticleSelectionMode.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#define STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME "Selected P. Mode"


namespace
{
	std::wstring parseSelectedParticleMode(const Storm::ParticleSelectionMode mode)
	{
		switch (mode)
		{
		case Storm::ParticleSelectionMode::Pressure: return STORM_TEXT("Pressure");
		case Storm::ParticleSelectionMode::Viscosity: return STORM_TEXT("Viscosity");
		case Storm::ParticleSelectionMode::ViscosityAndPressure: return STORM_TEXT("All");
		case Storm::ParticleSelectionMode::RbForce: return STORM_TEXT("Rb Total force");

		case Storm::ParticleSelectionMode::SelectionModeCount:
		default:
			Storm::throwException<std::exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(mode));
			break;
		}
	}

#define STORM_XMACRO_SELECTION_MODE \
	STORM_XMACRO_ELEM_BASE_SELECTION_MODE(FluidParticleSelectionMode, STORM_XMACRO_SELECTION_FLUIDS_MODE_BINDINGS) \
	STORM_XMACRO_ELEM_BASE_SELECTION_MODE(RbParticleSelectionMode, STORM_XMACRO_SELECTION_RB_MODE_BINDINGS) \

	// Don't modify this macro directly unless necessary.
	// It is the macro that make the links between the STORM_XMACRO_SELECTION_MODE xmacro, and the bindings mode xmacro.
#define STORM_XMACRO_ELEM_BASE_SELECTION_MODE(SelectionMode, BindingsXMacro) STORM_XMACRO_ELEM_SELECTION_MODE(SelectionMode, BindingsXMacro(SelectionMode))

#define STORM_XMACRO_SELECTION_FLUIDS_MODE_BINDINGS(SelectionMode)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, ViscosityAndPressure)	\

#define STORM_XMACRO_SELECTION_RB_MODE_BINDINGS(SelectionMode)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, ViscosityAndPressure)	\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, RbForce)					\


#define STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, BindingValueName) BindingValueName,
#define STORM_XMACRO_ELEM_SELECTION_MODE(SelectionMode, ...)		\
	enum class SelectionMode										\
	{																\
		__VA_ARGS__													\
		SelectionCount,												\
	};

	STORM_XMACRO_SELECTION_MODE;
#undef STORM_XMACRO_ELEM_SELECTION_MODE
#undef STORM_XMACRO_ELEM_SELECTION_BINDING

	template<class UsedSelectionMode> Storm::ParticleSelectionMode retrieveSelectionMode(uint8_t selectionModeAgnostic);

#define STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, BindingValueName) case SelectionMode::BindingValueName: return Storm::ParticleSelectionMode::BindingValueName;
#define STORM_XMACRO_ELEM_SELECTION_MODE(SelectionMode, ...) \
	template<> Storm::ParticleSelectionMode retrieveSelectionMode<SelectionMode>(uint8_t selectionModeAgnostic) \
	{																											\
		switch (static_cast<SelectionMode>(selectionModeAgnostic))												\
		{																										\
			__VA_ARGS__																							\
																												\
		default:																								\
			return static_cast<Storm::ParticleSelectionMode>(0);												\
		};																										\
	}

	STORM_XMACRO_SELECTION_MODE
#undef STORM_XMACRO_ELEM_SELECTION_MODE
#undef STORM_XMACRO_ELEM_SELECTION_BINDING
}


Storm::ParticleSelector::ParticleSelector() :
	_currentParticleSelectionMode{ Storm::ParticleSelectionMode::ViscosityAndPressure }
{
	_selectionModeStr = parseSelectedParticleMode(_currentParticleSelectionMode);
	_selectedParticleData._selectedParticle = std::make_pair(std::numeric_limits<decltype(_selectedParticleData._selectedParticle.first)>::max(), 0);

	this->clearRbTotalForce();
}

Storm::ParticleSelector::~ParticleSelector() = default;

void Storm::ParticleSelector::initialize()
{
	_fields = std::make_unique<Storm::UIFieldContainer>();

	(*_fields)
		.bindField(STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME, _selectionModeStr)
		;
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

void Storm::ParticleSelector::setParticleSelectionDisplayMode(const Storm::ParticleSelectionMode newMode)
{
	_selectionModeStr = parseSelectedParticleMode(newMode);
	_currentParticleSelectionMode = newMode;

	_fields->pushField(STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME);
}

void Storm::ParticleSelector::cycleParticleSelectionDisplayMode()
{
	const uint8_t cycledValue = (static_cast<uint8_t>(_currentParticleSelectionMode) + 1) % static_cast<uint8_t>(Storm::ParticleSelectionMode::SelectionModeCount);

	Storm::ParticleSelectionMode newMode;
	if (_selectedParticleData._hasRbTotalForce)
	{
		newMode = retrieveSelectionMode<RbParticleSelectionMode>(cycledValue);
	}
	else
	{
		newMode = retrieveSelectionMode<FluidParticleSelectionMode>(cycledValue);
	}

	this->setParticleSelectionDisplayMode(newMode);
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

void Storm::ParticleSelector::setRbPosition(const Storm::Vector3 &position)
{
	_selectedParticleData._rbPosition = position;
	_selectedParticleData._hasRbTotalForce = true;
}

void Storm::ParticleSelector::setRbTotalForce(const Storm::Vector3 &totalForce)
{
	_selectedParticleData._totalForcesOnRb = totalForce;
	_selectedParticleData._hasRbTotalForce = true;
}

void Storm::ParticleSelector::clearRbTotalForce()
{
	_selectedParticleData._hasRbTotalForce = false;
}

const Storm::Vector3& Storm::ParticleSelector::getSelectedForceToDisplay() const
{
	switch (_currentParticleSelectionMode)
	{
	case Storm::ParticleSelectionMode::Pressure:				return _selectedParticleData._pressureForce;
	case Storm::ParticleSelectionMode::Viscosity:				return _selectedParticleData._viscosityForce;
	case Storm::ParticleSelectionMode::ViscosityAndPressure:	return _selectedParticleData._externalSumForces;
	case Storm::ParticleSelectionMode::RbForce:					return _selectedParticleData._totalForcesOnRb;

	case Storm::ParticleSelectionMode::SelectionModeCount:
	default:
		Storm::throwException<std::exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(_currentParticleSelectionMode));
		break;
	}
}

const Storm::Vector3& Storm::ParticleSelector::getSelectedForcePosition(const Storm::Vector3 &particlePosition) const
{
	switch (_currentParticleSelectionMode)
	{
	case Storm::ParticleSelectionMode::RbForce: return _selectedParticleData._rbPosition;
	default:									return particlePosition;
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

void Storm::ParticleSelector::logForceComponents() const
{
	std::string rbSpecificInfosStr;
	if (_selectedParticleData._hasRbTotalForce)
	{
		const std::string rbForceStr = Storm::toStdString(_selectedParticleData._totalForcesOnRb);
		const std::string rbForceNormStr = Storm::toStdString(_selectedParticleData._totalForcesOnRb.norm());
		
		rbSpecificInfosStr.reserve(40 + rbForceStr.size() + rbForceNormStr.size());

		rbSpecificInfosStr += "\nRigidbody total force: ";
		rbSpecificInfosStr += rbForceStr;
		rbSpecificInfosStr += ". Norm: ";
		rbSpecificInfosStr += rbForceNormStr;
		rbSpecificInfosStr += " m.";
	}

	LOG_ALWAYS <<
		"Particle Forces components are :\n"
		"Pressure: " << _selectedParticleData._pressureForce << ". Norm: " << _selectedParticleData._pressureForce.norm() << " m.\n"
		"Viscosity: " << _selectedParticleData._viscosityForce << ". Norm: " << _selectedParticleData._viscosityForce.norm() << " m.\n"
		"Sum : " << _selectedParticleData._externalSumForces << ". Norm: " << _selectedParticleData._externalSumForces.norm() << " m." <<
		rbSpecificInfosStr
		;
}

