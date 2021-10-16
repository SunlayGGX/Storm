#include "ParticleSelector.h"
#include "SelectedParticleData.h"

#include "SingletonHolder.h"
#include "IGraphicsManager.h"
#include "IConfigManager.h"
#include "ISerializerManager.h"

#include "SceneFluidConfig.h"
#include "SceneSimulationConfig.h"
#include "SceneFluidCustomDFSPHConfig.h"
#include "GeneralDebugConfig.h"

#include "ParticleSelectionMode.h"

#include "SimulationMode.h"

#include "SerializeSupportedFeatureLayout.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#include "StringAlgo.h"

#include <set>

#include <boost\algorithm\string\case_conv.hpp>

#define STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME "Selected P. Mode"


namespace
{
	std::wstring parseSelectedParticleMode(const Storm::ParticleSelectionMode mode, const Storm::SerializeSupportedFeatureLayout &supportedFeatures)
	{
		std::wstring result;
		result.reserve(32);

#define STORM_PARSE_CASE(Case, CaseNameResult, supported)	\
case Storm::ParticleSelectionMode::Case:					\
	result += STORM_TEXT(CaseNameResult);					\
	if (!supported)											\
	{														\
		result += STORM_TEXT(" (N-S)");						\
	}														\
	break

		switch (mode)
		{
			STORM_PARSE_CASE(Velocity,						"Velocity",							true);
			STORM_PARSE_CASE(Pressure,						"Pressure",							true);
			STORM_PARSE_CASE(Viscosity,						"Viscosity",						true);
			STORM_PARSE_CASE(AllOnParticle,					"All On Particle",					true);
			STORM_PARSE_CASE(Custom,						"Custom",							true);
			STORM_PARSE_CASE(Drag,							"Drag",								supportedFeatures._hasDragComponentforces);
			STORM_PARSE_CASE(DynamicPressure,				"DynamicQ",							supportedFeatures._hasDynamicPressureQForces);
			STORM_PARSE_CASE(NoStick,						"NoStick",							supportedFeatures._hasNoStickForces);
			STORM_PARSE_CASE(Coenda,						"Coenda",							supportedFeatures._hasCoendaForces);
			STORM_PARSE_CASE(IntermediaryDensityPressure,	"Intermediary Density Pressure",	supportedFeatures._hasIntermediaryDensityPressureForces);
			STORM_PARSE_CASE(IntermediaryVelocityPressure,	"Intermediary Velocity Pressure",	supportedFeatures._hasIntermediaryVelocityPressureForces);
			STORM_PARSE_CASE(TotalEngineForce,				"All On System (Engine)",			supportedFeatures._hasPSystemTotalEngineForce);
			STORM_PARSE_CASE(Normal,						"Normal",							supportedFeatures._hasNormals);
			STORM_PARSE_CASE(RbForce,						"Rb Total force",					supportedFeatures._hasPSystemGlobalForce);
			STORM_PARSE_CASE(AverageRbForce,				"Rb Average Total force",			supportedFeatures._hasPSystemGlobalForce);

		case Storm::ParticleSelectionMode::SelectionModeCount:
		default:
			Storm::throwException<Storm::Exception>("Unknown particle selection mode. Mode was " + Storm::toStdString(mode));
			break;
		}
#undef STORM_PARSE_CASE

		return result;
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

#define STORM_XMACRO_SELECTION_FLUIDS_MODE_BINDINGS(SelectionMode)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Velocity)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Drag)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, DynamicPressure)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, NoStick)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Coenda)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, IntermediaryDensityPressure)		\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, IntermediaryVelocityPressure)	\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AllOnParticle)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, TotalEngineForce)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Custom)							\

#define STORM_XMACRO_SELECTION_RB_MODE_BINDINGS(SelectionMode)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Velocity)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Pressure)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Viscosity)						\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Drag)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, DynamicPressure)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, NoStick)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Coenda)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, IntermediaryDensityPressure)		\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, IntermediaryVelocityPressure)	\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AllOnParticle)					\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, TotalEngineForce)				\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Custom)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, Normal)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, RbForce)							\
	STORM_XMACRO_ELEM_SELECTION_BINDING(SelectionMode, AverageRbForce)					\


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
	bool checkSkippedSelectionMode(const Storm::ParticleSelector &particleSelector, Storm::ParticleSelectionMode selectionMode)
	{
		const Storm::SerializeSupportedFeatureLayout &supportedFeatures = particleSelector.getSupportedFeaturesList();

#define STORM_UNSUPPORTED_CONDITION(variableName) particleSelector.shouldKeepSupportedFeatures() || variableName
		switch (selectionMode)
		{
		case Storm::ParticleSelectionMode::DynamicPressure:
		{
			if (STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasDynamicPressureQForces))
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
			return true;
		}

		case Storm::ParticleSelectionMode::IntermediaryDensityPressure:
		{
			if (STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasIntermediaryDensityPressureForces))
			{
				const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
				const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

				const Storm::SceneSimulationConfig &simulConfig = configMgr.getSceneSimulationConfig();
				return simulConfig._simulationMode == Storm::SimulationMode::DFSPH;
			}
			return true;
		}

		case Storm::ParticleSelectionMode::IntermediaryVelocityPressure:
		{
			if (STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasIntermediaryVelocityPressureForces))
			{
				const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
				const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

				const Storm::SceneSimulationConfig &simulConfig = configMgr.getSceneSimulationConfig();
				return simulConfig._simulationMode == Storm::SimulationMode::DFSPH;
			}
			return true;
		}

