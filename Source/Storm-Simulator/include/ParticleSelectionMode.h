#pragma once


namespace Storm
{
	// What should we display.
	// Do not set the value manually, let the compiler do its job.
	enum class ParticleSelectionMode : uint8_t
	{
		Velocity,

		Pressure,
		Viscosity,
		Drag,
		DynamicPressure,
		NoStick,
		Coenda,
		IntermediaryPressure,
		AllOnParticle,
		TotalEngineForce,

		Custom,

		Normal,
		RbForce,
		AverageRbForce,

		// Should remain the last
		SelectionModeCount
	};
}
