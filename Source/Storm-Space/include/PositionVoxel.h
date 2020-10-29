#pragma once


namespace Storm
{
	class PositionVoxel
	{
	public:
		PositionVoxel();
		PositionVoxel(Storm::PositionVoxel &&other);
		PositionVoxel(const Storm::PositionVoxel &other);
		~PositionVoxel();

	public:
		void clear();
		void addData(const Storm::Vector3 &pos);

		__forceinline const std::vector<Storm::Vector3>& getData() const noexcept { return _positionData; }

	private:
		std::vector<Storm::Vector3> _positionData;
	};
}
