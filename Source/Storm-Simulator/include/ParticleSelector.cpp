#include "ParticleSelector.h"
#include "SelectedParticleData.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "ISerializerManager.h"

#include "SceneFluidConfig.h"
#include "SceneSimulationConfig.h"
#include "SceneFluidCustomDFSPHConfig.h"

#include "ParticleSelectionMode.h"

#include "SimulationMode.h"

#include "SerializeSupportedFeatureLayout.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#define STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME "Selected P. Mode"


namespace
{
	std::wstring parseSelectedParticleMode(const Storm::ParticleSelectionMode mode)
	{
		switch (mode)
		{
		case Storm::ParticleSelectionMode::Velocity: return STORM_TEXT("Velocity");
		case Storm::ParticleSelectionMode::Pressure: return STORM_TEXT("Pressure");
		case Storm::ParticleSelectionMode::Viscosity: return STORM_TEXT("Viscosity");
		case Storm::ParticleSelectionMode::Drag: return STORM_TEXT("Drag");
		case Storm::ParticleSelectionMode::DynamicPressure: return STORM_TEXT("DynamicQ");
		case Storm::ParticleSelectionMode::NoStick: return STORM_TEXT("NoStick");
		case Storm::ParticleSelectionMode::AllOnParticle: return STORM_TEXT("All On Particle");
		case Storm::ParticleSelectionMode::TotalEngineForce: return STORM_TEXT("All On System (Engine)");
		case Storm::ParticleSelectionMode::Normal: return STORM_TEXT("Normal");
		case Storm::ParticleSelectionMode::RbForce: return STORM_TEXT("Rb Total force");
		case Storm::ParticleSelectionMode::AverageRbForce: return STORM_TEXT("Rb Average Total force");

		case Storm::ParticleSelectionMode::SelectionModeCount:
		default:
			Storm::throwException<Storm::Exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(mode));
			break;
		}
	}

	constexpr auto dummySelectedParticleIndex()
	{
		return std::numeric_limits<decltype(Storm::SelectedParticleData::_selectedParticle.first)>::max();
	}

#define STORM_XMACRO_SELECTION_MODE \
	STORM_XMACRO_ELEM_BASE_SELECTION_MODE(FluidParticleSelectionMode, STORM_XMACRO_SELECTION_FLUIDS_MODE_BINDINGS) \
	STORM_XMACRO_ELEM_BASE_SELECTION_MODE(RbParticleSelectionMode, STORM_XMACRO_SELECTION_RB_MODE_BINDINGS) \

	// Don't modify this macro directly unless necessary.
	// It is the macro that make the links between the STORM_XMACRO_SELECTION_MODE xmacro, and the bindings mode xmacro.
#define STORM_XMACRO_ELEM_BASE_SELECTION_MODE(SelectionMode, BindingsXMacro) STORM_XMACRO_ELEM_SELECTION_MODE(SelectionMode, BindingsXMacro(SelectionMode))

#define STORM_XMACRO_SELECTION_FLUIDS_MODE_BINDINGS(SelectionMode)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Velocity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Drag)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, DynamicPressure)			\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, NoStick)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AllOnParticle)			\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, TotalEngineForce)		\

#define STORM_XMACRO_SELECTION_RB_MODE_BINDINGS(SelectionMode)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Velocity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Drag)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, DynamicPressure)			\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, NoStick)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AllOnParticle)			\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, TotalEngineForce)		\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Normal)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, RbForce)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AverageRbForce)			\


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
	template<> Storm::ParticleSelectionMode retrieveSelectionMode<SelectionMode>(uint8_t selectionModeAgnostic)	\
	{																														\
		switch (static_cast<SelectionMode>(selectionModeAgnostic))															\
		{																													\
			__VA_ARGS__																										\
																															\
		default:																											\
			return static_cast<Storm::ParticleSelectionMode>(0);															\
		};																													\
	}

	STORM_XMACRO_SELECTION_MODE
