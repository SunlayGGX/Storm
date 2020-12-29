#pragma once


namespace StormPackager
{
	class BuildProcess
	{
	public:
		bool execute(const std::string &devenvPath, const std::string &sln);
	};
}
