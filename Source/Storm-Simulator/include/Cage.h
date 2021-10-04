#pragma once

#include "ParticleSystemContainer.h"


namespace Storm
{
	struct SceneCageConfig;

	class Cage
	{
	public:
		Cage(const Storm::SceneCageConfig &sceneCageConfig);
		~Cage();

	public:
		void doEnclose(Storm::ParticleSystemContainer &pSystems) const;

	private:
		Storm::Vector3 _boxMin;
		Storm::Vector3 _boxMax;

		const bool _infiniteDomain;
		const Storm::Vector3 _velocityCoeffs;
	};
}
