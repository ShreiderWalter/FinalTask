#include "Helper.h"

Helper::Helper()
{
	manager = SharedMemoryManager::create(MAPPING_OBJECT_NAME).get();
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

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes
 
    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
 
    Gdiplus::GetImageEncodersSize(&num, &size);
    if(size == 0)
      return -1;  // Failure
 
    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if(pImageCodecInfo == nullptr)
      return -1;  // Failure
 
    GetImageEncoders(num, size, pImageCodecInfo);    
 
    for(UINT j = 0; j < num; ++j)
    {
      if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
      {
         *pClsid = pImageCodecInfo[j].Clsid;
         free(pImageCodecInfo);
         return j;  // Success
      }    
    }
 
    free(pImageCodecInfo);
    return -1;  // Failure
}

BYTE * myGetDibBits24(int & size_)
{
	int x1, y1, x2, y2, w, h;

    // get screen dimensions
    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    w   = x2 - x1;
    h   = y2 - y1;

    // copy screen to bitmap
    HDC     hScreen = GetDC(nullptr);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    HBITMAP hBmpSrc = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBmpSrc);
    BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

	//ULONG_PTR gdiplusToken;
 //   Gdiplus::GdiplusStartupInput gdiplusStartupInput;
 //   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	//ULONG uQuality = 33;
	//Gdiplus::Bitmap * pScreenShot = new Gdiplus::Bitmap(hBmpSrc, (HPALETTE)nullptr);
 //   Gdiplus::EncoderParameters encoderParams;
 //   encoderParams.Count = 1;
 //   encoderParams.Parameter[0].NumberOfValues = 1;
 //   encoderParams.Parameter[0].Guid  = Gdiplus::EncoderQuality;
 //   encoderParams.Parameter[0].Type  = Gdiplus::EncoderParameterValueTypeLong;
 //   encoderParams.Parameter[0].Value = &uQuality;
	//CLSID imageCLSID;
 //   GetEncoderClsid(L"image/jpeg", &imageCLSID);

 //   IStream *pStream = nullptr;
 //   LARGE_INTEGER liZero = {};
 //   ULARGE_INTEGER pos = {};
 //   STATSTG stg = {};
 //   ULONG bytesRead = 0;
 //   HRESULT hrRet = S_OK;

 //   BYTE * buffer = nullptr;  // this is your buffer that will hold the jpeg bytes
 //   DWORD dwBufferSize = 0;  // this is the size of that buffer;


 //   hrRet = CreateStreamOnHGlobal(nullptr, TRUE, &pStream);
 //   hrRet = pScreenShot->Save(pStream, &imageCLSID, &encoderParams) == 0 ? S_OK : E_FAIL;
 //   hrRet = pStream->Seek(liZero, STREAM_SEEK_SET, &pos);
 //   hrRet = pStream->Stat(&stg, STATFLAG_NONAME);

 //   // allocate a byte buffer big enough to hold the jpeg stream in memory
 //   buffer = new BYTE[stg.cbSize.LowPart];
 //   hrRet = (buffer == nullptr) ? E_OUTOFMEMORY : S_OK;
 //   dwBufferSize = stg.cbSize.LowPart;

 //   // copy the stream into memory
 //   hrRet = pStream->Read(buffer, stg.cbSize.LowPart, &bytesRead);

	////HGLOBAL mem = GlobalAlloc(GMEM_FIXED, dwBufferSize);
 //   //BYTE* pmem = (BYTE*)GlobalLock(mem);
	////memcpy(pmem, buffer, dwBufferSize);

	////Gdiplus::Bitmap * bitmap = Gdiplus::Bitmap::FromStream(pStream);
	////GlobalUnlock(mem);

	//if(pStream)
	//	pStream->Release();

	//
	////bitmap->GetHBITMAP(Gdiplus::Color::AliceBlue, &hBmpSrc);
	//size_ = dwBufferSize;
	
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

		//HGLOBAL mem = GlobalAlloc(GMEM_FIXED, bufferSize);
		//BYTE * pmem = (BYTE*)GlobalLock(mem);
		//memcpy(pmem, buffer, bufferSize);
		//IStream * pStream;
		//CreateStreamOnHGlobal(mem, FALSE, &pStream);
		//Gdiplus::Bitmap * bitmap = Gdiplus::Bitmap::FromStream(pStream);
		//CLSID pngClsid;
		//GetEncoderClsid(L"image/jpeg", &pngClsid);
		//bitmap->Save(L"C:\\Desktop.jpg", &pngClsid, NULL);

		manager->wait();
		manager->write(msg.c_str());
		manager->wait();
		manager->write(buffer, bufferSize);
	}
}

void Helper::run()
{
	std::thread thread(&Helper::threadProgress, this);
	thread.join();
}