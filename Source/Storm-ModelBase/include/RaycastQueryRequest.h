#pragma once


namespace Storm
{
	enum class PartitionSelection;
	struct RaycastHitResult;

	using HitResponseCallback = std::function<void(std::vector<Storm::RaycastHitResult> &&)>;

	struct RaycastQueryRequest
	{
	public:
		RaycastQueryRequest(Storm::HitResponseCallback &&callback);

		RaycastQueryRequest& addPartitionFlag(Storm::PartitionSelection flag);
		RaycastQueryRequest& setMinDistance(float value);
		RaycastQueryRequest& setMaxDistance(float value);
		RaycastQueryRequest& considerOnlyVisible(bool value);
		RaycastQueryRequest& firstHitOnly();

	public:
		std::vector<Storm::PartitionSelection> _particleSystemSelectionFlag;
		float _minDistance;
		float _maxDistance;

		bool _considerOnlyVisible;

		bool _wantOnlyFirstHit;

		HitResponseCallback _hitResponseCallback;
	};
}
