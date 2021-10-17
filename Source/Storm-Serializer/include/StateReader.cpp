#include "StateReader.h"

#include "StateLoadingOrders.h"
#include "SimulationState.h"
#include "SystemSimulationStateObject.h"

#include "StateFileHeader.h"

#include "SerializePackage.h"
#include "SerializePackageCreationModality.h"

#include "RunnerHelper.h"


namespace
{
	template<class Type>
	auto resetValue(Type &val) -> decltype(val = static_cast<Type>(0), void())
	{
		val = static_cast<Type>(0);
	}

	void resetValue(Storm::Vector3 &val)
	{
		val.setZero();
	}

	template<class Func, class ... Funcs>
	auto& extractFirstArray(Storm::SystemSimulationStateObject &pSystemState, const Func &getter, Funcs &...)
	{
		return getter(pSystemState);
	}

	template<class Func>
	auto& extractFirstArray(Storm::SystemSimulationStateObject &pSystemState, const Func &getter)
	{
		return getter(pSystemState);
	}

	template<class Func, class ... Funcs>
	void resetStateImpl(Storm::SystemSimulationStateObject &pSystemState, const std::size_t index, const Func &getter, Funcs &... remainingGetters)
	{
		resetStateImpl(pSystemState, index, getter);
		resetStateImpl(pSystemState, index, remainingGetters...);
	}

	template<class Func, class ... Funcs>
	void resetStateImpl(Storm::SystemSimulationStateObject &pSystemState, const std::size_t index, const Func &getter)
	{
		using GetterReturnType = decltype(getter(pSystemState));
		STORM_STATIC_ASSERT(std::is_lvalue_reference_v<GetterReturnType> && !std::is_const_v<GetterReturnType>, "Getter func should return a non const reference!");

		resetValue(getter(pSystemState)[index]);
	}

	template<class ... Funcs>
	void resetState(Storm::SystemSimulationStateObject &pSystemState, Funcs &... gettersToResettedArray)
	{
		Storm::runParallel(extractFirstArray(pSystemState, gettersToResettedArray...), [&](const Storm::Vector3 &, const std::size_t index)
		{
			resetStateImpl(pSystemState, index, gettersToResettedArray...);
		});
	}

	void applySettings(Storm::StateLoadingOrders &inOutLoadingOrder)
	{
#define STORM_MAKE_SystemSimulationStateObject_ARRAY_EXTRACTOR_LAMBDA(containerToExtract) \
	[](Storm::SystemSimulationStateObject &pSystemState) -> std::add_lvalue_reference_t<decltype(pSystemState.containerToExtract)> { return pSystemState.containerToExtract; }

		const auto velocitiesArrayExtractorLambda = STORM_MAKE_SystemSimulationStateObject_ARRAY_EXTRACTOR_LAMBDA(_velocities);
		const auto forcesArrayExtractorLambda = STORM_MAKE_SystemSimulationStateObject_ARRAY_EXTRACTOR_LAMBDA(_forces);

#undef STORM_MAKE_SystemSimulationStateObject_ARRAY_EXTRACTOR_LAMBDA


		const Storm::StateLoadingOrders::LoadingSettings &loadSetting = inOutLoadingOrder._settings;
		Storm::SimulationState &simulationState = *inOutLoadingOrder._simulationState;

		if (!loadSetting._loadPhysicsTime)
		{
			simulationState._currentPhysicsTime = 0.f;
		}

		const bool velocityReset = !loadSetting._loadVelocities;
		const bool forceReset = !loadSetting._loadForces;

		if (velocityReset && forceReset)
		{
			for (Storm::SystemSimulationStateObject &pSystemState : simulationState._pSystemStates)
			{
				resetState(pSystemState, velocitiesArrayExtractorLambda, forcesArrayExtractorLambda);
			}
		}
		else if (velocityReset)
		{
			for (Storm::SystemSimulationStateObject &pSystemState : simulationState._pSystemStates)
			{
				resetState(pSystemState, velocitiesArrayExtractorLambda);
			}
		}
		else /*if (forceReset)*/
		{
			for (Storm::SystemSimulationStateObject &pSystemState : simulationState._pSystemStates)
			{
				resetState(pSystemState, forcesArrayExtractorLambda);
			}
		}
	}