		case Storm::ParticleSelectionMode::NoStick:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasNoStickForces);

		case Storm::ParticleSelectionMode::Coenda:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasCoendaForces);

		case Storm::ParticleSelectionMode::Drag:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasDragComponentforces);

		case Storm::ParticleSelectionMode::Normal:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasNormals);

		case Storm::ParticleSelectionMode::TotalEngineForce:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasPSystemTotalEngineForce);

		case Storm::ParticleSelectionMode::AverageRbForce:
		case Storm::ParticleSelectionMode::RbForce:
			return STORM_UNSUPPORTED_CONDITION(supportedFeatures._hasPSystemGlobalForce);

		case Storm::ParticleSelectionMode::Custom:
			return particleSelector.hasCustomSelection();

		case Storm::ParticleSelectionMode::Pressure:
		case Storm::ParticleSelectionMode::Viscosity:
		case Storm::ParticleSelectionMode::Velocity:
		case Storm::ParticleSelectionMode::AllOnParticle:
		case Storm::ParticleSelectionMode::SelectionModeCount:
		default:
			return true;
		}
#undef STORM_UNSUPPORTED_CONDITION
	}

	template<class UsedSelectionMode>
	Storm::ParticleSelectionMode cycleSelectionMode(const Storm::ParticleSelector &particleSelector, Storm::ParticleSelectionMode currentSelectionMode)
	{
		do
		{
			const uint8_t cycledValue = (static_cast<uint8_t>(currentSelectionMode) + 1) % static_cast<uint8_t>(Storm::ParticleSelectionMode::SelectionModeCount);
			currentSelectionMode = retrieveSelectionMode<UsedSelectionMode>(cycledValue);

		} while (!checkSkippedSelectionMode<UsedSelectionMode>(particleSelector, currentSelectionMode));

		return currentSelectionMode;
	}

	template<bool left>
	void trim(std::span<char> &inOutSpan)
	{
		const std::size_t spanSize = inOutSpan.size();
		if (spanSize > 0)
		{
			std::size_t moving = 0;
			char* current;

			if constexpr (left)
			{
				current = &inOutSpan.front();
			}
			else
			{
				current = &inOutSpan.back();
			}

			do
			{
				if (*current == ' ' || *current == '\t' || *current == '\n')
				{
					++moving;
					if constexpr (left)
					{
						++current;
					}
					else
					{
						--current;
					}
				}
				else
				{
					break;
				}

			} while (moving < spanSize);

			if (moving > 0)
			{
				if constexpr (left)
				{
					inOutSpan = inOutSpan.subspan(moving);
				}
				else
				{
					inOutSpan = inOutSpan.subspan(0, spanSize - moving);
				}
			}
		}
	}

	Storm::CustomForceSelect parseForceSelect(std::span<char> &forceSelectSpan)
	{
		trim<true>(forceSelectSpan);
		trim<false>(forceSelectSpan);

		boost::to_lower(forceSelectSpan);

		const std::string_view forceSelectStr{ forceSelectSpan.data(), forceSelectSpan.size() };


#define STORM_RETURN_IF_PARSED(selectEnum, lowerName) if(forceSelectStr == lowerName) return Storm::CustomForceSelect::selectEnum

		STORM_RETURN_IF_PARSED(Pressure, "pressure");
		STORM_RETURN_IF_PARSED(Viscosity, "viscosity");
		STORM_RETURN_IF_PARSED(Drag, "drag");
		STORM_RETURN_IF_PARSED(Bernouilli, "bernouilli");
		STORM_RETURN_IF_PARSED(Bernouilli, "dynamicq");
		STORM_RETURN_IF_PARSED(Bernouilli, "dynamicpressure");
		STORM_RETURN_IF_PARSED(NoStick, "nostick");
		STORM_RETURN_IF_PARSED(Coenda, "coenda");

#undef STORM_RETURN_IF_PARSED

		Storm::throwException<Storm::Exception>(forceSelectStr + " cannot be parsed into a CustomForceSelect enumeration.");
	}

	template<class Logger>
	void logContribution(Logger &logger, const std::string_view name, const Storm::Vector3 &force, const Storm::Vector3 &normalizedVec)
	{
		logger << "\n" << name << " : ";
		const float contribution = force.dot(normalizedVec);
		const Storm::Vector3 colinearContrib = normalizedVec * contribution;
		const Storm::Vector3 perpDisorient = force - colinearContrib;
		logger << contribution << " N. Colinear contrib : " << colinearContrib << ". Disorient : " << perpDisorient << " (Norm " << perpDisorient.norm() << ")";
	}
}


