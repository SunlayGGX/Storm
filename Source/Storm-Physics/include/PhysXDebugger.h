#pragma once

#include "UniquePointer.h"


namespace Storm
{
	class PhysXPVDConnectHandler;

	class PhysXDebugger
	{
	public:
		PhysXDebugger(physx::PxFoundation &foundation);
		~PhysXDebugger();

	public:
		bool reconnect();
		void reconnectIfNeeded();

		void close();

		void finishSetup(physx::PxScene* pxScene);
		void prepareDestroy();

	public:
		physx::PxPvd* getPvd() const;

	private:
		bool _enabled;
		Storm::UniquePointer<physx::PxPvd> _pvd;
		Storm::UniquePointer<physx::PxPvdTransport> _transport;

		std::unique_ptr<Storm::PhysXPVDConnectHandler> _pvdConnectHandler;

		std::chrono::high_resolution_clock::time_point _lastReconnectionTimeFailure;

		std::string _ipStr;

		physx::PxScene* _scene;
	};
}