#undef STORM_XMACRO_ELEM_SELECTION_MODE
#undef STORM_XMACRO_ELEM_SELECTION_BINDING


	template<class UsedSelectionMode>
	bool checkSkippedSelectionMode(Storm::ParticleSelectionMode selectionMode)
	{
		switch (selectionMode)
		{
		case Storm::ParticleSelectionMode::DynamicPressure:
		{
			const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
			const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

			const Storm::SceneSimulationConfig &simulConfig = configMgr.getSceneSimulationConfig();
			switch (simulConfig._simulationMode)
			{
			case Storm::SimulationMode::DFSPH:
			{
				const Storm::SceneFluidConfig &fluidConfig = configMgr.getSceneFluidConfig();
				const Storm::SceneFluidCustomDFSPHConfig &dfsphFluidConfig = static_cast<const Storm::SceneFluidCustomDFSPHConfig &>(*fluidConfig._customSimulationSettings);
				return dfsphFluidConfig._useBernoulliPrinciple;
			}

			default:
				return true;
			}
		}

		case Storm::ParticleSelectionMode::AllOnParticle:
		case Storm::ParticleSelectionMode::Velocity:
		case Storm::ParticleSelectionMode::Pressure:
		case Storm::ParticleSelectionMode::Viscosity:
		case Storm::ParticleSelectionMode::NoStick:
		case Storm::ParticleSelectionMode::Drag:
		case Storm::ParticleSelectionMode::Normal:
		case Storm::ParticleSelectionMode::TotalEngineForce:
		case Storm::ParticleSelectionMode::RbForce:
		case Storm::ParticleSelectionMode::AverageRbForce:
		case Storm::ParticleSelectionMode::SelectionModeCount:
		default:
			return true;
		}
	}

	template<class UsedSelectionMode>
	Storm::ParticleSelectionMode cycleSelectionMode(Storm::ParticleSelectionMode currentSelectionMode)
	{
		do
		{
			const uint8_t cycledValue = (static_cast<uint8_t>(currentSelectionMode) + 1) % static_cast<uint8_t>(Storm::ParticleSelectionMode::SelectionModeCount);
			currentSelectionMode = retrieveSelectionMode<UsedSelectionMode>(cycledValue);

		} while (!checkSkippedSelectionMode<UsedSelectionMode>(currentSelectionMode));

		return currentSelectionMode;
	}
}


Storm::ParticleSelector::ParticleSelector() :
	_currentParticleSelectionMode{ Storm::ParticleSelectionMode::AllOnParticle },
	_selectedParticleData{ std::make_unique<Storm::SelectedParticleData>() },
	_supportedFeatures{ nullptr }
{
	_selectionModeStr = parseSelectedParticleMode(_currentParticleSelectionMode);
	_selectedParticleData->_selectedParticle = std::make_pair(dummySelectedParticleIndex(), 0);

	this->clearRbTotalForce();
}

Storm::ParticleSelector::~ParticleSelector() = default;

void Storm::ParticleSelector::initialize(const bool isInReplayMode)
{
	_fields = std::make_unique<Storm::UIFieldContainer>();

	(*_fields)
		.bindField(STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME, _selectionModeStr)
		;

	if (isInReplayMode)
	{
		const Storm::ISerializerManager &serializerMgr = Storm::SingletonHolder::instance().getSingleton<Storm::ISerializerManager>();
		_supportedFeatures = serializerMgr.getRecordSupportedFeature();
	}
	else
	{
		// The default one.
		_supportedFeatures = std::make_shared<Storm::SerializeSupportedFeatureLayout>();
	}
}

bool Storm::ParticleSelector::hasSelectedParticle() const noexcept
{
	return _selectedParticleData->_selectedParticle.first != std::numeric_limits<decltype(_selectedParticleData->_selectedParticle.first)>::max();
}

