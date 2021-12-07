//C++ console application Code:
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include <windows.h>
int main()
{
    ::SendMessageA(HWND_BROADCAST, WM_SYSCOMMAND,  SC_MONITORPOWER,  (LPARAM)2);
    ::Sleep(200);
    LockWorkStation();
    return 0;
}
