#include "PhysXDebugger.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "GeneralDebugConfig.h"

#include "SocketSetting.h"

#include "UIField.h"
#include "UIFieldContainer.h"



#define STORM_PVD_CONNECTION_FIELD_NAME "PVD Connected"

namespace Storm
{
	// I won't derive it from PxPvdClient because the documentation is lacking a lot and the class is buggy (the onConnect and onDisconnect are not fired on some cases).
	class PhysXPVDConnectHandler
	{
	public:
		PhysXPVDConnectHandler(physx::PxPvd* pvd, physx::PxPvdTransport* pvdTransport) :
			_isConnected{ false },
			_pvd{ pvd },
			_pvdTransport{ pvdTransport }
		{
			_field
				.bindField(STORM_PVD_CONNECTION_FIELD_NAME, _isConnected)
				;
		}

	public:
		void reconnect()
		{
			_pvd->disconnect();
			this->connect();
		}

		bool connect()
		{
			this->setIsConnected(_pvd->connect(*_pvdTransport, physx::PxPvdInstrumentationFlag::Enum::eALL));

			if (_isConnected)
			{
				this->initFlags();
				return true;
			}

			return false;
		}

		void disconnect()
		{
			_pvd->disconnect();
			this->setIsConnected(false);
			this->clearFlags();
		}

		void initFlags()
		{
			if (_isConnected && _pxPvdClient)
			{
				const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
				const Storm::GeneralDebugConfig &generalDebugConfig = configMgr.getGeneralDebugConfig();

				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_CONSTRAINTS, generalDebugConfig._pvdTransmitConstraints);
				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_CONTACTS, generalDebugConfig._pvdTransmitContacts);
				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_SCENEQUERIES, generalDebugConfig._pvdTransmitSceneQueries);
			}
		}

		void clearFlags()
		{
			if (_pxPvdClient)
			{
				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_CONSTRAINTS, false);
				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_CONTACTS, false);
				_pxPvdClient->setScenePvdFlag(physx::PxPvdSceneFlag::Enum::eTRANSMIT_SCENEQUERIES, false);
			}
		}

		void setScenePvdClient(physx::PxPvdSceneClient* pxPvdClient)
		{
			_pxPvdClient = pxPvdClient;
		}

	private:
		void setIsConnected(const bool connected)
		{
			if (_isConnected != connected)
			{
				_isConnected = connected;
				_field.pushField(STORM_PVD_CONNECTION_FIELD_NAME);

				if (connected)
				{
					LOG_DEBUG << "PhysX Visualizer Debugger (PVD) connected!";
				}
				else
				{
					LOG_DEBUG << "PhysX Visualizer Debugger (PVD) disconnected!";
				}
			}
		}

	private:
		bool _isConnected;
		Storm::UIFieldContainer _field;

		physx::PxPvd* _pvd;
		physx::PxPvdTransport* _pvdTransport;
		physx::PxPvdSceneClient* _pxPvdClient;
	};
}


Storm::PhysXDebugger::PhysXDebugger(physx::PxFoundation &foundation)
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const Storm::GeneralDebugConfig &generalDebugConfig = configMgr.getGeneralDebugConfig();

	_enabled =
		generalDebugConfig._physXPvdDebugSocketSettings->_isEnabled &&
		!configMgr.isInReplayMode()
		;

	if (_enabled)
	{
		const std::string &ip = generalDebugConfig._physXPvdDebugSocketSettings->getIPStr();

		_pvd = physx::PxCreatePvd(foundation);
		_transport = physx::PxDefaultPvdSocketTransportCreate(
			ip.c_str(),
			generalDebugConfig._physXPvdDebugSocketSettings->_port,
			generalDebugConfig._pvdConnectTimeoutMillisec
		);

		_pvdConnectHandler = std::make_unique<Storm::PhysXPVDConnectHandler>(_pvd.get(), _transport.get());
		if (!_pvdConnectHandler->connect())
		{
			LOG_DEBUG_WARNING << "PhysX Visualizer Debugger (PVD) wasn't connected!";
		}
	}
}

Storm::PhysXDebugger::~PhysXDebugger()
{
	if (_enabled)
	{
		this->close();

		_pvd.reset();
		_transport.reset();
	}
}

void Storm::PhysXDebugger::reconnect()
{
	if (_enabled)
	{
		_pvdConnectHandler->reconnect();
	}
}

void Storm::PhysXDebugger::reconnectIfNeeded()
{
	if (_enabled)
	{
		if (!_pvd->isConnected(false))
		{
			this->reconnect();
		}
	}
}

void Storm::PhysXDebugger::close()
{
	if (_enabled && _pvdConnectHandler)
	{
		_pvdConnectHandler->disconnect();
		_pvdConnectHandler.reset();
	}
}

void Storm::PhysXDebugger::finishSetup(physx::PxScene* pxScene)
{
	if (_enabled)
	{
		if (!pxScene)
		{
			Storm::throwException<Storm::Exception>("PxScene should be valid!");
		}

		_scene = pxScene;

		_pvdConnectHandler->setScenePvdClient(_scene->getScenePvdClient());
		_pvdConnectHandler->initFlags();
	}
}

void Storm::PhysXDebugger::prepareDestroy()
{
	_scene = nullptr;
	if (_enabled && _pvdConnectHandler)
	{
		_pvdConnectHandler->setScenePvdClient(nullptr);
		_pvdConnectHandler->disconnect();
	}
}

physx::PxPvd* Storm::PhysXDebugger::getPvd() const
{
	return _enabled ? _pvd.get() : nullptr;
}
