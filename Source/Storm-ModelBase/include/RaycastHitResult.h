#pragma once


namespace Storm
{
	struct RaycastHitResult
	{
	public:
		RaycastHitResult(std::size_t particleId, unsigned int systemId, float hitDistance);

	public:
		std::size_t _particleId;
		unsigned int _systemId;

		float _hitDistance;
	};
}