Storm::ParticleSelector::ParticleSelector() :
	_currentParticleSelectionMode{ Storm::ParticleSelectionMode::Velocity },
	_selectedParticleData{ std::make_unique<Storm::SelectedParticleData>() },
	_supportedFeatures{ nullptr },
	_keepUnsupported{ false }
{
	_selectedParticleData->_selectedParticle = std::make_pair(dummySelectedParticleIndex(), 0);
	this->clearCustomSelection();

	this->clearRbTotalForce();
}

Storm::ParticleSelector::~ParticleSelector() = default;

void Storm::ParticleSelector::initialize(const bool isInReplayMode)
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();

	_keepUnsupported = configMgr.getGeneralDebugConfig()._keepUnsupported;

	if (isInReplayMode)
	{
		const Storm::ISerializerManager &serializerMgr = singletonHolder.getSingleton<Storm::ISerializerManager>();
		_supportedFeatures = serializerMgr.getRecordSupportedFeature();
	}
	else
	{
		// The default one.
		_supportedFeatures = std::make_shared<Storm::SerializeSupportedFeatureLayout>();
	}

	_selectionModeStr = parseSelectedParticleMode(_currentParticleSelectionMode, *_supportedFeatures);

	_fields = std::make_unique<Storm::UIFieldContainer>();

	(*_fields)
		.bindField(STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME, _selectionModeStr)
		;
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
	_selectionModeStr = parseSelectedParticleMode(newMode, *_supportedFeatures);
	_currentParticleSelectionMode = newMode;

	_fields->pushField(STORM_SELECTED_PARTICLE_DISPLAY_MODE_FIELD_NAME);
}

