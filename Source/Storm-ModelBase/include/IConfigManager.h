#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	class IConfigManager : public Storm::ISingletonHeldInterface<IConfigManager>
	{
	public:
		virtual ~IConfigManager() = default;

	public:
		// Paths
		virtual const std::string& getTemporaryPath() const = 0;
		virtual const std::string& getExePath() const = 0;

		// Logs
		virtual const std::string& getLogFileName() const = 0;
		virtual bool noPopup() const = 0;
	};
}
