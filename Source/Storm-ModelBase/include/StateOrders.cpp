#include "SimulationState.h"

#include "StateSavingOrders.h"
#include "StateLoadingOrders.h"

#include "SystemSimulationStateObject.h"


Storm::SimulationState::~SimulationState() = default;

Storm::StateSavingOrders::StateSavingOrders(StateSavingOrders &&) = default;
Storm::StateSavingOrders::~StateSavingOrders() = default;

Storm::StateLoadingOrders::~StateLoadingOrders() = default;


Storm::StateSavingOrders::StateSavingOrders() :
	_simulationState{ std::make_unique<Storm::SimulationState>() }
{}

Storm::StateLoadingOrders::StateLoadingOrders() :
	_simulationState{ std::make_unique<Storm::SimulationState>() }
{}
