#pragma once


namespace Storm
{
	struct MassCoeffConfig
	{
	public:
		MassCoeffConfig();

	public:
		float _reducedMassCoefficient; // Apply reduced mass coefficient like SPlisHSPlasH does to prevent pressure spikes.
		float _startReducedMassCoeff;
		float _fadeInTimeSec;
		bool _updateMasses;
	};
}
