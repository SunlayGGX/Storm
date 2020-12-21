#include "ScriptFile.h"

#include "ScriptManager.h"

#include "StringHijack.h"
#include "ThrowException.h"

#include <fstream>



Storm::ScriptFile::ScriptFile(const std::string &filePath, bool prepareToCall /*= false*/) :
	_filePath{ filePath },
	_lastWriteTime{ std::filesystem::file_time_type::duration::zero() }
{
	assert(!filePath.empty() && "Script file file path shouldn't be empty!");

	if (!std::filesystem::exists(_filePath))
	{
		std::filesystem::create_directories(_filePath.parent_path());

		std::ofstream{ filePath } << std::string{};
	}

	if (!std::filesystem::is_regular_file(_filePath))
	{
		Storm::throwException<std::exception>(filePath + " is not a regular plain file (it should be to be used as a scripting canvas)!");
	}

	if (!prepareToCall)
	{
		_lastWriteTime = std::filesystem::last_write_time(filePath);
	}
}

void Storm::ScriptFile::update()
{
	const auto currentLastWriteTime = std::filesystem::last_write_time(_filePath);
	if (_lastWriteTime < currentLastWriteTime)
	{
		_lastWriteTime = currentLastWriteTime;
		LOG_DEBUG << "Script file " << _filePath << " was updated. The script inside will be read and sent.";

		const std::size_t fileSize = std::filesystem::file_size(_filePath);
		if (fileSize > 0)
		{
			std::string scriptContent;
			Storm::resize_hijack(scriptContent, fileSize);

			std::ifstream scriptContentStream{ _filePath };
			scriptContentStream.read(scriptContent.data(), fileSize);

			Storm::ScriptManager::instance().executeScript_ScriptThread(scriptContent);
		}
		else
		{
			LOG_DEBUG << "The script file is empty, we'll skip it.";
		}
	}
}

void Storm::ScriptFile::invalidate()
{
	_lastWriteTime = std::filesystem::file_time_type{ std::filesystem::file_time_type::duration::zero() };
}
