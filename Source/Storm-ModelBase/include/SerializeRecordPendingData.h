#pragma once


namespace Storm
{
	struct SerializeRecordElementsData
	{
		uint32_t _systemId;
		std::vector<Storm::Vector3> _positions;
		std::vector<Storm::Vector3> _velocities;
		std::vector<Storm::Vector3> _forces;
	};

	struct SerializeRecordPendingData
	{
	public:
		float _physicsTime;
		std::vector<Storm::SerializeRecordElementsData> _elements;
	};
}
