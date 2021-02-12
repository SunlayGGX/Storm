#pragma once


namespace Storm
{
	struct SocketSetting
	{
	public:
		SocketSetting();
		SocketSetting(const std::string_view &ip, uint16_t port);

	public:
		bool isValid() const;

		void setIP(const std::string_view &value);
		std::string getIPStr() const;

	public:
		std::string toStdString() const;

	public:
		bool _isEnabled;

		union
		{
			// Only ipv4
			uint8_t _numbers[4];
			uint32_t _packedValue;
		} _ip;

		uint16_t _port;
		unsigned int _timeoutMillisec;
	};
}
