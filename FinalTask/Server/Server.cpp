#include "Server.h"
#include <gdiplus.h>

using boost::asio::ip::tcp;

class Server::ServerSession
{
public:
	ServerSession(boost::asio::io_service & io_service);
	void start(Server *);
	tcp::socket & socket();

private:
	tcp::socket m_socket;
	char m_buffer[BUFFER_SIZE];
	int m_readBytes;

	void write(const std::string &);
	void writeImage(BYTE * image, int size);
	void threadProgress(SharedMemoryManager *);

	size_t ifRead(boost::system::error_code & error, size_t bytes);
	std::string read();
};

/** ServerBuffer implementation */
Server::ServerSession::ServerSession(boost::asio::io_service & io_service) : m_socket(io_service)
{
}

tcp::socket & Server::ServerSession::socket()
{
	return m_socket;
}

HBITMAP Bytes2Bitmap(char arrData[], int iLen)
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

int GetEncoderClsid(const WCHAR * format, CLSID * pClsid)
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

BYTE * hbitmapToJPEG(HBITMAP hBmpSrc, int & size_)
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

void Server::ServerSession::threadProgress(SharedMemoryManager * manager)
{
	while(true)
	{
		Server::m_mutex.lock();
		int size = 0;
		char * tmp = manager->read();
		size = std::atoi(tmp);
		unsigned char * buffer = (unsigned char *) manager->read();
		Server::m_mutex.unlock();

		HBITMAP image = Bytes2Bitmap((char *) buffer, size);
		buffer = hbitmapToJPEG(image, size);

		try
		{
			if(read() == "Size\n")
			{
				write(std::to_string(size).append("\n"));
			}

			if(read() == "Buffer\n")
			{
				writeImage(buffer, size);
			}
		}
		catch(const std::exception & e)
		{
			delete this;
			return;
		}
	}
}

void Server::ServerSession::start(Server * owner)
{
	std::thread thread(&ServerSession::threadProgress, this, owner->getMemoryManager());
	thread.detach();
}

size_t Server::ServerSession::ifRead(boost::system::error_code & error, size_t bytes)
{
	if(error)
	{
		throw boost::enable_current_exception(std::exception("Error!"));
	}
	m_readBytes = bytes;
	bool found = std::find(m_buffer, m_buffer + bytes, '\n') < m_buffer + bytes;
	return found ? 0 : 1;
}

std::string Server::ServerSession::read()
{
	m_readBytes = 0;
	try
	{
		boost::asio::read(m_socket, boost::asio::buffer(m_buffer), 
			boost::bind(&ServerSession::ifRead, this, _1, _2));
	}
	catch(const std::exception & e)
	{
		std::cout << "Client connection is broken!\n";
		throw;
	}

	std::string msg(m_buffer, m_readBytes);
	return msg;
}

void Server::ServerSession::writeImage(BYTE * image, int size)
{
	boost::system::error_code error;
	m_socket.write_some(boost::asio::buffer(image, size), error);
}

void Server::ServerSession::write(const std::string & msg)
{
	boost::system::error_code error;
	m_socket.write_some(boost::asio::buffer(msg), error);
}

/** Server implementation */
Server::Server(boost::asio::io_service & io_service, int port) : 
	m_io_service(io_service), m_acceptor(io_service, tcp::endpoint(tcp::v4(), port))
{
	m_manager = SharedMemoryManager::connect(MAPPING_OBJECT_NAME).get();
	bind();
}

SharedMemoryManager * Server::getMemoryManager() const
{
	return m_manager;
}

void Server::bind()
{
	ServerSession * session = new ServerSession(m_io_service);
	m_acceptor.async_accept(session->socket(), boost::bind(&Server::accept, this, session, 
		boost::asio::placeholders::error));
}

void Server::accept(ServerSession * session, const boost::system::error_code & error)
{
	if(!error)
	{
		session->start(this);
	}
	else
	{
		delete session;
	}

	bind();
}

std::mutex Server::m_mutex;


