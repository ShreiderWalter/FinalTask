#include "Server.h"

SERVICE_STATUS        g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler (DWORD);
DWORD WINAPI ServiceWorkerThread (LPVOID lpParam);

#define SERVICE_NAME  _T("FinalTask")

int _tmain (int argc, TCHAR *argv[])
{
    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {nullptr, nullptr}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
        return GetLastError ();
    }

    return 0;
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == nullptr)
    {
        return;
    }

    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle , &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T("FinalTask: ServiceMain: SetServiceStatus returned error"));
    }

    g_ServiceStopEvent = CreateEvent (nullptr, TRUE, FALSE, nullptr);
    if (g_ServiceStopEvent == nullptr)
    {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

                if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
                {
                        OutputDebugString(_T("FinalTask: ServiceMain: SetServiceStatus returned error"));
                }
                return;
    }

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T("FinalTask: ServiceMain: SetServiceStatus returned error"));
    }

    HANDLE hThread = CreateThread (nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);

    WaitForSingleObject (hThread, INFINITE);

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        OutputDebugString(_T("FinalTask: ServiceMain: SetServiceStatus returned error"));
    }
}

VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    switch (CtrlCode)
        {
     case SERVICE_CONTROL_STOP :

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            OutputDebugString(_T("FinalTask: ServiceCtrlHandler: SetServiceStatus returned error"));
        }

        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
        HANDLE hToken;
        WTSQueryUserToken(WTSGetActiveConsoleSessionId(), &hToken);
        STARTUPINFO si;
    PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        DWORD flag = CreateProcessAsUser(hToken,
                "C:\\Users\\Shindows\\Documents\\Ñ++\\FinalTask\\build\\Helper\\Debug\\Helper.exe",
                nullptr,
                nullptr,
                nullptr,
                FALSE,
                0,
                nullptr,
                nullptr,
                &si,
                &pi);
        if(!flag)
        {
                flag = GetLastError();
        }
        if(hToken)
                CloseHandle(hToken);

        boost::asio::io_service io_service;
        Server server(io_service, 8080);
        std::thread work([&]{io_service.run(); });
        work.detach();

    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        Sleep(3000);
    }

    return ERROR_SUCCESS;
}
