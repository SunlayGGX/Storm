#include "SerializePackage.h"

#include "ThrowException.h"
#include "MemoryHelper.h"

#include "Version.h"

#include "SerializePackageCreationModality.h"


namespace
{
	template<class Type>
	void handleBinaryPacket(const bool isSaving, std::fstream &inOutStream, Type &outVal)
	{
		if (isSaving)
		{
			Storm::binaryWrite(inOutStream, outVal);
		}
		else
		{
			Storm::binaryRead(inOutStream, outVal);
		}
	}

	bool modalityIsSaving(Storm::SerializePackageCreationModality modality)
	{
		switch (modality)
		{
		case Storm::SerializePackageCreationModality::SavingNew:
		case Storm::SerializePackageCreationModality::SavingAppend:
		case Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter:
		case Storm::SerializePackageCreationModality::SavingNewPreheaderProvidedAfter:
			return true;
		case Storm::SerializePackageCreationModality::Loading:
		case Storm::SerializePackageCreationModality::LoadingManual:
			return false;

		default:
			Storm::throwException<std::exception>("Unknown serialize package creation modality");
		}
	}
}


Storm::SerializePackage::SerializePackage(Storm::SerializePackageCreationModality modality, const std::string &packageFilePath) :
	_isSaving{ modalityIsSaving(modality) },
	_filePath{ packageFilePath }
{
	if (_isSaving || std::filesystem::exists(packageFilePath))
	{
		int openFlag = std::ios_base::binary;
		openFlag |= (_isSaving ? std::ios_base::out : std::ios_base::in);

		switch (modality)
		{
		case Storm::SerializePackageCreationModality::SavingAppend:
		case Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter:
			// ate, not app.
			openFlag |= std::ios_base::ate;
			break;

		case Storm::SerializePackageCreationModality::SavingNew:
		case Storm::SerializePackageCreationModality::SavingNewPreheaderProvidedAfter:
		case Storm::SerializePackageCreationModality::Loading:
		case Storm::SerializePackageCreationModality::LoadingManual:
		default:
			break;
		}

		_file.open(packageFilePath, openFlag);

		if (!_file.is_open())
		{
			std::string errorMsg = "Unexpected error happened when trying to open " + packageFilePath + "!";
			LOG_ERROR << errorMsg;
			Storm::throwException<std::exception>(errorMsg);
		}
	}
	else
	{
		std::string errorMsg = packageFilePath + " doesn't exists! We cannot serialize/deserialize it!";
		LOG_ERROR << errorMsg;
		Storm::throwException<std::exception>(errorMsg);
	}

	if (modality != SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter &&
		modality != SerializePackageCreationModality::SavingNewPreheaderProvidedAfter &&
		modality != SerializePackageCreationModality::LoadingManual)
	{
		Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();
		if (_isSaving)
		{
			this->operator <<(currentVersion);
		}
		else
		{
			Storm::Version version;
			this->operator <<(version);

			if (version != currentVersion)
			{
				std::string errorMsg =
					"Package " + packageFilePath + " was done with a previous version (" + static_cast<std::string>(version) + ") of the application\n"
					"Therefore it isn't compatible with current version (" + static_cast<std::string>(currentVersion) + ")!";

				LOG_ERROR << errorMsg;
				Storm::throwException<std::exception>(errorMsg);
			}
		}
	}
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(char &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(int8_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(uint64_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(uint32_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(uint16_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(uint8_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(int16_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(int32_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(int64_t &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(double &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(float &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(bool &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

Storm::SerializePackage& Storm::SerializePackage::operator<<(std::string &other)
{
	handleBinaryPacket(_isSaving, _file, other);
	return *this;
}

bool Storm::SerializePackage::isSerializing() const noexcept
{
	return _isSaving;
}

const std::fstream& Storm::SerializePackage::getUnderlyingStream() const noexcept
{
	return _file;
}

std::fstream& Storm::SerializePackage::getUnderlyingStream() noexcept
{
	return _file;
}

void Storm::SerializePackage::seekAbsolute(std::size_t newPos)
{
	_file.clear();
	_file.seekp(newPos);
}

const std::string& Storm::SerializePackage::getFilePath() const noexcept
{
	return _filePath;
}

void Storm::SerializePackage::flush()
{
	_file.flush();
}

std::size_t Storm::SerializePackage::getStreamPosition()
{
	return _file.tellg();
}

std::size_t Storm::SerializePackage::getPacketSize() const
{
	return std::filesystem::file_size(_filePath);
}
