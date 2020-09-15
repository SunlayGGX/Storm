#pragma once


namespace Storm
{
	struct RaycastHitResult
	{
	public:
		std::size_t _particleId;
		unsigned int _systemId;

		Storm::Vector3 _hitPosition;
	};
}
