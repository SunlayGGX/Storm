#include "BibliographyManager.h"

#include "SingletonHolder.h"
#include "IConfigManager.h"

#include "InternalReferenceConfig.h"

#include <fstream>


Storm::BibliographyManager::BibliographyManager() = default;
Storm::BibliographyManager::~BibliographyManager() = default;

void Storm::BibliographyManager::generateBibTexLibrary() const
{
	const Storm::IConfigManager &configMgr = Storm::SingletonHolder::instance().getSingleton<Storm::IConfigManager>();
	const std::filesystem::path libFilePath = std::filesystem::path{ configMgr.getTemporaryPath() } / "Library" / "bibliography.bib";

	std::filesystem::create_directories(libFilePath.parent_path());

	std::ofstream libFile{ libFilePath };
	std::ostreambuf_iterator<char> ofIter{ libFile };

	std::size_t count = 0;

	const std::vector<Storm::InternalReferenceConfig> &references = configMgr.getInternalReferencesConfig();
	for (const Storm::InternalReferenceConfig &reference : references)
	{
		if (!reference._bibTexLink.empty())
		{
			const std::filesystem::path bibTexLinkPath{ reference._bibTexLink };
			if (!std::filesystem::is_regular_file(bibTexLinkPath))
			{
				Storm::throwException<Storm::Exception>("'" + reference._bibTexLink + "' doesn't exists or isn't a valid file!");
			}

			std::copy(std::istreambuf_iterator<char>{ std::ifstream{ bibTexLinkPath } }, std::istreambuf_iterator<char>{}, ofIter);

			ofIter = '\n';
			++ofIter;

			ofIter = '\n';
			++ofIter;

			++count;
		}
	}

	LOG_COMMENT << "Successfully wrote " << count << " library references to " << libFilePath;
}
