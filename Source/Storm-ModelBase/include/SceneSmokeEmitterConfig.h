#pragma once


namespace Storm
{
	struct SceneSmokeEmitterConfig
	{
	public:
		SceneSmokeEmitterConfig();

	public:
		unsigned int _emitterId;
		Storm::Vector3 _position;
		float _emitCountPerSeconds;
		float _smokeAliveTimeSeconds;
		float _emitterStartTimeSeconds;
		float _emitterEndTimeSeconds;
		Storm::Vector4 _color;
	};
}
