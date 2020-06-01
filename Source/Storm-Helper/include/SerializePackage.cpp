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

#if defined(_WIN32)
	// Hijacking is a huge optimization, but it is really dirty...
	struct StdStringHijackerProxy
	{
		uint64_t _newAskedSize;
	};
#endif
}

#if defined(_WIN32)
	// We hijack std::string _Construct
	template<>
	template<>
	void std::string::_Construct<StdStringHijackerProxy>(StdStringHijackerProxy hijacker, const StdStringHijackerProxy, input_iterator_tag)
	{
		this->reserve(hijacker._newAskedSize);
		_Mypair._Myval2._Mysize = hijacker._newAskedSize;
	}
#endif

namespace
{
	void resize_hijack(std::string &inOutStr, uint64_t newStrSize)
	{
#if defined(_WIN32)
		StdStringHijackerProxy hijacker{ newStrSize };
		inOutStr._Construct(hijacker, hijacker, std::input_iterator_tag{});
#else
		// Linux or any other platform could not have been developped their std::string the same way Windows did (variable naming, method naming, ...)
		// Since I don't have the time to check on those platform how to hijack it, I use the old resize... But this could lead to a huge overhead since
		// we're initializing the string to a value that would be overwritten just after 
		// (a useless memset of a huge string that could have been avoided since we're providing the data afterwards)...
		inOutStr.resize(newStrSize);
#endif
	}
}


Storm::SerializePackage::SerializePackage(bool isSaving, const std::string &packageFilePath) :
	_isSaving{ isSaving }
{
	if (std::filesystem::exists(packageFilePath))
	{
		int openFlag = std::ios_base::binary;
		openFlag |= (isSaving ? std::ios_base::in : std::ios_base::out);

		_file.open(packageFilePath, openFlag);
	}
	else
	{
		std::string errorMsg = packageFilePath + " doesn't exists! We cannot serialize/deserialize it!";
		LOG_ERROR << errorMsg;
		Storm::throwException<std::exception>(errorMsg);
	}


	Storm::Version currentVersion = Storm::Version::retrieveCurrentStormVersion();
	if (isSaving)
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
	if (_isSaving)
	{
		const uint64_t strSize = other.size();
		Storm::binaryWrite(_file, strSize);
		_file.write(other.c_str(), strSize);
	}
	else
	{
		uint64_t strSize;
		Storm::binaryRead(_file, strSize);

		resize_hijack(other, strSize);
		_file.read(other.data(), strSize);
	}

	return *this;
}

bool Storm::SerializePackage::isSerializing() const noexcept
{
	return _isSaving;
}