void Storm::ParticleSelector::cycleParticleSelectionDisplayMode()
{
	Storm::ParticleSelectionMode newMode;
	if (_selectedParticleData->_hasRbTotalForce)
	{
		newMode = cycleSelectionMode<RbParticleSelectionMode>(*this, _currentParticleSelectionMode);
	}
	else
	{
		newMode = cycleSelectionMode<FluidParticleSelectionMode>(*this, _currentParticleSelectionMode);
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

void Storm::ParticleSelector::setSelectedParticleCoendaForce(const Storm::Vector3 &coendaForce)
{
	_selectedParticleData->_coendaForce = coendaForce;
}

void Storm::ParticleSelector::setSelectedParticlePressureDensityIntermediaryForce(const Storm::Vector3 &intermediaryPressureForce)
{
	_selectedParticleData->_intermediaryDensityPressureForce = intermediaryPressureForce;
}

void Storm::ParticleSelector::setSelectedParticlePressureVelocityIntermediaryForce(const Storm::Vector3 &intermediaryPressureForce)
{
	_selectedParticleData->_intermediaryVelocityPressureForce = intermediaryPressureForce;
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
	case Storm::ParticleSelectionMode::Velocity:						return _selectedParticleData->_velocity;
	case Storm::ParticleSelectionMode::Pressure:						return _selectedParticleData->_pressureForce;
	case Storm::ParticleSelectionMode::Viscosity:						return _selectedParticleData->_viscosityForce;
	case Storm::ParticleSelectionMode::Drag:							return _selectedParticleData->_dragForce;
	case Storm::ParticleSelectionMode::DynamicPressure:					return _selectedParticleData->_dynamicPressureForce;
	case Storm::ParticleSelectionMode::NoStick:							return _selectedParticleData->_noStickForce;
	case Storm::ParticleSelectionMode::Coenda:							return _selectedParticleData->_coendaForce;
	case Storm::ParticleSelectionMode::IntermediaryDensityPressure:		return _selectedParticleData->_intermediaryDensityPressureForce;
	case Storm::ParticleSelectionMode::IntermediaryVelocityPressure:	return _selectedParticleData->_intermediaryVelocityPressureForce;
	case Storm::ParticleSelectionMode::AllOnParticle:					return _selectedParticleData->_externalSumForces;
	case Storm::ParticleSelectionMode::TotalEngineForce:				return _selectedParticleData->_totalEngineForce;
	case Storm::ParticleSelectionMode::Custom:							return _selectedParticleData->_customCached;
	case Storm::ParticleSelectionMode::Normal:							return _selectedParticleData->_rbNormals;
	case Storm::ParticleSelectionMode::RbForce:							return _selectedParticleData->_totalForcesOnRb;
	case Storm::ParticleSelectionMode::AverageRbForce:					return _selectedParticleData->_averageForcesOnRb.getAverage();

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

	case Storm::ParticleSelectionMode::Custom:
	case Storm::ParticleSelectionMode::Normal:
	case Storm::ParticleSelectionMode::Velocity:
	case Storm::ParticleSelectionMode::Pressure:
	case Storm::ParticleSelectionMode::Viscosity:
	case Storm::ParticleSelectionMode::Drag:
	case Storm::ParticleSelectionMode::DynamicPressure:
	case Storm::ParticleSelectionMode::NoStick:
	case Storm::ParticleSelectionMode::Coenda:
	case Storm::ParticleSelectionMode::IntermediaryDensityPressure:
	case Storm::ParticleSelectionMode::IntermediaryVelocityPressure:
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

const Storm::SerializeSupportedFeatureLayout& Storm::ParticleSelector::getSupportedFeaturesList() const noexcept
{
	return *_supportedFeatures;
}

bool Storm::ParticleSelector::shouldKeepSupportedFeatures() const noexcept
{
	return _keepUnsupported;
}


bool Storm::ParticleSelector::customShouldRefresh() const noexcept
{
	return _currentParticleSelectionMode == Storm::ParticleSelectionMode::Custom && this->hasSelectedParticle();
}

bool Storm::ParticleSelector::clearCustomSelection()
{
	if (_selectedParticleData->_endCustomForceSelected != _selectedParticleData->_customForceSelected)
	{
		_selectedParticleData->_endCustomForceSelected = _selectedParticleData->_customForceSelected;
		return this->customShouldRefresh();
	}

	return false;
}

bool Storm::ParticleSelector::setCustomSelection(std::string &&customSelectionCSL)
{
	std::vector<std::string_view> splitted;
	splitted.reserve(static_cast<std::size_t>(Storm::CustomForceSelect::Count));

	Storm::StringAlgo::split(splitted, customSelectionCSL, Storm::StringAlgo::makeSplitPredicate(','));

	Storm::SelectedParticleData &data = *_selectedParticleData;
	std::set<Storm::CustomForceSelect> uniqueSelect;

	for (std::string_view &splitElem : splitted)
	{
		// This comes from a true std::string, therefore this is modifyable memory.
		uniqueSelect.emplace(parseForceSelect(std::span<char>{ const_cast<char*>(splitElem.data()), splitElem.size() }));
	}

	if (uniqueSelect != std::set<Storm::CustomForceSelect>{ data._customForceSelected, data._endCustomForceSelected })
	{
		this->clearCustomSelection();

		if (!uniqueSelect.empty())
		{
			for (const Storm::CustomForceSelect selection : uniqueSelect)
			{
				*data._endCustomForceSelected = selection;
				++data._endCustomForceSelected;
			}

			this->computeCustomSelection();
		}

		return this->customShouldRefresh();
	}

	return false;
}

bool Storm::ParticleSelector::hasCustomSelection() const noexcept
{
	return _selectedParticleData->_endCustomForceSelected != _selectedParticleData->_customForceSelected;
}

void Storm::ParticleSelector::computeCustomSelection()
{
	Storm::SelectedParticleData &data = *_selectedParticleData;
	data._customCached.setZero();

	for (const Storm::CustomForceSelect* iter = _selectedParticleData->_customForceSelected; iter != data._endCustomForceSelected; ++iter)
	{
		switch (*iter)
		{
		case Storm::CustomForceSelect::Pressure:	data._customCached += data._pressureForce; break;
		case Storm::CustomForceSelect::Viscosity:	data._customCached += data._viscosityForce; break;
		case Storm::CustomForceSelect::Drag:		data._customCached += data._dragForce; break;
		case Storm::CustomForceSelect::Bernouilli:	data._customCached += data._dynamicPressureForce; break;
		case Storm::CustomForceSelect::NoStick:		data._customCached += data._noStickForce; break;
		case Storm::CustomForceSelect::Coenda:		data._customCached += data._coendaForce; break;

		case Storm::CustomForceSelect::Count:
		default:
			__assume(false);
		}
	}
}

void Storm::ParticleSelector::logForceComponents() const
{
	const auto &selectedParticleDataRef = *_selectedParticleData;

	std::string customStr;
	if (this->hasCustomSelection())
	{
		const std::string customForceStr = Storm::toStdString(selectedParticleDataRef._customCached);
		customStr.reserve(customForceStr.size() + 32);

		customStr += "\nCustom: ";
		customStr += customForceStr;
		customStr += ". Norm: ";
		customStr += Storm::toStdString(selectedParticleDataRef._customCached.norm());
		customStr += " N.\n";
	}

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
		STORM_APPEND_DATA_TO_STREAM("Coenda", _coendaForce, "N", _supportedFeatures->_hasCoendaForces)
		STORM_APPEND_DATA_TO_STREAM("Intermediary Density Pressure", _intermediaryDensityPressureForce, "N", _supportedFeatures->_hasIntermediaryDensityPressureForces)
		STORM_APPEND_DATA_TO_STREAM("Intermediary Velocity Pressure", _intermediaryVelocityPressureForce, "N", _supportedFeatures->_hasIntermediaryVelocityPressureForces)
		STORM_APPEND_DATA_TO_STREAM("Sum", _externalSumForces, "N", true)
		STORM_APPEND_DATA_TO_STREAM("Total system force", _totalEngineForce, "N", _supportedFeatures->_hasPSystemTotalEngineForce) <<
		customStr <<
		rbSpecificInfosStr
		;

#undef STORM_APPEND_RB_VECTOR_DATA
#undef STORM_APPEND_DATA_TO_STREAM
#undef STORM_SUPPORT_STR
}

void Storm::ParticleSelector::logForceComponentsContributionToVector(const Storm::Vector3 &vec) const
{
	if (vec.isZero())
	{
		Storm::throwException<Storm::Exception>("Vector shouldn't be null!");
	}

	const Storm::Vector3 vecNormalized = vec.normalized();
	const auto &dataRef = *_selectedParticleData;
	
	auto alwaysLogger = LOG_ALWAYS;
	
	alwaysLogger <<
		"Contribution to " << vec << " (normalized is " << vecNormalized << ")";

	logContribution(alwaysLogger, "Pressure", dataRef._pressureForce, vecNormalized);
	logContribution(alwaysLogger, "Viscosity", dataRef._viscosityForce, vecNormalized);

	if (_supportedFeatures->_hasDragComponentforces)
	{
		logContribution(alwaysLogger, "Drag", dataRef._dragForce, vecNormalized);
	}
	if (_supportedFeatures->_hasDynamicPressureQForces)
	{
		logContribution(alwaysLogger, "DynamicPressure", dataRef._dynamicPressureForce, vecNormalized);
	}
	if (_supportedFeatures->_hasNoStickForces)
	{
		logContribution(alwaysLogger, "NoStick", dataRef._noStickForce, vecNormalized);
	}
	if (_supportedFeatures->_hasCoendaForces)
	{
		logContribution(alwaysLogger, "Coenda", dataRef._coendaForce, vecNormalized);
	}
	if (_supportedFeatures->_hasIntermediaryDensityPressureForces)
	{
		logContribution(alwaysLogger, "Intermediary Density Pressure", dataRef._intermediaryDensityPressureForce, vecNormalized);
	}
	if (_supportedFeatures->_hasIntermediaryVelocityPressureForces)
	{
		logContribution(alwaysLogger, "Intermediary Velocity Pressure", dataRef._intermediaryVelocityPressureForce, vecNormalized);
	}

	logContribution(alwaysLogger, "Sum", dataRef._externalSumForces, vecNormalized);
}

void Storm::ParticleSelector::logForceComponentsContributionToVelocity() const
{
	this->logForceComponentsContributionToVector(_selectedParticleData->_velocity);
}
