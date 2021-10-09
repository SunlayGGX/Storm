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

		// Do not use like the others. Should remain the last one.
		Count
	};
}
