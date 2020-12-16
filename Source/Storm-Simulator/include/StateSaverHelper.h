#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	struct SimulationState;

	class StateSaverHelper
	{
	public:
		static void saveIntoState(Storm::SimulationState &state, const Storm::ParticleSystemContainer &pSystemContainer);
	};
}
