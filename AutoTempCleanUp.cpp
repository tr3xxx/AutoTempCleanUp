#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <lmcons.h>
#include <iostream>

using namespace std;

#define ID_TRAYICON 1
#define ID_AUTOSTART 2
#define ID_CLEANUP 3
#define ID_INFO 4
#define ID_EXIT 5

// Function to display a message box with information about the program
void ShowInfo(HWND hwnd) {
    LPCWSTR url = TEXT("https://github.com/tr3xxx");
    ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
}


// Function to perform the cleanup operation
void CleanUp() {
    
    string temp_path = "C:\\Windows\\Temp\\*";
    
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    string user_temp_path = "C:\\Users\\<username>\\AppData\\Local\\Temp\\*"; 
    
    if (GetUserNameA(username, &username_len))
    {
		for (int i = 0; i < user_temp_path.length(); i++)
		{
			if (user_temp_path[i] == '<')
			{
				user_temp_path.replace(i, 9, username);
			}
		}
    }
    
    system(("cmd /c \"cleanmgr /sagerun:1 & del " + temp_path + " /q & del " + user_temp_path + " /q\"").c_str());
}

// Function to add or remove the program from autostart
void SetAutoStart(bool enable) {
    HKEY hkey;
    wstring keypath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    wstring keyname = L"TempCleaner";
    wstring keyvalue = L"C:\\TempCleaner\\TempCleaner.exe"; // replace this with the actual path to the executable file

    if (RegOpenKeyEx(HKEY_CURRENT_USER, keypath.c_str(), 0, KEY_ALL_ACCESS, &hkey) == ERROR_SUCCESS) {
        if (enable) {
            RegSetValueEx(hkey, keyname.c_str(), 0, REG_SZ, (LPBYTE)keyvalue.c_str(), (DWORD)(keyvalue.size() + 1));
        }
        else {
            RegDeleteValue(hkey, keyname.c_str());
        }
        RegCloseKey(hkey);
    }
}

// Window procedure for the main window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static NOTIFYICONDATA nid;
    switch (uMsg) {
    case WM_CREATE: {
        // Initialize the system tray icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = ID_TRAYICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        
       // nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
        HICON hIcon = static_cast<HICON>(LoadImage(NULL, L"icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
        if (hIcon != NULL) {
            nid.hIcon = hIcon;
        }
        else {
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
        }

		wstring tip = L"TempCleaner";
        lstrcpy(nid.szTip, tip.c_str());
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    }
    case WM_USER + 1: {
        // Handle system tray icon events
        switch (LOWORD(lParam)) {
            case WM_LBUTTONDOWN | WM_RBUTTONDOWN: {
                // Display a context menu when the user left-clicks or right-clicks on the icon
                HMENU hMenu = CreatePopupMenu();
                wstring autostart = L"Autostart";
                AppendMenu(hMenu, MF_STRING, ID_AUTOSTART, autostart.c_str());
                wstring cleanup = L"CleanUp Now";
                AppendMenu(hMenu, MF_STRING, ID_CLEANUP, cleanup.c_str());
                wstring info = L"Info";
                AppendMenu(hMenu, MF_STRING, ID_INFO, info.c_str());
                wstring separator = L"";
                AppendMenu(hMenu, MF_SEPARATOR, 0, separator.c_str());
                wstring exit = L"Exit";
                AppendMenu(hMenu, MF_STRING, ID_EXIT, exit.c_str());
                SetForegroundWindow(hwnd);
                POINT pt;
                GetCursorPos(&pt);

                // Create a NOTIFYICONIDENTIFIER structure for the tray icon
                NOTIFYICONIDENTIFIER nid;
                nid.cbSize = sizeof(NOTIFYICONIDENTIFIER);
                nid.hWnd = hwnd;
                nid.uID = ID_TRAYICON;

                // Get the bounding rectangle of the taskbar button associated with the tray icon
                RECT taskbarRect;
                Shell_NotifyIconGetRect(&nid, &taskbarRect);

                // Calculate the position of the menu above the tray icon
                POINT menuPos;
                menuPos.x = taskbarRect.left + (taskbarRect.right - taskbarRect.left) / 2;
                menuPos.y = taskbarRect.top + (taskbarRect.bottom - taskbarRect.top) / 2;
               
				printf("menuPosX", menuPos.x);
				printf("menuPosY", menuPos.y);

                // Show the popup menu
                TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, menuPos.x, menuPos.y, 0, hwnd, NULL);

                DestroyMenu(hMenu);
                break;
            }
        }

    }
    case WM_COMMAND: {
        // Handle menu commands
        switch (LOWORD(wParam)) {
            case ID_AUTOSTART: {
                // Add or remove the program from autostart
                bool enabled = !IsDlgButtonChecked(hwnd, ID_AUTOSTART);
                SetAutoStart(enabled);
                CheckDlgButton(hwnd, ID_AUTOSTART, enabled ? BST_CHECKED : BST_UNCHECKED);
                break;
            }
            case ID_CLEANUP: {
                // Perform the cleanup operation
                CleanUp();
			    wstring msg = L"TempCleaner has cleaned up your temp files.";
			    wstring title = L"TempCleaner";
                MessageBox(hwnd, msg.c_str(),title.c_str(), MB_OK | MB_ICONINFORMATION);
                break;
            }
            case ID_INFO: {
                // Show information about the program
                ShowInfo(hwnd);
                break;
            }
            case ID_EXIT: {
                // Exit the program
                Shell_NotifyIcon(NIM_DELETE, &nid);
                PostQuitMessage(0);
                break;
            }
        }
        break;
    }
    case WM_DESTROY: {
        // Remove the system tray icon and exit the message loop
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point of the program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the main window
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TempCleaner";
    RegisterClass(&wc);
    HWND hwnd = CreateWindowW(L"TempCleaner", L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
    // Show the main window and enter the message loop
    ShowWindow(hwnd, SW_HIDE);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}