bool Storm::ParticleSelector::setParticleSelection(unsigned int particleSystemId, std::size_t particleIndex)
{
	auto &selectedParticleRef = _selectedParticleData->_selectedParticle;
	if (selectedParticleRef.first != particleSystemId || selectedParticleRef.second != particleIndex)
	{
		Storm::IGraphicsManager &graphicMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>();
		graphicMgr.safeSetSelectedParticle(particleSystemId, particleIndex);

		selectedParticleRef.first = particleSystemId;
		selectedParticleRef.second = particleIndex;

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

		_selectedParticleData->_selectedParticle.first = dummySelectedParticleIndex();

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
	Storm::ParticleSelectionMode newMode;
	if (_selectedParticleData->_hasRbTotalForce)
	{
		newMode = cycleSelectionMode<RbParticleSelectionMode>(_currentParticleSelectionMode);
	}
	else
	{
		newMode = cycleSelectionMode<FluidParticleSelectionMode>(_currentParticleSelectionMode);
	}

	this->setParticleSelectionDisplayMode(newMode);
}

void Storm::ParticleSelector::setSelectedParticleVelocity(const Storm::Vector3 &velocity)
{
	_selectedParticleData->_velocity = velocity;
}

void Storm::ParticleSelector::setSelectedParticlePressureForce(const Storm::Vector3 &pressureForce)
{
	_selectedParticleData->_pressureForce = pressureForce;
}

void Storm::ParticleSelector::setSelectedParticleViscosityForce(const Storm::Vector3 &viscoForce)
{
	_selectedParticleData->_viscosityForce = viscoForce;
}

void Storm::ParticleSelector::setSelectedParticleDragForce(const Storm::Vector3 &dragForce)
{
	_selectedParticleData->_dragForce = dragForce;
}

void Storm::ParticleSelector::setSelectedParticleBernoulliDynamicPressureForce(const Storm::Vector3& qForce)
{
	_selectedParticleData->_dynamicPressureForce = qForce;
}


void Storm::ParticleSelector::setSelectedParticleNoStickForce(const Storm::Vector3 &noStickForce)
{
	_selectedParticleData->_noStickForce = noStickForce;
}

void Storm::ParticleSelector::setSelectedParticleSumForce(const Storm::Vector3 &sumForce)
{
	_selectedParticleData->_externalSumForces = sumForce;
}

void Storm::ParticleSelector::setTotalEngineSystemForce(const Storm::Vector3 &totalForce)
{
	_selectedParticleData->_totalEngineForce = totalForce;
}

void Storm::ParticleSelector::setRbParticleNormals(const Storm::Vector3 &normals)
{
	_selectedParticleData->_rbNormals = normals;
	_selectedParticleData->_hasRbTotalForce = true;
}

void Storm::ParticleSelector::setRbPosition(const Storm::Vector3 &position)
{
	_selectedParticleData->_rbPosition = position;
	_selectedParticleData->_hasRbTotalForce = true;
}

void Storm::ParticleSelector::setRbTotalForce(const Storm::Vector3 &totalForce)
{
	_selectedParticleData->_totalForcesOnRb = totalForce;
	_selectedParticleData->_averageForcesOnRb.addValue(totalForce);
	_selectedParticleData->_hasRbTotalForce = true;
}

void Storm::ParticleSelector::clearRbTotalForce()
{
	_selectedParticleData->_hasRbTotalForce = false;
}

const Storm::Vector3& Storm::ParticleSelector::getSelectedVectorToDisplay() const
{
	switch (_currentParticleSelectionMode)
	{
	case Storm::ParticleSelectionMode::Velocity:				return _selectedParticleData->_velocity;
	case Storm::ParticleSelectionMode::Pressure:				return _selectedParticleData->_pressureForce;
	case Storm::ParticleSelectionMode::Viscosity:				return _selectedParticleData->_viscosityForce;
	case Storm::ParticleSelectionMode::Drag:					return _selectedParticleData->_dragForce;
	case Storm::ParticleSelectionMode::DynamicPressure:			return _selectedParticleData->_dynamicPressureForce;
	case Storm::ParticleSelectionMode::NoStick:					return _selectedParticleData->_noStickForce;
	case Storm::ParticleSelectionMode::AllOnParticle:			return _selectedParticleData->_externalSumForces;
	case Storm::ParticleSelectionMode::TotalEngineForce:		return _selectedParticleData->_totalEngineForce;
	case Storm::ParticleSelectionMode::Normal:					return _selectedParticleData->_rbNormals;
	case Storm::ParticleSelectionMode::RbForce:					return _selectedParticleData->_totalForcesOnRb;
	case Storm::ParticleSelectionMode::AverageRbForce:			return _selectedParticleData->_averageForcesOnRb.getAverage();

	case Storm::ParticleSelectionMode::SelectionModeCount:
	default:
		Storm::throwException<Storm::Exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(_currentParticleSelectionMode));
		break;
	}
}

const Storm::Vector3& Storm::ParticleSelector::getSelectedVectorPosition(const Storm::Vector3 &particlePosition) const
{
	switch (_currentParticleSelectionMode)
	{
	case Storm::ParticleSelectionMode::RbForce:
	case Storm::ParticleSelectionMode::AverageRbForce:
		return _selectedParticleData->_rbPosition;

	case Storm::ParticleSelectionMode::TotalEngineForce:
		// Since fluids does not have a particle system position (the system is free to expands and its center is not worth computing)
		// We prefer to start the force at the selected particle, otherwise we can use the rb position since the total engine force is a force per system.
		return _selectedParticleData->_hasRbTotalForce ? _selectedParticleData->_rbPosition : particlePosition;

	case Storm::ParticleSelectionMode::Normal:
	case Storm::ParticleSelectionMode::Velocity:
	case Storm::ParticleSelectionMode::Pressure:
	case Storm::ParticleSelectionMode::Viscosity:
	case Storm::ParticleSelectionMode::Drag:
	case Storm::ParticleSelectionMode::DynamicPressure:
	case Storm::ParticleSelectionMode::NoStick:
	case Storm::ParticleSelectionMode::AllOnParticle:
	case Storm::ParticleSelectionMode::SelectionModeCount:
	default:
		return particlePosition;
	}
}

