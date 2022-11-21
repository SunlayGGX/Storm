#pragma once


namespace Storm
{
	struct PushedParticleEmitterData;
	struct SceneSmokeEmitterConfig;

	struct EmitParticleData
	{
	public:
		EmitParticleData(const Storm::Vector3 &position, float time);

	public:
		Storm::Vector3 _position;
		float _remainingTime;
	};

	class EmitterObject
	{
	public:
		EmitterObject(const SceneSmokeEmitterConfig &associatedCfg);

	public:
		std::size_t getEmittedCount() const noexcept { return _emitted.size(); }

	public:
		void update(float deltaTime, Storm::PushedParticleEmitterData &appendDataThisFrame);

	private:
		void updateEmittedList(float deltaTime);
		void updateEmittedData(float deltaTime, EmitParticleData &emitted) const;
		void decreaseEmittedLife(float deltaTime);
		void emitNew(float deltaTime);

	private:
		bool _enabled;

		const SceneSmokeEmitterConfig &_cfg;
		float _spawningTime;
		float _currentEmitterTime;
		float _nextSpawnTime;

		std::list<EmitParticleData> _emitted;
	};
}
