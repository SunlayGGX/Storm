#pragma once

namespace Storm
{
	struct SerializeRecordParticleSystemData
	{
		uint32_t _systemId;
		float _wantedDensity;
		Storm::Vector3 _pSystemPosition = Storm::Vector3::Zero(); // Only valid for dynamic Rb
		Storm::Vector3 _pSystemGlobalForce = Storm::Vector3::Zero(); // Only valid for dynamic Rb
		Storm::Vector3 _pSystemTotalEngineForce;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;
		std::vector<float> _densities;
		std::vector<float> _pressures;
		std::vector<float> _volumes;
		std::vector<Storm::Vector3> _normals;
		std::vector<Storm::Vector3> _pressureComponentforces;
		std::vector<Storm::Vector3> _viscosityComponentforces;
		std::vector<Storm::Vector3> _dragComponentforces;
		std::vector<Storm::Vector3> _dynamicPressureQForces;
		std::vector<Storm::Vector3> _noStickForces;
		std::vector<Storm::Vector3> _coandaForces;
		std::vector<Storm::Vector3> _intermediaryPressureDensityComponentForces;
		std::vector<Storm::Vector3> _intermediaryPressureVelocityComponentForces;
		std::vector<Storm::Vector3> _blowerForces;
	};
}
