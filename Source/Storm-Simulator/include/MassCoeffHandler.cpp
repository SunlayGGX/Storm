#include "MassCoeffHandler.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "ITimeManager.h"

#include "SceneFluidConfig.h"

#include "ThreadingSafety.h"



Storm::MassCoeffHandler::MassCoeffHandler()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
	const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
	const Storm::SceneFluidConfig &sceneFluidConfig = configMgr.getSceneFluidConfig();

	const Storm::MassCoeffConfig &massCoeffControlConfig = sceneFluidConfig._massCoeffControlConfig;

	_vary = !std::isnan(massCoeffControlConfig._startReducedMassCoeff);
	if (_vary)
	{
		_reducedMassCoeff = massCoeffControlConfig._startReducedMassCoeff;
	}
	else
	{
		_reducedMassCoeff = massCoeffControlConfig._reducedMassCoefficient;
	}
}

void Storm::MassCoeffHandler::update()
{
	if (_vary)
	{
		assert(Storm::isSimulationThread() && "This method should only be executed on simulation thread!");

		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
		const Storm::SceneFluidConfig &sceneFluidConfig = configMgr.getSceneFluidConfig();
		const Storm::MassCoeffConfig &massCoeffControlConfig = sceneFluidConfig._massCoeffControlConfig;

		const Storm::ITimeManager &timeMgr = singletonHolder.getSingleton<Storm::ITimeManager>();
		float currentTime = timeMgr.getCurrentPhysicsElapsedTime();

		if (currentTime < massCoeffControlConfig._fadeInTimeSec)
		{
			const float alphaCoeff = currentTime / massCoeffControlConfig._fadeInTimeSec;

			this->setReducedMassCoeff(
				std::lerp(massCoeffControlConfig._startReducedMassCoeff, massCoeffControlConfig._reducedMassCoefficient, alphaCoeff)
			);
		}
		else
		{
			this->setReducedMassCoeff(massCoeffControlConfig._reducedMassCoefficient);
			_vary = false;
		}
	}
}

Storm::CallbackIdType Storm::MassCoeffHandler::bindListenerToReducedMassCoefficientChanged(OnReducedMassCoefficientChangedDelegate &&listenerCallback)
{
	assert(Storm::isSimulationThread() && "This method should only be executed on simulation thread!");
	return _onReducedMassCoeffChangedEvent.add(std::move(listenerCallback));
}

void Storm::MassCoeffHandler::unbindListenerToReducedMassCoefficientChanged(const Storm::CallbackIdType listenerId)
{
	assert(Storm::isSimulationThread() && "This method should only be executed on simulation thread!");
	_onReducedMassCoeffChangedEvent.remove(listenerId);
}

float Storm::MassCoeffHandler::getReducedMassCoeff() const noexcept
{
	assert(Storm::isSimulationThread() && "This method should only be executed on simulation thread!");
	return _reducedMassCoeff;
}

void Storm::MassCoeffHandler::setReducedMassCoeff(const float newValue)
{
	assert(Storm::isSimulationThread() && "This method should only be executed on simulation thread!");
	
	if (_reducedMassCoeff != newValue)
	{
		_reducedMassCoeff = newValue;
		Storm::prettyCallMultiCallback(_onReducedMassCoeffChangedEvent, _reducedMassCoeff);
	}
}
