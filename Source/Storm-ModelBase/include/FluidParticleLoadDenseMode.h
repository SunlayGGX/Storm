#pragma once


namespace Storm
{
	enum class FluidParticleLoadDenseMode
	{
		Normal,

		// SplishSplash normal way is overly complex and is kind of bugged.
		// But I need to implement like them to compare my engine with theirs (having the same particle count is important)...
		// Their algorithm is really overly complex for nothing so it is normal there is some bug inside.
		// Since it is harder to understand and less maintenable, I'll be using it just for the comparison, then use my normal way afterwards.
		// Those bugs are :
		// - Disymmetry when spawning
		// - Particle that doesn't fill the requested block (we lose a particle diameter on X and on Y axis). And making errors when the box is less or equal to the particle diameter...
		AsSplishSplash,
	};
}
