#include "ScreenUtils.h"

int ScreenUtils::GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
{
	UINT  num = 0;
	UINT  size = 0;
 
	Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;
 
	Gdiplus::GetImageEncodersSize(&num, &size);
	if(size == 0)
		return -1;
 
	pImageCodecInfo = (Gdiplus::ImageCodecInfo *)(malloc(size));
	if(pImageCodecInfo == nullptr)
		return -1;
 
	GetImageEncoders(num, size, pImageCodecInfo);    
 
	for(UINT j = 0; j < num; ++j)
	{
		if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}    
	}
 
	free(pImageCodecInfo);
	return -1; 
}

HBITMAP ScreenUtils::Bytes2Bitmap(char arrData[], int iLen)
{
	PBITMAPFILEHEADER bmfHeader;
	PBITMAPINFO pbi;
	HDC hDC;
	HBITMAP hBmpRet;
	int iRet;
	char *lpbitmap;
	int iSizeOfBmInfo;
	const int iSizeOfBmfHeader = sizeof(BITMAPFILEHEADER);
 
	bmfHeader = (PBITMAPFILEHEADER) arrData;
	arrData += iSizeOfBmfHeader;
 
	iSizeOfBmInfo = bmfHeader->bfOffBits - iSizeOfBmfHeader;
	pbi = (PBITMAPINFO)arrData;
	arrData += iSizeOfBmInfo;
 
	hDC = GetDC(nullptr);
	hBmpRet = CreateCompatibleBitmap(hDC,
	pbi->bmiHeader.biWidth, pbi->bmiHeader.biHeight);
 
	iRet = SetDIBits(hDC, hBmpRet, 0,
		pbi->bmiHeader.biHeight,
		arrData,
		pbi, DIB_RGB_COLORS);
 
	::ReleaseDC(nullptr, hDC);
 
	return hBmpRet;
}

BYTE * ScreenUtils::hbitmapToJPEG(HBITMAP hBmpSrc, int & size_)
{
	ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	ULONG uQuality = 33;
	Gdiplus::Bitmap * pScreenShot = new Gdiplus::Bitmap(hBmpSrc, (HPALETTE)nullptr);
    Gdiplus::EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Guid  = Gdiplus::EncoderQuality;
    encoderParams.Parameter[0].Type  = Gdiplus::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].Value = &uQuality;
	CLSID imageCLSID;
    GetEncoderClsid(L"image/jpeg", &imageCLSID);

    IStream *pStream = nullptr;
    LARGE_INTEGER liZero = {};
    ULARGE_INTEGER pos = {};
    STATSTG stg = {};
    ULONG bytesRead = 0;
    HRESULT hrRet = S_OK;

    BYTE * buffer = nullptr;
    DWORD dwBufferSize = 0;
    hrRet = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
    hrRet = pScreenShot->Save(pStream, &imageCLSID, &encoderParams) == 0 ? S_OK : E_FAIL;
    hrRet = pStream->Seek(liZero, STREAM_SEEK_SET, &pos);
    hrRet = pStream->Stat(&stg, STATFLAG_NONAME);

    buffer = new BYTE[stg.cbSize.LowPart];
    hrRet = (buffer == nullptr) ? E_OUTOFMEMORY : S_OK;
    dwBufferSize = stg.cbSize.LowPart;
    hrRet = pStream->Read(buffer, stg.cbSize.LowPart, &bytesRead);
	if(pStream)
		pStream->Release();

	size_ = dwBufferSize;

	return buffer;
}

char * ScreenUtils::Bitmap2Bytes(HBITMAP hBitmap, int& len)
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

BYTE * ScreenUtils::myGetDibBits24(int & size_)
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
