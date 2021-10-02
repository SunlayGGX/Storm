#include "KernelHandler.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IGraphicsManager.h"

#include "SceneSimulationConfig.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#define STORM_KERNEL_VALUE_FIELD_NAME "Kernel"


namespace
{
	float computeNewKernelValue(const Storm::SceneSimulationConfig &sceneSimulationConfig, const float dynamicIncrementCoeff)
	{
		return sceneSimulationConfig._particleRadius * (sceneSimulationConfig._kernelCoefficient + dynamicIncrementCoeff);
	}
}


Storm::KernelHandler::KernelHandler() :
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() }
{

}

Storm::KernelHandler::~KernelHandler() = default;

void Storm::KernelHandler::initialize()
{
	const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();

	const Storm::SceneSimulationConfig &sceneSimulationConfig = singletonHolder.getSingleton<Storm::IConfigManager>().getSceneSimulationConfig();

	_currentKernelValue = computeNewKernelValue(sceneSimulationConfig, 0.f);
	_shouldIncrementKernel = sceneSimulationConfig._kernelIncrementSpeedInSeconds != -1.f && sceneSimulationConfig._maxKernelIncrementCoeff != 0.f;

	(*_uiFields)
		.bindField(STORM_KERNEL_VALUE_FIELD_NAME, _currentKernelValue)
		;

	singletonHolder.getSingleton<Storm::IGraphicsManager>().setKernelAreaRadius(_currentKernelValue);
}

void Storm::KernelHandler::update(const float currentPhysicsTimeInSeconds)
{
	if (_shouldIncrementKernel)
	{
		const Storm::SceneSimulationConfig &sceneSimulationConfig = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getSceneSimulationConfig();

		if (currentPhysicsTimeInSeconds >= sceneSimulationConfig._kernelIncrementSpeedInSeconds)
		{
			this->setKernelValue(
				computeNewKernelValue(sceneSimulationConfig, sceneSimulationConfig._maxKernelIncrementCoeff)
			);
		}
		else
		{
			const float newKernelValue = computeNewKernelValue(
				sceneSimulationConfig,
				sceneSimulationConfig._maxKernelIncrementCoeff * currentPhysicsTimeInSeconds / sceneSimulationConfig._kernelIncrementSpeedInSeconds
			);

			this->setKernelValue(newKernelValue);
		}
	}
}

void Storm::KernelHandler::setKernelValue(const float newValue)
{
	Storm::updateField(*_uiFields, STORM_KERNEL_VALUE_FIELD_NAME, _currentKernelValue, newValue);
	Storm::SingletonHolder::instance().getSingleton<Storm::IGraphicsManager>().setKernelAreaRadius(_currentKernelValue);
}
