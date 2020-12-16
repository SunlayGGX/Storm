#include "StateReader.h"

#include "StateLoadingOrders.h"
#include "SimulationState.h"
#include "SystemSimulationStateObject.h"

#include "RunnerHelper.h"

#include "ThrowException.h"


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
				Storm::throwException<std::exception>("Loaded system " + std::to_string(pSystemState._id) + " velocities count mismatches forces count!");
			}
		}
	}

	void checkSettingsValidity(const Storm::StateLoadingOrders &inLoadingOrder)
	{
		const std::filesystem::path filePath{ inLoadingOrder._settings._filePath };
		if (!std::filesystem::is_regular_file(filePath))
		{
			Storm::throwException<std::exception>("'" + inLoadingOrder._settings._filePath + "' is not a file or doesn't exist");
		}
	}
}


void Storm::StateReader::execute(Storm::StateLoadingOrders &inOutLoadingOrder)
{
	STORM_NOT_IMPLEMENTED;

	checkSettingsValidity(inOutLoadingOrder);

	// TODO : Read

	checkValidity(*inOutLoadingOrder._simulationState);
	applySettings(inOutLoadingOrder);
}
