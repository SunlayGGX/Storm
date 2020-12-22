#include "StateFileHeader.h"

#include "SerializePackage.h"


namespace
{
	constexpr Storm::Version retrieveLatestStateVersion()
	{
		// This is the version we'll write when saving the state file.
		// Always increment it, never decrement !
		return Storm::Version{ 1, 0, 0 };
	}

	// Never modify those or retrocompatibility issues will happen.
	enum Checksum : uint64_t
	{
		k_badChecksum = 0xFFFFAABBCCDDE001,
		k_goodChecksum = 0xABCDEFAA1001FF77,
	};
}


Storm::StateFileHeader::StateFileHeader() :
	_checksumMagicWord{ Checksum::k_badChecksum },
	_stateFileVersion{ retrieveLatestStateVersion() },
	_stateFileSize{ 0 }
{

}

Storm::StateFileHeader::~StateFileHeader() = default;

bool Storm::StateFileHeader::isValid() const
{
	return _checksumMagicWord == Checksum::k_goodChecksum;
}

void Storm::StateFileHeader::serialize(Storm::SerializePackage &package)
{
	package.seekAbsolute(0);

	this->serializePreheader(package, false);
	this->serializeHeaderPart(package);
}

void Storm::StateFileHeader::endSerialize(Storm::SerializePackage &package)
{
	if (package.isSerializing())
	{
		this->serializePreheader(package, true);
	}
	else
	{
		Storm::throwException<Storm::StormException>(__FUNCTION__ " shouldn't be used when loading the state file!");
	}
}

void Storm::StateFileHeader::serializePreheader(Storm::SerializePackage &package, bool end)
{
	// Never modify this package layout inside this method or retrocompatibility issues will happen. The preheader is fixed since the first version and should never be changed.
	// If something is to be added/modified, then add/modify it inside the header part !

	if (package.isSerializing())
	{
		if (end)
		{
			// Seek just after the checksum.
			package.seekAbsolute(sizeof(uint64_t));
			_stateFileSize = static_cast<uint64_t>(std::filesystem::file_size(package.getFilePath()));
			package << _stateFileSize;

			package.seekAbsolute(0);
			_checksumMagicWord = Checksum::k_goodChecksum;
			package << _checksumMagicWord;
		}
		else
		{
			package << _checksumMagicWord << _stateFileSize << _stateFileVersion;

			_headerStreamPosition = package.getStreamPosition();
		}
	}
	else
	{
		package << _checksumMagicWord;
		if (_checksumMagicWord != Checksum::k_goodChecksum)
		{
			Storm::throwException<Storm::StormException>("Preheader serializer checksum is invalid. Something went wrong when the state file was created and it was never completed!");
		}

		package << _stateFileSize;
		const uint64_t currentFileSize = static_cast<uint64_t>(std::filesystem::file_size(package.getFilePath()));
		if (_stateFileSize != currentFileSize)
		{
			Storm::throwException<Storm::StormException>(
				"File size is not what we expected (expected : " + std::to_string(_stateFileSize) + ", current : " + std::to_string(currentFileSize) + ").\n"
				"The state file is corrupted!"
			);
		}

		package << _stateFileVersion;
		if (_stateFileVersion > retrieveLatestStateVersion())
		{
			Storm::throwException<Storm::StormException>(
				"An issue happened with the state file and the version was changed to " + Storm::toStdString(_stateFileVersion) + ".\n"
				"The state file is corrupted!"
			);
		}

		_headerStreamPosition = package.getStreamPosition();
	}
}

void Storm::StateFileHeader::serializeHeaderPart(Storm::SerializePackage &package)
{
	// This is here you can add additional data you want to save.
	// Since the package file was validated and the right version was retrieved, we can execute the right retrocompatibility code.

	package << _configFileNameUsed;

	_bodyStreamPosition = package.getStreamPosition();
}
