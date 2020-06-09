#define STORM_AUTOMATION_NO_AUTO_LIBRARY_LINK
#include "MainDiscover.h"

#define CATCH_CONFIG_RUNNER
#	include <catch2/catch.hpp>
#undef CATCH_CONFIG_RUNNER


namespace
{
	struct TestRun
	{
		TestRun(Catch::IStreamingReporterPtr &&reporter, const Catch::TestRunInfo& runInfo, const Catch::Totals &totals)
			: _reporter{ std::move(reporter) }
			, _runInfo{ runInfo }
			, _totals{ totals }
		{
			_reporter->testRunStarting(runInfo);
		}

		~TestRun()
		{
			Catch::TestRunStats testrunstats(_runInfo, _totals, false);
			_reporter->testRunEnded(testrunstats);
		}

		Catch::IStreamingReporterPtr _reporter;
		const Catch::TestRunInfo &_runInfo;
		const Catch::Totals &_totals;
	};

	struct TestGroupRun
	{
		TestGroupRun(const Catch::IStreamingReporterPtr &reporter, const std::string &configName)
			: _groupInfo{ configName, 1, 1 }
			, _reporter{ reporter.get() }
		{
			_reporter->testGroupStarting(_groupInfo);
		}

		~TestGroupRun()
		{
			_reporter->testGroupEnded(_groupInfo);
		}

		const Catch::IStreamingReporterPtr::pointer _reporter;
		Catch::GroupInfo _groupInfo;
	};
}

constexpr const char* retrieveDiscoveryCommandLineTag()
{
	return "--discover";
}

int runTestOrDiscovery(int argc, const char* argv[])
{
	Catch::Session session;

	bool doDiscover = false;

	// Add option to commandline
	{
		using namespace Catch::clara;

		auto cli = session.cli()
			| Opt(doDiscover)
			[retrieveDiscoveryCommandLineTag()]
		("Perform VS Test Adaptor discovery");

		session.cli(cli);
	}

	// Process commandline
	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0)
	{
		return returnCode;
	}

	// Check if custom discovery needs to be performed
	if (doDiscover)
	{
		try
		{
			// Retrieve testcases
			const auto& config = session.config();
			auto testspec = config.testSpec();
			auto testcases = filterTests(Catch::getAllTestCasesSorted(config), testspec, config);

			// Setup reporter
			Catch::TestRunInfo runInfo(config.name());

			auto pConfig = std::make_shared<const Catch::Config>(session.configData());

			Catch::Totals totals;

			{
				TestRun testRun{
					Catch::getRegistryHub()
						.getReporterRegistry()
						.create("xml", pConfig),
					runInfo,
					totals
				};

				TestGroupRun testGroupRun{ testRun._reporter, config.name() };

				// Report test cases
				for (const auto &testcase : testcases)
				{
					Catch::TestCaseInfo caseinfo(testcase.name,
						testcase.className,
						testcase.description,
						testcase.tags,
						testcase.lineInfo
					);

					testRun._reporter->testCaseStarting(caseinfo);
					testRun._reporter->testCaseEnded(Catch::TestCaseStats(caseinfo, totals, "", "", false));
				}
			}

			return 0;
		}
		catch (const std::exception &ex)
		{
			Catch::cerr() << ex.what() << std::endl;
			return Catch::MaxExitCode;
		}
	}

	// Let Catch2 do its thing
	return session.run();
}
