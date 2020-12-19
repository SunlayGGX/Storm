#pragma once


namespace Storm
{
	enum class ThreadEnumeration;

	class ScriptFile
	{
	public:
		ScriptFile(const std::string &filePath, bool prepareToCall = false);

	public:
		void update();
		void invalidate();

	private:
		std::filesystem::path _filePath;
		std::filesystem::file_time_type _lastWriteTime;
	};
}
