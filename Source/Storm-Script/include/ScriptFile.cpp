#include "ScriptFile.h"

#include "ScriptManager.h"

#include <fstream>


Storm::ScriptFile::ScriptFile(const Storm::ThreadEnumeration associatedThread, const std::string &filePath) :
	_associatedThread{ associatedThread },
	_filePath{ filePath },
	_lastWriteTime{ std::filesystem::last_write_time(filePath) }
{

}

void Storm::ScriptFile::update()
{
	const auto currentLastWriteTime = std::filesystem::last_write_time(_filePath);
	if (_lastWriteTime < currentLastWriteTime)
	{
		_lastWriteTime = currentLastWriteTime;
		LOG_DEBUG << "Script file " << _filePath << " was updated. The script inside will be read and sent.";

		const std::size_t fileSize = std::filesystem::file_size(_filePath);
		
		std::string scriptContent;
		scriptContent.reserve(fileSize);

		std::ifstream scriptContentStream{ _filePath };
		scriptContentStream.read(scriptContent.data(), fileSize);

		Storm::ScriptManager::instance().execute(_associatedThread, std::move(scriptContent));
	}
}

void Storm::ScriptFile::invalidate()
{
	_lastWriteTime = std::filesystem::file_time_type{ std::filesystem::file_time_type::duration::zero() };
}
