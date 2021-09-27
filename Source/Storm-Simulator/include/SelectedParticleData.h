#pragma once

#include "Average.h"


namespace Storm
{
	struct SelectedParticleData
	{
		std::pair<unsigned int, std::size_t> _selectedParticle;
		Storm::Vector3 _velocity;

		Storm::Vector3 _pressureForce;
		Storm::Vector3 _viscosityForce;
		Storm::Vector3 _dragForce;
		Storm::Vector3 _dynamicPressureForce;
		Storm::Vector3 _noStickForce;
		Storm::Vector3 _externalSumForces;
		Storm::Vector3 _totalEngineForce;

		bool _hasRbTotalForce;
		Storm::Vector3 _rbPosition;
		Storm::Vector3 _rbNormals;
		Storm::Vector3 _totalForcesOnRb;

		Storm::Average<Storm::Vector3, Storm::MovingAverageTraits<32>> _averageForcesOnRb;
	};
}
