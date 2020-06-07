#pragma once


namespace Storm
{
	struct FluidBlockData
	{
		Storm::Vector3 _minBox;
		Storm::Vector3 _maxBox;
	};

	struct FluidData
	{
	public:
		FluidData();

	public:
		std::vector<Storm::FluidBlockData> _fluidGenData;
	};
}
