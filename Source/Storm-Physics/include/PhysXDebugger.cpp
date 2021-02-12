#include "PhysXDebugger.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralDebugConfig.h"

#include "SocketSetting.h"



Storm::PhysXDebugger::PhysXDebugger(physx::PxFoundation &foundation)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralDebugConfig &generalDebugConfig = configMgr.getGeneralDebugConfig();

	_enabled = generalDebugConfig._physXPvdDebugSocketSettings->_isEnabled;
	if (_enabled)
	{
		const std::string ipAddress = generalDebugConfig._physXPvdDebugSocketSettings->getIPStr();

		_pvd = physx::PxCreatePvd(foundation);
		_transport = physx::PxDefaultPvdSocketTransportCreate(
			ipAddress.c_str(),
			generalDebugConfig._physXPvdDebugSocketSettings->_port,
			generalDebugConfig._physXPvdDebugSocketSettings->_timeoutMillisec
		);

		if (_pvd->connect(*_transport, physx::PxPvdInstrumentationFlag::Enum::eALL))
		{
			LOG_DEBUG << "PhysX Visualizer Debugger (PVD) connected!";
		}
		else
		{
			LOG_DEBUG_WARNING << "PhysX Visualizer Debugger (PVD) wasn't connected!";
		}
	}
}

Storm::PhysXDebugger::~PhysXDebugger()
{
	_pvd.reset();
	_transport.reset();
}

physx::PxPvd* Storm::PhysXDebugger::getPvd() const
{
	return _enabled ? _pvd.get() : nullptr;
}
