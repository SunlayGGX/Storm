#pragma once


namespace Storm
{
	enum class CustomForceSelect : uint8_t
	{
		Pressure,
		Viscosity,
		Drag,
		Bernouilli,
		NoStick,
		Coenda,

		// All Compositions
		AllPressure,
		AllViscosity,
		AllDrag,
		AllBernouilli,
		AllNoStick,
		AllCoenda,

		// Do not use like the others. Should remain the last one.
		Count
	};
}
