#pragma once

#include "SceneFluidDefaultCustomConfig.h"


namespace Storm
{
	struct SceneFluidCustomIISPHConfig : public Storm::SceneFluidDefaultCustomConfig
	{
	public:
		SceneFluidCustomIISPHConfig();

	public:
		unsigned int _maxPredictIteration;
		unsigned int _minPredictIteration;
		float _maxError;
	};
}