	void checkValidity(const Storm::SimulationState &simulationState)
	{
		for (const Storm::SystemSimulationStateObject &pSystemState : simulationState._pSystemStates)
		{
			const std::size_t velocitiesCount = pSystemState._velocities.size();
			const std::size_t forcesCount = pSystemState._forces.size();

			if (velocitiesCount != forcesCount)
			{
				Storm::throwException<Storm::Exception>("Loaded system " + std::to_string(pSystemState._id) + " velocities count mismatches forces count!");
			}
		}
	}

	void validateOrders(const Storm::StateLoadingOrders &loadingOrder)
	{
		if (loadingOrder._simulationState == nullptr)
		{
			Storm::throwException<Storm::Exception>("Simulation state object is null!");
		}
	}

	void checkSettingsValidity(const Storm::StateLoadingOrders &inLoadingOrder)
	{
		const Storm::StateLoadingOrders::LoadingSettings &loadSetting = inLoadingOrder._settings;
		if (!std::filesystem::is_regular_file(loadSetting._filePath))
		{
			Storm::throwException<Storm::Exception>("'" + Storm::toStdString(loadSetting._filePath) + "' is not a file or doesn't exist");
		}
	}

	class StateReaderImpl : public Storm::StateFileHeader
	{
	public:
		StateReaderImpl(Storm::StateLoadingOrders &loadingOrder) :
			_loadingOrder{ loadingOrder }
		{}

	public:
		void serialize(Storm::SerializePackage &package)
		{
			Storm::StateFileHeader::serialize(package);

			if (_stateFileVersion < Storm::Version{ 1, 1, 0 })
			{
				this->serialize_v1_0_0(package);
			}
			else if (_stateFileVersion < Storm::Version{ 1, 2, 0 })
			{
				this->serialize_v1_1_0(package);
			}
			else
			{
				Storm::throwException<Storm::Exception>("Cannot read the current state because the version " + Storm::toStdString(_stateFileVersion) + " isn't handled ");
			}

			this->applyHeaderSettings();
		}

	private:
		void applyHeaderSettings()
		{
			_loadingOrder._simulationState->_configSceneName = std::move(_configFileNameUsed);
		}

	private:
		void serialize_v1_0_0(Storm::SerializePackage &package);
		void serialize_v1_1_0(Storm::SerializePackage &package);

	private:
		Storm::StateLoadingOrders &_loadingOrder;
	};

	void StateReaderImpl::serialize_v1_0_0(Storm::SerializePackage &package)
	{
		Storm::SimulationState &simulationState = *_loadingOrder._simulationState;

		package << simulationState._currentPhysicsTime;

		uint64_t pSystemCount = 0;
		package << pSystemCount;

		simulationState._pSystemStates.resize(pSystemCount);

		for (Storm::SystemSimulationStateObject &pState : simulationState._pSystemStates)
		{
			package <<
				pState._id <<
				pState._isFluid <<
				pState._isStatic <<
				pState._positions <<
				pState._velocities <<
				pState._forces
				;

			if (pState._isFluid)
			{
				package <<
					pState._densities <<
					pState._pressures <<
					pState._masses
					;
			}
			else
			{
				package <<
					pState._globalPosition <<
					pState._volumes
					;
			}
		}
	}

	void StateReaderImpl::serialize_v1_1_0(Storm::SerializePackage &package)
	{
		Storm::SimulationState &simulationState = *_loadingOrder._simulationState;

		package << simulationState._currentPhysicsTime;

		uint64_t pSystemCount = 0;
		package << pSystemCount;

		simulationState._pSystemStates.resize(pSystemCount);

		for (Storm::SystemSimulationStateObject &pState : simulationState._pSystemStates)
		{
			package <<
				pState._id <<
				pState._isFluid <<
				pState._isStatic <<
				pState._positions <<
				pState._velocities <<
				pState._forces
				;

			if (pState._isFluid)
			{
				package <<
					pState._densities <<
					pState._pressures <<
					pState._masses
					;
			}
			else
			{
				package <<
					pState._globalPosition <<
					pState._volumes <<
					pState._normals
					;
			}
		}
	}
}


void Storm::StateReader::execute(Storm::StateLoadingOrders &inOutLoadingOrder)
{
	checkSettingsValidity(inOutLoadingOrder);
	validateOrders(inOutLoadingOrder);

	Storm::SerializePackage package{ Storm::SerializePackageCreationModality::LoadingManual, Storm::toStdString(inOutLoadingOrder._settings._filePath) };
	{
		StateReaderImpl tmp{ inOutLoadingOrder };
		package << tmp;
	}

	checkValidity(*inOutLoadingOrder._simulationState);
	applySettings(inOutLoadingOrder);
}
