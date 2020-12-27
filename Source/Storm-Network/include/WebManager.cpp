#include "WebManager.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"
#include "IOSManager.h"

#include "GeneralWebConfig.h"
#include "PreferredBrowser.h"

#include "StormProcessStartup.h"


namespace
{
	[[noreturn]] void noBrowserExceptionStop()
	{
		Storm::throwException<Storm::StormException>("Cannot open any URL when user hasn't specified its Browser (You must specify it through your general config).");
	}

	const std::string_view obtainBrowserNameOnCmd(const Storm::PreferredBrowser browser)
	{
		switch (browser)
		{
		case Storm::PreferredBrowser::Chrome:				return "chrome ";
		case Storm::PreferredBrowser::Firefox:				return "firefox ";
		case Storm::PreferredBrowser::InternetExplorer:		return "iexplore ";
		case Storm::PreferredBrowser::Edge:					return "msedge ";

		case Storm::PreferredBrowser::None:
		default: noBrowserExceptionStop();
		}
	}

	const std::string_view obtainBrowserPrivateOnCmd(const Storm::PreferredBrowser browser)
	{
		switch (browser)
		{
		case Storm::PreferredBrowser::Chrome:				return "-incognito ";
		case Storm::PreferredBrowser::Firefox:				return "-private ";
		case Storm::PreferredBrowser::InternetExplorer:		return "-private ";
		case Storm::PreferredBrowser::Edge:					return "-inprivate ";

		case Storm::PreferredBrowser::None:
		default: noBrowserExceptionStop();
		}
	}
}


Storm::WebManager::WebManager() = default;
Storm::WebManager::~WebManager() = default;

std::size_t Storm::WebManager::openURL(const std::string_view &url)
{
	const std::size_t urlLength = url.size();

	if (urlLength > 0)
	{
		const Storm::SingletonHolder &singletonHolder = Storm::SingletonHolder::instance();
		const Storm::IConfigManager &configMgr = singletonHolder.getSingleton<Storm::IConfigManager>();
		const Storm::GeneralWebConfig &generalWebConfig = configMgr.getGeneralWebConfig();

		Storm::StormProcessStartup startCommand{
			._bindIO = false,
			._shareLife = false,
			._isCmd = true
		};

		/* 1st : build the command line to start a browser on the requested url */

		const std::string_view browserNameOnCmd = obtainBrowserNameOnCmd(generalWebConfig._preferredBrowser);
		const std::string_view browserPrivateModeOnCmd = obtainBrowserNameOnCmd(generalWebConfig._preferredBrowser);

		startCommand._commandLine.reserve(urlLength + 64 + browserNameOnCmd.size() + browserPrivateModeOnCmd.size());

		// Mandatory or when we start the process, the Application will hang until user closes its browser (we'll wait indefinitely until the get the exit code of the browser).
		startCommand._commandLine += "start ";

		startCommand._commandLine += browserNameOnCmd;
		if (generalWebConfig._urlOpenIncognito)
		{
			startCommand._commandLine += browserPrivateModeOnCmd;
		}

		startCommand._commandLine += url;

		/* 2nd : request to execute the command line */
		Storm::IOSManager &osMgr = singletonHolder.getSingleton<Storm::IOSManager>();
		return osMgr.startProcess(std::move(startCommand));
	}
	else
	{
		// The purpose of Storm application is not to start a browser on the default home page... 
		LOG_DEBUG_WARNING << "We ignored the request to start a browser on an empty url.";
	}

	return std::numeric_limits<std::size_t>::max();
}
