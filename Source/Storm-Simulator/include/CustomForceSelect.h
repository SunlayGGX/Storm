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
		Coanda,

		// All Compositions
		AllPressure,
		AllViscosity,
		AllDrag,
		AllBernouilli,
		AllNoStick,
		AllCoanda,

		// Do not use like the others. Should remain the last one.
		Count
	};
}
