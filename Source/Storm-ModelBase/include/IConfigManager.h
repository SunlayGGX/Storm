#pragma once

#include "SingletonHeldInterfaceBase.h"


namespace Storm
{
	struct SceneData;
	struct GraphicData;
	struct RigidBodySceneData;

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
		virtual const std::string& getLogFolderPath() const = 0;
		virtual Storm::LogLevel getLogLevel() const = 0;
		virtual int getRemoveLogOlderThanDaysCount() const = 0;
		virtual bool getShouldOverrideOldLog() const = 0;
		virtual bool getShouldLogFpsWatching() const = 0;
		virtual bool noPopup() const = 0;

		// Scene data
		virtual const Storm::SceneData& getSceneData() const = 0;
		virtual const Storm::GraphicData& getGraphicData() const = 0;
		virtual const std::vector<Storm::RigidBodySceneData>& getRigidBodiesData() const = 0;
	};
}
