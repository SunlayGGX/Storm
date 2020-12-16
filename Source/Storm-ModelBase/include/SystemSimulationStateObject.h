#pragma once


namespace Storm
{
	struct SystemSimulationStateObject
	{
	public:
		unsigned int _id;
		bool _isFluid;
		bool _isWall;

		Storm::Vector3 _globalPosition;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;

		std::vector<float> _densities;
		std::vector<float> _pressures;
		std::vector<float> _masses;
		std::vector<float> _volumes;
	};
}
