#include "SocketSetting.h"

#include "StringAlgo.h"


namespace
{
	uint8_t extractSafeIpNumber(const std::string_view &numberStr)
	{
		char* endRadix;
		const char* numCtr = numberStr.data();
		int64_t tmp = strtoll(numCtr, &endRadix, 10);

		if (endRadix != numCtr + numberStr.size())
		{
			Storm::throwException<Storm::Exception>(numberStr + " cannot be correctly parsed into a number!");
		}
		else if (tmp < 0 || tmp > std::numeric_limits<uint8_t>::max())
		{
			Storm::throwException<Storm::Exception>(numberStr + " is out of range of a normal ip number (should be between 0 and 255)");
		}

		return static_cast<uint8_t>(tmp);
	}

	void appendIp(const Storm::SocketSetting &socketSetting, std::string &inOutStr)
	{
		inOutStr += std::to_string(socketSetting._ip._numbers[0]);
		inOutStr += '.';
		inOutStr += std::to_string(socketSetting._ip._numbers[1]);
		inOutStr += '.';
		inOutStr += std::to_string(socketSetting._ip._numbers[2]);
		inOutStr += '.';
		inOutStr += std::to_string(socketSetting._ip._numbers[3]);
	}
}


Storm::SocketSetting::SocketSetting() :
	_isEnabled{ false },
	_port{ 0 },
	_ip{ ._packedValue = 0x0 },
	_timeoutMillisec{ 33 }
{

}

Storm::SocketSetting::SocketSetting(const std::string_view &ip, uint16_t port) :
	_isEnabled{ false },
	_port{ port },
	_timeoutMillisec{ 33 }
{
	this->setIP(ip);
}

bool Storm::SocketSetting::isValid() const
{
	// This is not complete, but handles major broken cases.
	return
		_ip._packedValue != 0x0 && // 0.0.0.0 shouldn't be used
		_ip._packedValue != 0xFFFFFFFF && // 255.255.255.255 shouldn't be used
		_port > 1500 // Port under 1023 are ports well known for being reserved or used for important stuffs... With some margins, 1500 seems to be a good threshold to not conflict with any sensible processes.
		;
}

void Storm::SocketSetting::setIP(const std::string_view &value)
{
	std::vector<std::string_view> numberSplitted;

	Storm::StringAlgo::split(numberSplitted, value, Storm::StringAlgo::makeSplitPredicate('.'));
	if (numberSplitted.size() != 4)
	{
		Storm::throwException<Storm::Exception>(value + " is an invalid IPv4 ip address!");
	}

	_ip._numbers[0] = extractSafeIpNumber(numberSplitted[0]);
	_ip._numbers[1] = extractSafeIpNumber(numberSplitted[1]);
	_ip._numbers[2] = extractSafeIpNumber(numberSplitted[2]);
	_ip._numbers[3] = extractSafeIpNumber(numberSplitted[3]);
}

std::string Storm::SocketSetting::getIPStr() const
{
	std::string result;
	result.reserve(15);

	appendIp(*this, result);

	return result;
}

std::string Storm::SocketSetting::toStdString() const
{
	std::string result;
	result.reserve(24);

	appendIp(*this, result);

	result += "::";
	result += std::to_string(_port);

	return result;
}
