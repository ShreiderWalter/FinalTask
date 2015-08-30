#ifndef SERVER_H
#define SERVER_H

#include <WinSock2.h>
#include <Shlwapi.h>
#include <Windows.h>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <boost\bind.hpp>
#include <boost\asio.hpp>
#include <boost\exception\all.hpp>
#include <algorithm>
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "SharedMemoryManager.h"

class Server 
{
private:
	class ServerSession;
	//SharedMemoryManager * manager;
	std::vector<ServerSession *> sessions;
	boost::asio::io_service & m_io_service;
	boost::asio::ip::tcp::acceptor m_acceptor;

	void bind();
	void accept(ServerSession * , const boost::system::error_code &);
	void setImage();
	int m_size;
	unsigned char * m_buffer;
	SharedMemoryManager * m_manager;

public:
	Server(boost::asio::io_service & io_service, int port);
	SharedMemoryManager * getMemoryManager() const;
	static std::mutex m_mutex;


};


#endif //SERVER_H