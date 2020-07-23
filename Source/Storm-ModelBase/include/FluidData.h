#pragma once


namespace Storm
{
	struct FluidBlockData
	{
	public:
		FluidBlockData();

	public:
		Storm::Vector3 _firstPoint;
		Storm::Vector3 _secondPoint;
	};

	struct FluidData
	{
	public:
		FluidData();

	public:
		unsigned int _fluidId;
		std::vector<Storm::FluidBlockData> _fluidGenData;
		float _density;
		float _dynamicViscosity;
		float _cinematicViscosity;
		float _soundSpeed;
		float _kPressureStiffnessCoeff; // k1 in the pressure state equation of Taits
		float _kPressureExponentCoeff; // k2 in the pressure state equation of Taits
	};
}
