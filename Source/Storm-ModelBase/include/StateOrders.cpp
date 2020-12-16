#include "SimulationState.h"

#include "StateSavingOrders.h"
#include "StateLoadingOrders.h"

#include "SystemSimulationStateObject.h"


Storm::SimulationState::~SimulationState() = default;

Storm::StateSavingOrders::StateSavingOrders(StateSavingOrders &&) = default;
Storm::StateSavingOrders::~StateSavingOrders() = default;

Storm::StateLoadingOrders::~StateLoadingOrders() = default;
