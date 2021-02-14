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
		const std::string& getIPStr() const;
		uint32_t getIP() const;
		uint8_t getIPNumber(const std::size_t numberIndex) const;

	public:
		std::string toStdString() const;

	public:
		bool _isEnabled;

		uint16_t _port;

	private:
		union
		{
			// Only ipv4
			uint8_t _numbers[4];
			uint32_t _packedValue;
		} _ip;

		std::string _ipStrCached;
	};
}
