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
#include "ScriptManager.h"
#include "WebManager.h"
#include "NetworkManager.h"
#include "BibliographyManager.h"

#include "ScriptImplementation.inl.h"

#include "Version.h"
#include "TimeHelper.h"
#include "UIModality.h"
#include "OSHelper.h"

#include "ThreadEnumeration.h"
#include "ThreadFlaggerObject.h"


namespace
{
	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		Storm::SingletonHolder,
		Storm::ScriptManager,
		Storm::ConfigManager,
		Storm::LoggerManager,
		Storm::SerializerManager,
		Storm::ProfilerManager,
		Storm::ThreadManager,
		Storm::RandomManager,
		Storm::BibliographyManager,
		Storm::NetworkManager,
		Storm::OSManager,
		Storm::TimeManager,
		Storm::InputManager,
		Storm::WebManager,
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
		STORM_DECLARE_THIS_THREAD_IS << Storm::ThreadFlagEnum::MainThread;

		Storm::ConfigManager::instance().initialize(argc, argv);

		Storm::LoggerManager::instance().initialize();

		Storm::ThreadManager::instance().initialize();

		if (shouldRunSimulation())
		{
			STORM_DECLARE_THIS_THREAD_IS <<
				Storm::ThreadFlagEnum::SimulationThread <<
				Storm::ThreadFlagEnum::RaycastThread <<
				Storm::ThreadFlagEnum::SpaceThread <<
				Storm::ThreadFlagEnum::LoadingThread <<
				Storm::ThreadFlagEnum::PhysicsThread;

			Storm::OSHelper::logOSEnvironmentInformation();

			const bool hasUI = Storm::ConfigManager::instance().withUI();

			Storm::TimeManager::instance().initialize();

			Storm::BibliographyManager::instance().initialize();
			Storm::RandomManager::instance().initialize();
			Storm::OSManager::instance().initialize();

			Storm::NetworkManager::instance().initialize();
			Storm::WebManager::instance().initialize();

			if (hasUI)
			{
				Storm::InputManager::instance().initialize(Storm::WithUI{});
			}
			else
			{
				Storm::InputManager::instance().initialize(Storm::NoUI{});
			}

			Storm::SerializerManager::instance().initialize();

			Storm::PhysicsManager::instance().initialize();

			Storm::ShaderManager::instance().initialize();

			if (hasUI)
			{
				Storm::GraphicManager::instance().initialize(Storm::WithUI{});
			}
			else
			{
				Storm::GraphicManager::instance().initialize(Storm::NoUI{});
			}

			Storm::AssetLoaderManager::instance().initialize();

			if (hasUI)
			{
				Storm::WindowsManager::instance().initialize(Storm::WithUI{});
			}
			else
			{
				Storm::WindowsManager::instance().initialize(Storm::NoUI{});
			}

			Storm::SimulatorManager::instance().initialize();

			Storm::SpacePartitionerManager::instance().initialize(Storm::SimulatorManager::instance().getKernelLength());

			Storm::RaycastManager::instance().initialize();

			Storm::ProfilerManager::instance().initialize();

			// Here will be executed the custom initialization from script.
			Storm::ScriptManager::instance().initialize();
		}

		LOG_COMMENT << "Application Creation finished";
	}

	void cleanUpStormApplication()
	{
		if (g_singletonMaker != nullptr)
		{
			LOG_COMMENT << "Application Cleanup";

			const bool hasUI = Storm::ConfigManager::instance().withUI();

			Storm::ScriptManager::instance().cleanUp();

			Storm::TimeManager::instance().cleanUp();
			Storm::SimulatorManager::instance().cleanUp();
			Storm::RaycastManager::instance().cleanUp();
			Storm::SpacePartitionerManager::instance().cleanUp();
			Storm::ThreadManager::instance().cleanUp();

			if (hasUI)
			{
				Storm::InputManager::instance().cleanUp(Storm::WithUI{});
				Storm::GraphicManager::instance().cleanUp(Storm::WithUI{});
			}
			else
			{
				Storm::InputManager::instance().cleanUp(Storm::NoUI{});
				Storm::GraphicManager::instance().cleanUp(Storm::NoUI{});
			}

			Storm::ProfilerManager::instance().cleanUp();
			Storm::AssetLoaderManager::instance().cleanUp();
			Storm::ShaderManager::instance().cleanUp();
			Storm::PhysicsManager::instance().cleanUp();

			if (hasUI)
			{
				Storm::WindowsManager::instance().cleanUp(Storm::WithUI{});
			}
			else
			{
				Storm::WindowsManager::instance().cleanUp(Storm::NoUI{});
			}

			Storm::WebManager::instance().cleanUp();
			Storm::OSManager::instance().cleanUp();
			Storm::NetworkManager::instance().cleanUp();
			Storm::BibliographyManager::instance().cleanUp();
			Storm::RandomManager::instance().cleanUp();
			Storm::SerializerManager::instance().cleanUp();
			Storm::ConfigManager::instance().cleanUp();
			Storm::LoggerManager::instance().cleanUp();

			Storm::unhookVectoredExceptionsDebugging();

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
	catch (const Storm::Exception &ex)
	{
		LOG_FATAL <<
			"Catched an unhandled Storm exception in the Application main loop (run). Process will exit.\n"
			"Reason : " << ex.what() << ".\n"
			"Stack trace was :\n" << ex.stackTrace();
			;
		throw;
	}
	catch (const std::exception &ex)
	{
		LOG_FATAL << "Catched an unhandled std exception in the Application main loop (run). Process will exit. Reason : " << ex.what();
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
		catch (const Storm::Exception &cleanUpEx)
		{
			std::string errorMsg2{ 
				"Another exception happened during cleanup : " + Storm::toStdString(cleanUpEx) + ".\n"
				"Stack trace :\n" + cleanUpEx.stackTrace()
			};

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
