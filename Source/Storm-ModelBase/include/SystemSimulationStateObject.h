#pragma once


namespace Storm
{
	struct SystemSimulationStateObject
	{
	public:
		uint32_t _id;
		uint8_t _isFluid;
		uint8_t _isStatic;

		Storm::Vector3 _globalPosition;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;

		std::vector<float> _densities;
		std::vector<float> _pressures;
		std::vector<float> _masses;
		std::vector<float> _volumes;
		std::vector<Storm::Vector3> _normals;
	};
}
