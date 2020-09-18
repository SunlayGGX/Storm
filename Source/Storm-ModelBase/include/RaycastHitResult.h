#pragma once


namespace Storm
{
	struct RaycastHitResult
	{
	public:
		RaycastHitResult(std::size_t particleId, unsigned int systemId, const Storm::Vector3 &hitPosition);

	public:
		std::size_t _particleId;
		unsigned int _systemId;

		Storm::Vector3 _hitPosition;
	};
}
