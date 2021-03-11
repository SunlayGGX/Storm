#pragma once


namespace Storm
{
	enum class VolumeComputationTechnique
	{
		None,

		Auto,

		// Doesn't work for Concave geometries
		TriangleIntegration,
	};
}
