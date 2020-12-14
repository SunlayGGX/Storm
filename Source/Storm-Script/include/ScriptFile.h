#pragma once


namespace Storm
{
	enum class ThreadEnumeration;

	class ScriptFile
	{
	public:
		ScriptFile(const Storm::ThreadEnumeration associatedThread, const std::string &filePath);

	public:
		void update();
		void invalidate();

	private:
		std::filesystem::path _filePath;
		std::filesystem::file_time_type _lastWriteTime;
		const Storm::ThreadEnumeration _associatedThread;
	};
}
