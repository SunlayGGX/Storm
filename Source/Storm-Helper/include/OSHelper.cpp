#include "OSHelper.h"

#include "LeanWindowsInclude.h"


namespace
{
#ifdef environ
	struct EnvLoggerArg
	{
	public:
		constexpr EnvLoggerArg(const std::string_view &prettyDesc, const std::string_view &argToSearchFor) :
			_prettyDescriptor{ prettyDesc },
			_arg{ argToSearchFor }
		{}

	public:
		std::string_view _prettyDescriptor;
		std::string_view _arg;
	};

	template<class ArgType, class ... RemainEnvArgStrType>
	struct EnvLogger
	{
	public:
		static bool logEnv(const std::string_view &env, const ArgType &envParam, const RemainEnvArgStrType &... args)
		{
			return
				EnvLogger<ArgType>::logEnv(env, envParam) ||
				EnvLogger<RemainEnvArgStrType...>::logEnv(env, args...);
		}
	};

	template<class ArgType>
	struct EnvLogger<ArgType>
	{
	public:
		static bool logEnv(const std::string_view &env, const ArgType &envParam)
		{
			bool result;
			std::string_view envValue = EnvLogger<ArgType>::getEnvironValue(env, envParam, result);
			if (result)
			{
				LOG_DEBUG << envParam._prettyDescriptor << " is " << envValue;
				return true;
			}

			return false;
		}

		static constexpr std::string_view getEnvironValue(const std::string_view &env, const ArgType &envParam, bool &outSuccess)
		{
			const std::size_t pos = env.find(envParam._arg);
			if (pos != std::string_view::npos)
			{
				const std::size_t equalOffset = pos + envParam._arg.size();
				if (env[equalOffset] == '=')
				{
					outSuccess = true;
					return env.substr(equalOffset + 1);
				}
			}

			outSuccess = false;
			return "";
		}
	};

	template<class ... EnvArgStrType>
	void logEnvInformation(const char** env, const EnvArgStrType &... args)
	{
		if (env)
		{
			std::string_view envStr;
			while (*env)
			{
				envStr = *env;
				++env;

				EnvLogger<EnvArgStrType...>::logEnv(envStr, args...);
			}
		}
	}

	template<class EnvLoggerArg>
	std::pair<bool, std::string_view> retrieveEnvInformation(const char** env, const EnvLoggerArg &arg)
	{
		std::pair<bool, std::string_view> result;
		if (env)
		{

			std::string_view envStr;
			while (*env)
			{
				envStr = *env;
				++env;

				result.second = EnvLogger<EnvLoggerArg>::getEnvironValue(envStr, arg, result.first);
				if (result.first)
				{
					return result;
				}
			}
		}

		result.first = false;
		return result;
	}
#endif
}

std::string Storm::OSHelper::getRawQuotedCommandline()
{
	return Storm::toStdString(::GetCommandLine());
}

void Storm::OSHelper::logOSEnvironmentInformation()
{
#ifdef environ
	logEnvInformation(const_cast<const char**>(environ),
		EnvLoggerArg{ "Computer name", "COMPUTERNAME" },
		EnvLoggerArg{ "User name", "USERNAME" },
		EnvLoggerArg{ "Os level", "OS" },
		EnvLoggerArg{ "Domain", "USERDOMAIN" }
	);
#else

	// We cannot even use getenv because getenv suppose we know the variable to query.
	// Therefore we should know what is the variable name on every platform.
	// Too much work (portability is heavy stuff) for a feature that is here just to help debugging... So just log we don't know about those variables.
	// If we really need it, then we'll do it later.
	LOG_DEBUG_WARNING <<
		"OS environment variable logging disabled because 'environ' C-Macro doesn't exist.\n"
		"We're not compiling for a POSIX like OS and we don't even know what environment variable to find.";

#endif
}


std::string Storm::OSHelper::getComputerNameFromEnviron()
{
#ifdef environ
	const auto hostnameInfo = retrieveEnvInformation(const_cast<const char**>(environ), EnvLoggerArg{ "Computer name", "COMPUTERNAME" });
	if (hostnameInfo.first)
	{
		return std::string{ hostnameInfo.second };
	}
	else
	{
		LOG_DEBUG_ERROR << "No computer name inside environ C-Macro.";
	}
#else

	// We cannot even use getenv because getenv suppose we know the variable to query.
	// Therefore we should know what is the variable name on every platform.
	// Too much work (portability is heavy stuff) for a feature that is here just to help debugging... So just log we don't know about those variables.
	// If we really need it, then we'll do it later.
	LOG_DEBUG_ERROR <<
		"OS environment variable logging disabled because 'environ' C-Macro doesn't exist.\n"
		"We're not compiling for a POSIX like OS so we cannot get the computer name.";

#endif

	return std::string{};
}

