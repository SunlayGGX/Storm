#pragma once

#include "SceneFluidDefaultCustomConfig.h"


namespace Storm
{
	struct SceneFluidCustomDFSPHConfig : public Storm::SceneFluidDefaultCustomConfig
	{
	public:
		SceneFluidCustomDFSPHConfig();

	public:
		std::size_t _neighborThresholdDensity;
		bool _enableThresholdDensity;
		bool _useFixRotation;
		bool _enableDensitySolve;

		float _kPressurePredictedCoeff; // This one is for testing... This is the multiplication factor for the kDFSPH and co. It doesn't exist inside the true formula (therefore should be 1.f in the real DFSPH method)
		
		bool _useBernoulliPrinciple;
	};
}
