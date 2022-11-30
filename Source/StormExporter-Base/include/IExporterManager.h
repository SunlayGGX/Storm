#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	enum class ExitCode : int;
}

namespace StormExporter
{
	class IExporterManager : public Storm::ISingletonHeldInterface<IExporterManager>
	{
	protected:
		~IExporterManager() = default;

	public:
		virtual void doInitialize() = 0;
		virtual void doCleanUp() = 0;
		virtual Storm::ExitCode run() = 0;
	};
}
