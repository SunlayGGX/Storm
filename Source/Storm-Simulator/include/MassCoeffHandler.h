#pragma once

#include "MultiCallback.h"


namespace Storm
{
	using OnReducedMassCoefficientChangedDelegate = std::function<void(float)>;

	class MassCoeffHandler
	{
	public:
		MassCoeffHandler();

	public:
		void update();

	public:
		Storm::CallbackIdType bindListenerToReducedMassCoefficientChanged(OnReducedMassCoefficientChangedDelegate &&listenerCallback);
		void unbindListenerToReducedMassCoefficientChanged(const Storm::CallbackIdType listenerId);

	public:
		float getReducedMassCoeff() const noexcept;
		void setReducedMassCoeff(const float newValue);

	private:
		float _reducedMassCoeff;
		bool _vary;
		Storm::MultiCallback<OnReducedMassCoefficientChangedDelegate> _onReducedMassCoeffChangedEvent;
	};
}
