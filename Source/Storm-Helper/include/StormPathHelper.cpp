#include "StormPathHelper.h"

#include "MemoryHelper.h"


namespace
{
	template<std::size_t folderCount>
	bool checkRootCandidateHasStormFolders(const std::filesystem::path &rootPathCandidate, const std::filesystem::path(&expectedFolders)[folderCount])
	{
		std::size_t iter = 0;

		const auto expectedFoldersBeginIter = std::begin(expectedFolders);
		const auto expectedFoldersEndIter = std::end(expectedFolders);

		for (const auto &dirIter : std::filesystem::directory_iterator{ rootPathCandidate })
		{
			if (dirIter.is_directory())
			{
				const std::filesystem::path dirName = dirIter.path().stem();
				if (std::find(expectedFoldersBeginIter, expectedFoldersEndIter, dirName) != expectedFoldersEndIter)
				{
					++iter;
					if (iter == folderCount)
					{
						return true;
					}
				}
			}
		}

		return false;
	}
}


std::filesystem::path Storm::StormPathHelper::findStormRootPath(const std::filesystem::path &exeFolderPath)
{
	if (exeFolderPath.empty())
	{
		Storm::throwException<Storm::StormException>("Exe folder shouldn't be empty!");
	}

	std::filesystem::path rootPath = exeFolderPath;

	const std::filesystem::path expectedStormRootFolderName = "Storm";

	// Those are the first folder under the root we see when we package Storm.
	const std::filesystem::path folderShouldFind[] = {
		"bin",
		"Resource",
		"Config",
		"Launcher",
	};

	std::filesystem::path likelyBestRootCandidate;

	const std::filesystem::path rootDir = rootPath.root_path();

	do
	{
		const std::filesystem::path currentRootStem = rootPath.stem();

		if (currentRootStem == expectedStormRootFolderName)
		{
			if (checkRootCandidateHasStormFolders(rootPath, folderShouldFind))
			{
				return rootPath;
			}
		}
		else if (likelyBestRootCandidate.empty())
		{
			if (checkRootCandidateHasStormFolders(rootPath, folderShouldFind))
			{
				likelyBestRootCandidate = rootPath;
			}
		}

		if (rootPath == rootDir)
		{
			if (!checkRootCandidateHasStormFolders(rootPath, folderShouldFind))
			{
				if (!likelyBestRootCandidate.empty())
				{
					rootPath = likelyBestRootCandidate;
				}
				else
				{
					// Legacy hard coded in case we haven't found.
					rootPath = exeFolderPath
						.parent_path() // \bin
						.parent_path() // \Debug or \Release
						;
				}
			}

			return rootPath;
		}
		else if (rootPath.has_parent_path())
		{
			rootPath = rootPath.parent_path();
		}

	} while (true);
}

