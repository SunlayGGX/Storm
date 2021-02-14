#pragma once

#include <boost\asio\io_service.hpp>


namespace Storm
{
	class NetworkCore
	{
	public:
		~NetworkCore();

	public:
		void initialize();

	public:
		void execute();

	private:
		boost::asio::io_service _networkService;
	};
}
