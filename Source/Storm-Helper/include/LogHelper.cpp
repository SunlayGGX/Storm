#include "LogHelper.h"

#include "LogLevel.h"

#include "ThrowException.h"


std::string_view Storm::parseLogLevel(Storm::LogLevel logLevel)
{
#define STORM_SWITCH_CASE_STRINGIFY(CaseStatement) case Storm::LogLevel::CaseStatement: return #CaseStatement;
	switch (logLevel)
	{
		STORM_SWITCH_CASE_STRINGIFY(Debug);
		STORM_SWITCH_CASE_STRINGIFY(DebugWarning);
		STORM_SWITCH_CASE_STRINGIFY(DebugError);
		STORM_SWITCH_CASE_STRINGIFY(Comment);
		STORM_SWITCH_CASE_STRINGIFY(Warning);
		STORM_SWITCH_CASE_STRINGIFY(Error);
		STORM_SWITCH_CASE_STRINGIFY(Fatal);
		STORM_SWITCH_CASE_STRINGIFY(Always);
	}
#undef STORM_SWITCH_CASE_STRINGIFY

	Storm::throwException<std::exception>("Unknown Level!");
}
