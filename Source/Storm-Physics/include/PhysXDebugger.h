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
		void reconnect();
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

		std::string _ipStr;

		physx::PxScene* _scene;
	};
}
