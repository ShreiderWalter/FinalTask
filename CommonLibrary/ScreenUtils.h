#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include <Windows.h>
#include <iostream>
#include <string>
#include <memory>
#include <tchar.h>
#include <Sddl.h>
#include <Wtsapi32.h>
#include <gdiplus.h>
#pragma comment(lib, "Advapi32.lib");
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Wtsapi32.lib")

#define PIPE_NAME TEXT("\\\\.\\pipe\\Pipe")
#define BUFFER_SIZE 4403200
#define MAPPING_OBJECT_NAME TEXT("Global\\MyMappingObject")
#define EVENT_NAME TEXT("Global\\AcceptLock")
#define EVENT_WRITE_NAME TEXT("Global\\WriteLock")
#define MUTEX_NAME TEXT("Global\\MutexLock")


class ScreenUtils
{
private:
	ScreenUtils();
	ScreenUtils(const ScreenUtils &);
	~ScreenUtils();
	static int GetEncoderClsid(const WCHAR * format, CLSID * pClsid);

public:
	static HBITMAP Bytes2Bitmap(char arrData[], int iLen);
	static BYTE * hbitmapToJPEG(HBITMAP hBmpSrc, int & size_);
	static char * Bitmap2Bytes(HBITMAP hBitmap, int& len);
	static BYTE * myGetDibBits24(int & size_);
};

#endif //SCREENUTILS_H