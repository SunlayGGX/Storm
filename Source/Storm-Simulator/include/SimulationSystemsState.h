#pragma once


namespace Storm
{
	enum class SimulationSystemsState : uint8_t
	{
		Normal, // The simulation systems are running normally.
		Stationnary, // The simulation has attained a state where nearly nothing is moving (even though the simulation is running, systems are idles).
		Unstable, // The simulation is unstable.
	};
}
