#include "Client.h"

HINSTANCE Client::BCX_hInstance = nullptr;
int Client::BCX_ScaleX = 0;
int Client::BCX_ScaleY = 0;
std::string Client::BCX_ClassName = "";
HWND Client::Form = nullptr;
int Client::failed_error = -1;

Client::Client(boost::asio::io_service & io_service) : m_socket(io_service), m_runFlag(true)
{
}

void Client::connect(boost::asio::ip::tcp::endpoint ep)
{
	try
	{
		m_socket.connect(ep);
	}
	catch(...)
	{
		failed_error = CONNECTION_FAILED;
	}
}

void Client::run()
{
	while(m_runFlag)
	{
		write("Size\n");
		size = readSize();
		write("Buffer\n");
		read();
	}
}

size_t Client::ifRead(const boost::system::error_code & error, size_t bytes)
{
	if(error)
	{
		throw boost::enable_current_exception(std::exception("Error!"));
	}
	readBytes = bytes;
	bool found = std::find(m_buffer, m_buffer + bytes, '\n') < m_buffer + bytes;
	return found ? 0 : 1;
}

size_t Client::ifReadImage(const boost::system::error_code & error, size_t bytes)
{
	if(error)
	{
		throw boost::enable_current_exception(std::exception("Error!"));
	}
	readBytes = bytes;
	bool found = bytes >= size;
	return found ? 0 : 1;
}

HBITMAP Bytes2Bitmap(char arrData[], int iLen)
{
	PBITMAPFILEHEADER    bmfHeader;
	PBITMAPINFO    pbi;
	HDC            hDC;
	HBITMAP        hBmpRet;
	int            iRet;
	char        *lpbitmap;
	int            iSizeOfBmInfo;
	const int    iSizeOfBmfHeader = sizeof(BITMAPFILEHEADER);
 
	bmfHeader = (PBITMAPFILEHEADER) arrData;
	arrData += iSizeOfBmfHeader;
 
	iSizeOfBmInfo = bmfHeader->bfOffBits - iSizeOfBmfHeader;
	pbi = (PBITMAPINFO) arrData;
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

int GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
{
	UINT  num = 0;
	UINT  size = 0;
	
	Gdiplus::ImageCodecInfo * pImageCodecInfo = nullptr;

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

void Client::read()
{
	readBytes = 0;
	try
	{
		boost::asio::read(m_socket, boost::asio::buffer(m_buffer), boost::bind(&Client::ifReadImage, this, _1, _2));
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

		HGLOBAL mem = GlobalAlloc(GMEM_FIXED, size);
		BYTE * pmem = (BYTE *) GlobalLock(mem);
		memcpy(pmem, m_buffer, size);
		IStream * pStream;
		CreateStreamOnHGlobal(mem, FALSE, &pStream);
		Gdiplus::Bitmap * bitmap = Gdiplus::Bitmap::FromStream(pStream);
		HBITMAP image;
		bitmap->GetHBITMAP(Gdiplus::Color::AliceBlue, &image);
		this->image = image;
	}
	catch(const std::exception & error)
	{
		failed_error = CONNECTION_CORRUPTED;
	}
}

int Client::readSize()
{
	readBytes = 0;
	try
	{
		boost::asio::read(m_socket, boost::asio::buffer(m_buffer), boost::bind(&Client::ifRead, this, _1, _2));
		std::string num(m_buffer, readBytes);
		return std::stoi(num);
	}
	catch(const std::exception & error)
	{
		failed_error = CONNECTION_CORRUPTED;
	}
}

void Client::write(const std::string & msg)
{
	boost::system::error_code error;
	try
	{
		m_socket.write_some(boost::asio::buffer(msg), error);
	}
	catch(...)
	{
		failed_error = CONNECTION_FAILED;
	}
	
}

HWND Client::BCX_Form(char * Caption, int X, int Y, int W, int H, int Style, int Exstyle)
{
	HWND A;

	if (!Style)
	{
		Style = WS_MINIMIZEBOX |
			WS_CAPTION |
			WS_POPUP |
			WS_SYSMENU |
			WS_OVERLAPPEDWINDOW | 
			WS_VISIBLE;
	}

	A = CreateWindowEx(Exstyle, BCX_ClassName.c_str(), Caption,
		Style,
		X * BCX_ScaleX,
		Y * BCX_ScaleY,
		(4 + W) * BCX_ScaleX,
		(12 + H) * BCX_ScaleY,
		nullptr, (HMENU)nullptr, BCX_hInstance, nullptr);
	SendMessage(A, (UINT)WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT),
		(LPARAM)MAKELPARAM(FALSE, 0));
	return A;
}

void Client::LoadForm()
{
	Form = BCX_Form(" Monitor ", 0, 0, 1000, 500);
	Show(Form);
}