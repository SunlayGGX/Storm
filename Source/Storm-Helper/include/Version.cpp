#include "Version.h"

#include "SerializePackage.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/lexical_cast.hpp>


namespace
{
	enum : Storm::Version::VersionNumber
	{
		k_parsingNumberOffset = static_cast<Storm::Version::VersionNumber>('0'),
	};

	enum
	{
		k_majorFocus = 24,
		k_minorFocus = 16,
		k_subminorFocus = 8,

		k_filter = 0xFF
	};

	template<unsigned int focus>
	__forceinline constexpr Storm::Version::VersionNumber extractVersionComponent(const uint32_t versionPackedValue)
	{
		return (versionPackedValue >> focus) & k_filter;
	}
}


Storm::Version::Version(const std::string_view &versionStr) :
	_value{ 0 }
{
	std::vector<std::string_view> splitted;
	splitted.reserve(3);

	boost::algorithm::split(splitted, versionStr, boost::algorithm::is_any_of("."));

	switch (splitted.size())
	{
	case 3: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[2]) - k_parsingNumberOffset) << k_subminorFocus;
	case 2: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[1]) - k_parsingNumberOffset) << k_minorFocus;
	case 1: _value |= (boost::lexical_cast<Storm::Version::VersionNumber>(splitted[0]) - k_parsingNumberOffset) << k_majorFocus;
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

	result += std::to_string(extractVersionComponent<k_majorFocus>(_value));
	result += '.';
	result += std::to_string(extractVersionComponent<k_minorFocus>(_value));
	result += '.';
	result += std::to_string(extractVersionComponent<k_subminorFocus>(_value));

	return result;
}