unsigned int Storm::ParticleSelector::getSelectedParticleSystemId() const noexcept
{
	return _selectedParticleData->_selectedParticle.first;
}

std::size_t Storm::ParticleSelector::getSelectedParticleIndex() const noexcept
{
	return _selectedParticleData->_selectedParticle.second;
}

void Storm::ParticleSelector::logForceComponents() const
{
	const auto &selectedParticleDataRef = *_selectedParticleData;

	std::string rbSpecificInfosStr;
	if (selectedParticleDataRef._hasRbTotalForce)
	{
		const Storm::Vector3 &averageTotalForce = selectedParticleDataRef._averageForcesOnRb.getAverage();

		const std::string rbNormalStr = Storm::toStdString(selectedParticleDataRef._rbNormals);
		const std::string rbNormalNormStr = Storm::toStdString(selectedParticleDataRef._rbNormals.norm());
		const std::string rbForceStr = Storm::toStdString(selectedParticleDataRef._totalForcesOnRb);
		const std::string rbForceNormStr = Storm::toStdString(selectedParticleDataRef._totalForcesOnRb.norm());
		const std::string rbAverageForceStr = Storm::toStdString(averageTotalForce);
		const std::string rbAverageForceNormStr = Storm::toStdString(averageTotalForce.norm());
		
		rbSpecificInfosStr.reserve(64 + rbForceStr.size() + rbForceNormStr.size() + rbAverageForceStr.size() + rbAverageForceNormStr.size());

#define STORM_SUPPORT_STR(supported) (supported ? "" : "  (NOT SUPPORTED)")

#define STORM_APPEND_RB_VECTOR_DATA(vectorName, vectorStr, vectorNormStr, unit, supported)	\
	rbSpecificInfosStr += "\n" vectorName ": ";												\
	rbSpecificInfosStr += vectorStr;														\
	rbSpecificInfosStr += ". Norm: ";														\
	rbSpecificInfosStr += vectorNormStr;													\
	rbSpecificInfosStr += " " unit ".";														\
	rbSpecificInfosStr += STORM_SUPPORT_STR(supported)										\

		STORM_APPEND_RB_VECTOR_DATA("Rigidbody P normal", rbNormalStr, rbNormalNormStr, "", _supportedFeatures->_hasNormals);
		STORM_APPEND_RB_VECTOR_DATA("Complete Forces (with PhysX)", rbForceStr, rbForceNormStr, "N", _supportedFeatures->_hasPSystemGlobalForce);
		STORM_APPEND_RB_VECTOR_DATA("Average", rbAverageForceStr, rbAverageForceNormStr, "N", true);
	}

#define STORM_APPEND_DATA_TO_STREAM(name, memberName, unit, supported) \
	name ": " << selectedParticleDataRef.memberName << ". Norm: " << selectedParticleDataRef.memberName.norm() << " " unit "." << STORM_SUPPORT_STR(supported) << "\n"

	LOG_ALWAYS <<
		"Particle vectors components are :\n"
		STORM_APPEND_DATA_TO_STREAM("Velocity", _velocity, "m/s", true)
		STORM_APPEND_DATA_TO_STREAM("Pressure", _pressureForce, "N", true)
		STORM_APPEND_DATA_TO_STREAM("Viscosity", _viscosityForce, "N", true)
		STORM_APPEND_DATA_TO_STREAM("Drag", _dragForce, "N", _supportedFeatures->_hasDragComponentforces)
		STORM_APPEND_DATA_TO_STREAM("DynamicPressure", _dynamicPressureForce, "N", _supportedFeatures->_hasDynamicPressureQForces)
		STORM_APPEND_DATA_TO_STREAM("NoStick", _noStickForce, "N", _supportedFeatures->_hasNoStickForces)
		STORM_APPEND_DATA_TO_STREAM("Sum", _externalSumForces, "N", true)
		STORM_APPEND_DATA_TO_STREAM("Total system force", _totalEngineForce, "N", _supportedFeatures->_hasPSystemTotalEngineForce) <<
		rbSpecificInfosStr
		;

#undef STORM_APPEND_RB_VECTOR_DATA
#undef STORM_APPEND_DATA_TO_STREAM
#undef STORM_SUPPORT_STR
}

