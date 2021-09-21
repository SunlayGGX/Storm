#pragma once

#include "SceneFluidDefaultCustomConfig.h"


namespace Storm
{
	struct SceneFluidCustomPCISPHConfig : public Storm::SceneFluidDefaultCustomConfig
	{
	public:
		SceneFluidCustomPCISPHConfig();

	public:
		unsigned int _maxPredictIteration;
		unsigned int _minPredictIteration;
		float _maxError;
	};
}
