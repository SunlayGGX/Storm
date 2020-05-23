#pragma once


namespace Storm
{
	struct EarlyExitAnswer
	{
		int _exitCode;
		std::vector<std::string> _unconsumedErrorMsgs;
	};
}
