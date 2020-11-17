#include "KernelHandler.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralSimulationData.h"

#include "UIField.h"
#include "UIFieldContainer.h"

#define STORM_KERNEL_VALUE_FIELD_NAME "Kernel"


namespace
{
	float computeNewKernelValue(const Storm::GeneralSimulationData &generalSimulationConfigData, const float dynamicIncrementCoeff)
	{
		return generalSimulationConfigData._particleRadius * (generalSimulationConfigData._kernelCoefficient + dynamicIncrementCoeff);
	}
}


Storm::KernelHandler::KernelHandler() :
	_uiFields{ std::make_unique<Storm::UIFieldContainer>() }
{

}

Storm::KernelHandler::~KernelHandler() = default;

void Storm::KernelHandler::initialize()
{
	const Storm::GeneralSimulationData &generalSimulationConfigData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();

	_currentKernelValue = computeNewKernelValue(generalSimulationConfigData, 0.f);
	_shouldIncrementKernel = generalSimulationConfigData._kernelIncrementSpeedInSeconds != -1.f && generalSimulationConfigData._maxKernelIncrementCoeff != 0.f;

	(*_uiFields)
		.bindField(STORM_KERNEL_VALUE_FIELD_NAME, _currentKernelValue)
		;
}

void Storm::KernelHandler::update(const float currentPhysicsTimeInSeconds)
{
	if (_shouldIncrementKernel)
	{
		const Storm::GeneralSimulationData &generalSimulationConfigData = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>().getGeneralSimulationData();

		if (currentPhysicsTimeInSeconds >= generalSimulationConfigData._kernelIncrementSpeedInSeconds)
		{
			this->setKernelValue(
				computeNewKernelValue(generalSimulationConfigData, generalSimulationConfigData._maxKernelIncrementCoeff)
			);
		}
		else
		{
			const float newKernelValue = computeNewKernelValue(
				generalSimulationConfigData,
				generalSimulationConfigData._maxKernelIncrementCoeff * currentPhysicsTimeInSeconds / generalSimulationConfigData._kernelIncrementSpeedInSeconds
			);

			this->setKernelValue(newKernelValue);
		}
	}
}

void Storm::KernelHandler::setKernelValue(const float newValue)
{
	updateField(*_uiFields, STORM_KERNEL_VALUE_FIELD_NAME, _currentKernelValue, newValue);
}
