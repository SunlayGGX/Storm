#pragma once


namespace Storm
{
	class VoxelGrid
	{
	public:
		VoxelGrid(const Storm::Vector3 &upCorner, const Storm::Vector3 &downCorner, float voxelEdgeLength);

	public:
		void getVoxelsDataAtPosition(const std::vector<std::size_t>* &outContainingVoxelPtr, std::vector<const std::vector<std::size_t>*> &outNeighborData, const Storm::Vector3 &particlePosition) const;

		void fill(const std::vector<Storm::Vector3> &particlePositions);
		void clear();

	private:
		
	};
}
