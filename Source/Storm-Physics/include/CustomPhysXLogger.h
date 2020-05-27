#pragma once

namespace Storm
{
	class CustomPhysXLogger : public physx::PxErrorCallback
	{
	public:
		void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) final override;
	};
}
