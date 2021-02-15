#pragma once


namespace Storm
{
	enum class NetworkApplication;

	struct EndPointIdentifier
	{
	public:
		Storm::NetworkApplication _connectedApplication;
		std::string _ipAddress;
		unsigned int _pid;
	};
}
