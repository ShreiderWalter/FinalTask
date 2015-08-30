#ifndef HELPER_H
#define HELPER_H

#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <gdiplus.h>
#include <cstdint>
#include <Gdipluspixelformats.h>
#include <Wtsapi32.h>
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib, "Wtsapi32.lib")

#include "SharedMemoryManager.h"

#define BUFFER_SIZE 1024
//#define PIPE_NAME TEXT("\\\\.\\pipe\\Pipe")

class Helper
{
private:
	SharedMemoryManager * m_manager;
	void threadProgress();
public:
	Helper();
	~Helper();
	void run();

};

#endif //HELPER_H