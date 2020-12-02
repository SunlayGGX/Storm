#pragma once

namespace Storm
{
	struct SerializeRecordParticleSystemData
	{
		uint32_t _systemId;
		Storm::Vector3 _pSystemPosition = Storm::Vector3::Zero(); // Only valid for dynamic Rb
		Storm::Vector3 _pSystemGlobalForce = Storm::Vector3::Zero(); // Only valid for dynamic Rb
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;
		std::vector<Storm::Vector3> _pressureComponentforces;
		std::vector<Storm::Vector3> _viscosityComponentforces;
	};
}
