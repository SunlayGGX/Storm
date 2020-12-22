#include "RecordPreHeaderSerializer.h"

#include "SerializePackage.h"
#include "Version.h"


namespace
{
	enum class RecordMagicWord : decltype(Storm::RecordPreHeader::_magicWordChecksum)
	{
		k_badMagicWord = 0x11223399,
		k_goodMagicWord = 0xABCDE7F0
	};
}


Storm::RecordPreHeaderSerializer::RecordPreHeaderSerializer() = default;

Storm::RecordPreHeaderSerializer::RecordPreHeaderSerializer(Storm::Version &version)
{
	_preHeader._recordVersion = version;
}

void Storm::RecordPreHeaderSerializer::serialize(Storm::SerializePackage &package)
{
	using MagicWordChecksumType = decltype(_preHeader._magicWordChecksum);

	const std::size_t preHeaderExpectedSize = sizeof(_preHeader._recordFileSize) + sizeof(MagicWordChecksumType) + Storm::Version::getSizeInSerializePacket();

	const bool isSaving = package.isSerializing();
	if (isSaving)
	{
		auto tmpMagicWord = static_cast<MagicWordChecksumType>(RecordMagicWord::k_badMagicWord);
		package << tmpMagicWord;
	}
	else
	{
		const std::size_t realPacketSize = std::filesystem::file_size(package.getFilePath());
		if (realPacketSize < preHeaderExpectedSize)
		{
			Storm::throwException<std::exception>(
				"The record serialized package file isn't even the size of the preheader. This is unexpected!"
			);
		}

		package << _preHeader._magicWordChecksum;
		if (_preHeader._magicWordChecksum != static_cast<MagicWordChecksumType>(RecordMagicWord::k_goodMagicWord))
		{
			Storm::throwException<std::exception>(
				"Magic word checksum when reading the record package is wrong :\n"
				"Expected : " + Storm::toStdString(RecordMagicWord::k_goodMagicWord) + "\n"
				"Current : " + Storm::toStdString(_preHeader._magicWordChecksum) + "\n"
				"This fact highlight that the record file was either corrupted or never finished to be written."
			);
		}
	}

	package << _preHeader._recordFileSize;

	if (!isSaving)
	{
		if (_preHeader._recordFileSize < preHeaderExpectedSize)
		{
			Storm::throwException<std::exception>(
				"File size shouldn't be inferior to the preHeader size. Something went wrong..."
				"Size value was " + Storm::toStdString(_preHeader._recordFileSize)
			);
		}

		const std::uintmax_t currentFileSizeInBytes = std::filesystem::file_size(package.getFilePath());
		if (_preHeader._recordFileSize != currentFileSizeInBytes)
		{
			Storm::throwException<std::exception>(
				"Wrong record file size mismatch. The file is corrupted.\n"
				"Expected : " + std::to_string(_preHeader._recordFileSize) + " bytes.\n"
				"Current : " + std::to_string(currentFileSizeInBytes) + " bytes."
			);
		}
	}

	package << _preHeader._recordVersion;
}

void Storm::RecordPreHeaderSerializer::endSerializing(Storm::SerializePackage &package)
{
	using MagicWordChecksumType = decltype(_preHeader._magicWordChecksum);

	// Seek back to the beginning of the file to overwrite the Preheader data with the real data.

	package.seekAbsolute(0);

	uint64_t realPacketSize = std::filesystem::file_size(package.getFilePath());
	MagicWordChecksumType goodMagicWord = static_cast<MagicWordChecksumType>(RecordMagicWord::k_goodMagicWord);
	package 
		<< goodMagicWord
		<< realPacketSize
		;
}

const Storm::Version& Storm::RecordPreHeaderSerializer::getRecordVersion() const noexcept
{
	return _preHeader._recordVersion;
}
