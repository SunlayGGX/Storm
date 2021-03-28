#include "Version.h"

#include "SerializePackage.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>


Storm::Version::Version(const std::string_view &versionStr) :
	_value{ 0 }
{
	enum : Storm::Version::VersionNumber
	{
		k_parsingNumberOffset = static_cast<Storm::Version::VersionNumber>('0')
	};

	std::vector<std::string_view> splitted;
	splitted.reserve(3);

	boost::algorithm::split(splitted, versionStr, boost::algorithm::is_any_of("."));

	switch (splitted.size())
	{
	case 3: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[2]) - k_parsingNumberOffset) << 8;
	case 2: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[1]) - k_parsingNumberOffset) << 16;
	case 1: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[0]) - k_parsingNumberOffset) << 24;
	case 0:
		break;

	default: Storm::throwException<Storm::Exception>(versionStr + " isn't a valid version!");
	}
}

bool Storm::Version::operator==(const std::string_view &versionStr) const
{
	return *this == Storm::Version{ versionStr };
}

bool Storm::Version::operator<(const std::string_view &versionStr) const
{
	return *this < Storm::Version{ versionStr };
}

void Storm::Version::serialize(Storm::SerializePackage &package)
{
	package << _value;
}

std::size_t Storm::Version::getSizeInSerializePacket()
{
	return sizeof(Storm::Version::_value);
}

Storm::Version& Storm::Version::operator=(const std::string_view &versionStr)
{
	return *this = Storm::Version{ versionStr };
}

Storm::Version::operator std::string() const
{
	std::string result;
	result.reserve(8);

	result += std::to_string(_value);
	result += '.';
	result += std::to_string(_value);
	result += '.';
	result += std::to_string(_value);

	return result;
}
