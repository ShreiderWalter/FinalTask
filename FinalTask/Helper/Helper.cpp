#include "Helper.h"

Helper::Helper()
{
	m_manager = SharedMemoryManager::create(MAPPING_OBJECT_NAME).get();
}

Helper::~Helper()
{
}

char * Bitmap2Bytes(HBITMAP hBitmap, int& len)
{
	BITMAP bmpObj;
	HDC hDCScreen;
	int iRet;
	DWORD dwBmpSize;
 
	BITMAPFILEHEADER    bmfHeader;
	LPBITMAPINFO        lpbi;
	const DWORD dwSizeOfBmfHeader = sizeof(BITMAPFILEHEADER);
	DWORD dwSizeOfBmInfo = sizeof(BITMAPINFO);
 
	hDCScreen = GetDC(NULL);
	GetObject(hBitmap, sizeof(BITMAP), &bmpObj);
 
	HGLOBAL hDIB = GlobalAlloc(GHND, dwSizeOfBmInfo + 8);
	lpbi = (LPBITMAPINFO)GlobalLock(hDIB);
	lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 
	iRet = GetDIBits(hDCScreen, hBitmap, 0,
		(UINT)bmpObj.bmHeight,
		NULL,
		lpbi, DIB_RGB_COLORS);
 
	if(lpbi->bmiHeader.biCompression == BI_BITFIELDS)
		dwSizeOfBmInfo += 8;
 
	dwBmpSize = lpbi->bmiHeader.biSizeImage;
	HGLOBAL hDIBBmData = GlobalAlloc(GHND, dwBmpSize);
	char* lpbitmap = (char*)GlobalLock(hDIBBmData);
 
	iRet = GetDIBits(hDCScreen, hBitmap, 0,
		(UINT)bmpObj.bmHeight,
		lpbitmap,
		lpbi, DIB_RGB_COLORS);
 
	DWORD dwSizeofDIB   =    dwBmpSize + dwSizeOfBmfHeader + dwSizeOfBmInfo;
	bmfHeader.bfOffBits = (DWORD)dwSizeOfBmfHeader + (DWORD)dwSizeOfBmInfo;
	bmfHeader.bfSize = dwSizeofDIB;
	bmfHeader.bfType = 0x4D42; //BM
	bmfHeader.bfReserved1 = bmfHeader.bfReserved2 = 0;
 
	char * arrData = new char[dwSizeofDIB];
	memcpy(arrData, &bmfHeader, dwSizeOfBmfHeader);
	memcpy(arrData + dwSizeOfBmfHeader, lpbi, dwSizeOfBmInfo);
	memcpy(arrData + dwSizeOfBmfHeader + dwSizeOfBmInfo, lpbitmap, dwBmpSize);
 
	GlobalUnlock(hDIB);
	GlobalUnlock(hDIBBmData);
	GlobalFree(hDIB);
	GlobalFree(hDIBBmData);
	ReleaseDC(nullptr, hDCScreen);
 
	len = dwSizeofDIB;
	return arrData;
}

BYTE * myGetDibBits24(int & size_)
{
	int x1, y1, x2, y2, w, h;

    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    w   = x2 - x1;
    h   = y2 - y1;

    HDC     hScreen = GetDC(nullptr);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    HBITMAP hBmpSrc = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBmpSrc);
    BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);
	
	return (BYTE *)Bitmap2Bytes(hBmpSrc, size_);
}

void Helper::threadProgress()
{
	while(true)
	{
		BYTE * buffer = nullptr;
		int bufferSize = 0;
		buffer = myGetDibBits24(bufferSize);
		std::string msg = std::to_string(bufferSize);
		std::cout << "Size : " << msg << "\n";

		m_manager->wait();
		m_manager->write(msg.c_str());
		m_manager->wait();
		m_manager->write(buffer, bufferSize);
	}
}

void Helper::run()
{
	std::thread thread(&Helper::threadProgress, this);
	thread.join();
}