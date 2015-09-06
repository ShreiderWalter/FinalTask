#ifndef CLIENT_H
#define CLIENT_H

#include <WinSock2.h>
#include <Windows.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <thread>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include "SharedMemoryManager.h"
#include "ScreenUtils.h"

#define CONNECTION_FAILED 1
#define CONNECTION_CORRUPTED 2

#define Show(Window) RedrawWindow(Window, 0, 0, 0); ShowWindow(Window, SW_SHOW);

class Client
{
private:
	boost::asio::ip::tcp::socket m_socket;
	char m_buffer[BUFFER_SIZE];
	bool m_runFlag;
	int readBytes;
	size_t ifRead(const boost::system::error_code & error, size_t bytes);
	size_t ifReadImage(const boost::system::error_code & error, size_t bytes);
	static HWND BCX_Form(char*, int = 0, int = 0, int = 250, int = 150, int = 0, int = 0);
	int size;
	BYTE * result;

public:
	Client(boost::asio::io_service & io_service);
	void connect(boost::asio::ip::tcp::endpoint);
	void run();
	void read();
	int readSize();
	void write(const std::string &);
	void LoadForm();

	HBITMAP image;

	static HINSTANCE BCX_hInstance;
	static int BCX_ScaleX;
	static int BCX_ScaleY;
	static std::string BCX_ClassName;
	static HWND Form;
	static int failed_error;
};

#endif //CLIENT_H