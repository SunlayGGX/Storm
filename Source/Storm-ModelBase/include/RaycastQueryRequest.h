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
		~RaycastQueryRequest();

	public:
		std::vector<Storm::PartitionSelection> _particleSystemSelectionFlag;
		float _minDistance;
		float _maxDistance;

		HitResponseCallback _hitResponseCallback;
	};
}
