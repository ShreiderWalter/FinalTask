#include "Server.h"

int main()
{
	boost::asio::io_service io_service;
	Server server(io_service, 8080);
	io_service.run();
    return 0;
}