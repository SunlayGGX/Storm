#pragma once


namespace Storm
{
	// What should we display.
	// Do not set the value manually, let the compiler do its job.
	enum class ParticleSelectionMode : uint8_t
	{
		Pressure,
		Viscosity,
		ViscosityAndPressure,

		RbForce,

		// Should remain the last
		SelectionModeCount
	};
}
