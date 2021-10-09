#pragma once


namespace Storm
{
	struct SerializeSupportedFeatureLayout
	{
	public:
		SerializeSupportedFeatureLayout();

	public:
		uint8_t _hasPSystemGlobalForce : 1;
		uint8_t _hasDensities : 1;
		uint8_t _hasPressures : 1;
		uint8_t _hasVolumes : 1;
		uint8_t _hasDragComponentforces : 1; 
		uint8_t _hasDynamicPressureQForces : 1;
		uint8_t _hasNormals : 1;
		uint8_t _hasNoStickForces : 1;
		uint8_t _hasPSystemTotalEngineForce : 1;
		uint8_t _hasIntermediaryPressureForces : 1;
		uint8_t _hasWantedDensity : 1;
		uint8_t _hasCoendaForces : 1;
		uint8_t _hasKernelLength : 1;
		uint8_t _hasInfiniteDomainFlag : 1;
	};
}
