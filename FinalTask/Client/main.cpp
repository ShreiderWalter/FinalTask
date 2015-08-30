#include "Client.h"
#include "resource.h"
#include <CommCtrl.h>

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")

#pragma comment(lib, "ComCtl32.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

std::string ipString;
int portNumber = 0;
boost::asio::ip::tcp::endpoint ep;
boost::asio::io_service io_service;
Client client(io_service);

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case IDCANCEL:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;

			case IDOK:
				char buffer[1024];
				HWND editIP = GetDlgItem(hDlg, IDC_EDIT1);
				GetWindowText(editIP, buffer, 1024);
				ipString = std::string(buffer);

				HWND editPORT = GetDlgItem(hDlg, IDC_EDIT2);
				GetWindowText(editPORT, buffer, 1024);
				portNumber = std::atoi(std::string(buffer).c_str());
				SendMessage(hDlg, WM_DESTROY, 0, 0);
				break;
		}
		break;

	case WM_CLOSE:
		if(MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
		MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			DestroyWindow(hDlg);
		}
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR CmdLine, int CmdShow)
{
	HWND mDlg;
	InitCommonControls();
	mDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	ShowWindow(mDlg, CmdShow);
	BOOL ret;
	MSG dMsg;
	while((ret = GetMessage(&dMsg, 0, 0, 0)) != 0)
	{
		if(ret == -1)
		{
			return -1;
		}
		if(!IsDialogMessage(mDlg, &dMsg))
		{
			TranslateMessage(&dMsg);
			DispatchMessage(&dMsg);
		}
	}
	//todo validate port and ip 
	ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ipString), portNumber);
	client.connect(ep);

	WNDCLASS Wc;
	MSG Msg;
	client.BCX_ClassName = "Picture";
	client.BCX_ScaleX = 1;
	client.BCX_ScaleY = 1;
	client.BCX_hInstance = hInst;
	Wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	Wc.lpfnWndProc = WndProc;
	Wc.cbClsExtra = 0;
	Wc.cbWndExtra = 0;
	Wc.hInstance = hInst;
	Wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	Wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	Wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	Wc.lpszMenuName = nullptr;
	Wc.lpszClassName = client.BCX_ClassName.c_str();
	RegisterClass(&Wc);

	client.LoadForm();
	std::thread thread(&Client::run, &client);
	thread.detach();

	MSG msg = { 0 };
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		InvalidateRect(client.Form, nullptr, FALSE);

		if (Client::failed_error > 0)
		{
			int messagebox = 0;
			if (Client::failed_error == CONNECTION_FAILED)
			{
				messagebox = MessageBox(nullptr, "Connection failed!", "Error", MB_ICONERROR);
			}
			else if (Client::failed_error == CONNECTION_CORRUPTED)
			{
				messagebox = MessageBox(nullptr, "Connection corrupted!", "Error", MB_ICONERROR);
			}

			if (messagebox > 0)
			{
				switch (messagebox)
				{
				case IDOK:
					UnregisterClass(client.BCX_ClassName.c_str(), client.BCX_hInstance);
					PostQuitMessage(0);
					return 0;
				}
			}
		}
	}

	return 0;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	PAINTSTRUCT ps;
	HDC hdcDestination;

	switch (Msg)
	{
	case WM_CREATE:	
		break;
	case WM_ACTIVATE:
		break;
	case WM_PAINT:
    	PAINTSTRUCT 	ps;
    	HDC 			hdc;
    	BITMAP 			bitmap;
    	HDC 			hdcMem;
        HGDIOBJ 		oldBitmap;

    	hdc = BeginPaint(hWnd, &ps);

    	hdcMem = CreateCompatibleDC(hdc);
		oldBitmap = SelectObject(hdcMem, client.image);

        GetObject(client.image, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

    	EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		UnregisterClass(client.BCX_ClassName.c_str(), client.BCX_hInstance);
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}