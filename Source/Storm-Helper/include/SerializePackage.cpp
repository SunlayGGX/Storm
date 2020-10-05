#include "SerializePackage.h"

#include "ThrowException.h"
#include "MemoryHelper.h"

#include "Version.h"


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
}


Storm::SerializePackage::SerializePackage(Storm::SerializePackageCreationModality modality, const std::string &packageFilePath) :
	_isSaving{ modality != SerializePackageCreationModality::Loading },
	_filePath{ packageFilePath }
{
	if (std::filesystem::exists(packageFilePath))
	{
		int openFlag = std::ios_base::binary;
		openFlag |= (_isSaving ? std::ios_base::in : std::ios_base::out);

		switch (modality)
		{
		case Storm::SerializePackageCreationModality::SavingAppend:
		case Storm::SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter:
			// ate, not app.
			openFlag |= std::ios_base::ate;
			break;

		case Storm::SerializePackageCreationModality::SavingNew:
		case Storm::SerializePackageCreationModality::Loading:
		default:
			break;
		}

		_file.open(packageFilePath, openFlag);
	}
	else
	{
		std::string errorMsg = packageFilePath + " doesn't exists! We cannot serialize/deserialize it!";
		LOG_ERROR << errorMsg;
		Storm::throwException<std::exception>(errorMsg);
	}

	if (modality != SerializePackageCreationModality::SavingAppendPreheaderProvidedAfter)
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
