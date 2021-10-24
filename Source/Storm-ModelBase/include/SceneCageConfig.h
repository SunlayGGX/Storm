#pragma once


namespace Storm
{
	struct SceneCageConfig
	{
	public:
		SceneCageConfig();

	public:
		Storm::Vector3 _boxMin;
		Storm::Vector3 _boxMax;

		float _rbSimulKillY;

		bool _deterministic;
		Storm::Vector3 _reboundPenalty;

		bool _infiniteDomain;
		Storm::Vector3 _passthroughVelReduceCoeffLeftBottomFront;
		Storm::Vector3 _passthroughVelReduceCoeffRightTopBack;
	};
}
