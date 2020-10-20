#pragma once

namespace Storm
{
	struct SerializeRecordParticleSystemData
	{
		uint32_t _systemId;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;
		std::vector<Storm::Vector3> _pressureComponentforces;
		std::vector<Storm::Vector3> _viscosityComponentforces;
	};
}
