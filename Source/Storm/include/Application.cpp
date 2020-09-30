#include "Application.h"

#include "SingletonAllocator.h"
#include "SingletonHolder.h"
#include "LoggerManager.h"
#include "ConfigManager.h"
#include "WindowsManager.h"
#include "InputManager.h"
#include "ShaderManager.h"
#include "AssetLoaderManager.h"
#include "GraphicManager.h"
#include "OSManager.h"
#include "RandomManager.h"
#include "TimeManager.h"
#include "SimulatorManager.h"
#include "PhysicsManager.h"
#include "ThreadManager.h"
#include "SpacePartitionerManager.h"
#include "RaycastManager.h"
#include "ProfilerManager.h"
#include "SerializerManager.h"

#include "Version.h"
#include "TimeHelper.h"
#include "ThreadEnumeration.h"


namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		Storm::SingletonHolder,
		Storm::ConfigManager,
		Storm::LoggerManager,
		Storm::SerializerManager,
		Storm::ProfilerManager,
		Storm::ThreadManager,
		Storm::RandomManager,
		Storm::OSManager,
		Storm::TimeManager,
		Storm::InputManager,
		Storm::WindowsManager,
		Storm::AssetLoaderManager,
		Storm::ShaderManager,
		Storm::GraphicManager,
		Storm::PhysicsManager,
		Storm::SpacePartitionerManager,
		Storm::RaycastManager,
		Storm::SimulatorManager
	>;

	std::unique_ptr<SingletonAllocatorAlias> g_singletonMaker;

	bool shouldRunSimulation()
	{
		return !Storm::ConfigManager::instance().shouldDisplayHelp();
	}

	void initializeStormApplication(int argc, const char* argv[])
	{
		g_singletonMaker = std::make_unique<SingletonAllocatorAlias>();

		LOG_ALWAYS << 
			"Welcome to Storm executable (v" << static_cast<std::string>(Storm::Version::retrieveCurrentStormVersion()) << ") (a new SPH simulator).\n"
			"Current date is " << Storm::TimeHelper::getCurrentDateTime(true)
			;
		LOG_COMMENT << "Application Creation started";
	
		STORM_REGISTER_THREAD(MainThread);

		Storm::ConfigManager::instance().initialize(argc, argv);

		Storm::LoggerManager::instance().initialize();

		Storm::ThreadManager::instance().initialize();

		if (shouldRunSimulation())
		{
			Storm::TimeManager::instance().initialize();

			Storm::RandomManager::instance().initialize();
			Storm::OSManager::instance().initialize();

			Storm::InputManager::instance().initialize();

			Storm::SerializerManager::instance().initialize();

			Storm::PhysicsManager::instance().initialize();

			Storm::ShaderManager::instance().initialize();
			Storm::GraphicManager::instance().initialize();

			Storm::AssetLoaderManager::instance().initialize();

			Storm::WindowsManager::instance().initialize();

			Storm::SimulatorManager::instance().initialize();

			Storm::SpacePartitionerManager::instance().initialize(Storm::SimulatorManager::instance().getKernelLength());

			Storm::RaycastManager::instance().initialize();

			Storm::ProfilerManager::instance().initialize();
		}

		LOG_COMMENT << "Application Creation finished";
	}

	void cleanUpStormApplication()
	{
		if (g_singletonMaker != nullptr)
		{
			LOG_COMMENT << "Application Cleanup";

			Storm::TimeManager::instance().cleanUp();
			Storm::SimulatorManager::instance().cleanUp();
			Storm::RaycastManager::instance().cleanUp();
			Storm::SpacePartitionerManager::instance().cleanUp();
			Storm::ThreadManager::instance().cleanUp();
			Storm::InputManager::instance().cleanUp();
			Storm::ProfilerManager::instance().cleanUp();
			Storm::GraphicManager::instance().cleanUp();
			Storm::AssetLoaderManager::instance().cleanUp();
			Storm::ShaderManager::instance().cleanUp();
			Storm::PhysicsManager::instance().cleanUp();
			Storm::WindowsManager::instance().cleanUp();
			Storm::OSManager::instance().cleanUp();
			Storm::RandomManager::instance().cleanUp();
			Storm::SerializerManager::instance().cleanUp();
			Storm::ConfigManager::instance().cleanUp();
			Storm::LoggerManager::instance().cleanUp();

			g_singletonMaker.reset();
		}
	}
}

Storm::Application::Application(int argc, const char* argv[])
{
	Storm::setupFullAssertionBox();

	initializeStormApplication(argc, argv);
}

Storm::Application::~Application()
{
	cleanUpStormApplication();
}

Storm::ExitCode Storm::Application::run()
{
	Storm::ExitCode result = ExitCode::k_success;

	try
	{
		if (shouldRunSimulation())
		{
			LOG_COMMENT << "Start Application Run";
			result = Storm::SimulatorManager::instance().run();
		}
		else
		{
			LOG_COMMENT << "Help requested, therefore the simulator wasn't initialized! We will exit now.";
		}
	}
	catch (const std::exception &ex)
	{
		LOG_FATAL << "Catched an unhandled std exception in the Application main loop (run). Process will exit " << ex.what();
		throw;
	}
	catch (...)
	{
		LOG_FATAL << "Catched an unhandled '...' exception (unknown) in the Application main loop (run). Process will exit.";
		throw;
	}

	return result;
}

Storm::EarlyExitAnswer Storm::Application::ensureCleanStateAfterException(const std::string &errorMsg, bool wasStdException)
{
	Storm::EarlyExitAnswer result;

	if (g_singletonMaker != nullptr)
	{
		// If it Application finished at init time, the singletons are still allocated (the remains) so we could still log them...
		LOG_FATAL << errorMsg;

		try
		{
			cleanUpStormApplication();
		}
		catch (const std::exception &cleanUpEx)
		{
			std::string errorMsg2{ "Another exception happened during cleanup : " + Storm::toStdString(cleanUpEx) };
			if (Storm::LoggerManager::isAlive() && Storm::LoggerManager::instance().isInitialized())
			{
				LOG_FATAL << errorMsg2;
			}
			else
			{
				result._unconsumedErrorMsgs.emplace_back(std::move(errorMsg2));
			}

			result._exitCode = static_cast<int>(Storm::ExitCode::k_imbricatedStdException);
		}
		catch (...)
		{
			result._exitCode = static_cast<int>(Storm::ExitCode::k_imbricatedUnknownException);
		}
	}
	else
	{
		result._unconsumedErrorMsgs.emplace_back(errorMsg);
	}

	if (wasStdException)
	{
		result._exitCode = static_cast<int>(Storm::ExitCode::k_stdException);
	}
	else
	{
		result._exitCode = static_cast<int>(Storm::ExitCode::k_unknownException);
	}

	return result;
}
