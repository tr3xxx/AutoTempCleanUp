#include <Windows.h>
#include <shellapi.h>
#include <Shellapi.h>
#include <filesystem>
#include <string>
#include <lmcons.h>
#include <iostream>
#include <fstream>
#include <tchar.h>

using namespace std;

#define ID_TRAYICON 1
#define ID_CLEANUP 3
#define ID_INFO 4
#define ID_EXIT 5
#define IDI_ICON 101

/**
 * @brief Opens a browser window which opens the link to the GitHub repo
 */
void ShowInfo() {
    LPCWSTR url = TEXT("https://github.com/tr3xxx/AutoTempCleanUp");
    ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
}



/**
* @brief Cleans up the users temp and %temp% folder by using windows clean up tool
**/
void CleanUp() {
    string temp_path = "C:\\Windows\\Temp\\*";
    
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    string user_temp_path = "C:\\Users\\<username>\\AppData\\Local\\Temp\\*"; 
    

    // TODO: Change the loop the a replace function
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
    

    // Run the cmd command
    system(("cmd /c \"cleanmgr /sagerun:1 & del " + temp_path + " /q & del " + user_temp_path + " /q\"").c_str());
    
}

/**
* @brief Displays a context menu when the user left-clicks or right-clicks on the trayicon.
* @param hwnd the handle to the window that will be associated with the context menu
**/
void openPopUp(HWND hwnd) {
    // Create a new popup menu
    HMENU hMenu = CreatePopupMenu();

    // Add the "CleanUp Now" menu item
    wstring cleanup = L"CleanUp Now";
    AppendMenu(hMenu, MF_STRING, ID_CLEANUP, cleanup.c_str());

    // Add the "Info" menu item
    wstring info = L"Info";
    AppendMenu(hMenu, MF_STRING, ID_INFO, info.c_str());

    // Add a separator between the "Info" and "Exit" menu items
    wstring separator = L"";
    AppendMenu(hMenu, MF_SEPARATOR, 0, separator.c_str());

    // Add the "Exit" menu item
    wstring exit = L"Exit";
    AppendMenu(hMenu, MF_STRING, ID_EXIT, exit.c_str());

    // Bring the window to the foreground to ensure the menu is displayed in front of other windows
    SetForegroundWindow(hwnd);

    // Get the current cursor position
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
    POINT menuPos = pt;

    // Show the popup menu at the calculated position
    TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, menuPos.x, menuPos.y, 0, hwnd, NULL);

    // Destroy the menu to free up resources
    DestroyMenu(hMenu);
}

/**
*
* @brief Window procedure for the main window. Handles creation and destruction of the system tray icon, and events related to the icon.
* @param hwnd The handle to the window.
* @param uMsg The message identifier.
* @param wParam Additional message information.
* @param lParam Additional message information.
* @return The result of the message processing and depends on the message sent.
*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Define a static NOTIFYICONDATA variable to hold information about the tray icon
    static NOTIFYICONDATA nid;
    switch (uMsg) {
    case WM_CREATE: {
        // Initialize the system tray icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = ID_TRAYICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_USER + 1;
        // Load the icon from the resources and set it as the tray icon
        HICON hIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));
        if (hIcon != NULL) {
            nid.hIcon = hIcon;
        }
        else {
            nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
        }

        // Set the tooltip text for the tray icon and add it to the system tray
		wstring tip = L"AutoTempCleanUp";
        lstrcpy(nid.szTip, tip.c_str());
        Shell_NotifyIcon(NIM_ADD, &nid);
        break;
    }
    case WM_USER + 1: {
        // Handle system tray icon events
        switch (LOWORD(lParam)) {
            // Open the popup menu when the user left - clicks or right - clicks on the tray icon
            case WM_LBUTTONDOWN: {
				openPopUp(hwnd);
                break;
            }
            case WM_RBUTTONDOWN: {
                openPopUp(hwnd);
                break;
            }
        }

    }
    case WM_COMMAND: {
        // Handle menu commands
        switch (LOWORD(wParam)) {
            case ID_CLEANUP: {
                // Perform the cleanup operation and show a notification
                CleanUp();
                
                NOTIFYICONDATA nid = {};

                HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
                if (hIcon != NULL) {
                    nid.hIcon = hIcon;
                }
                else {
                    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPLICATION));
                }

                wstring msg = L"AutoTempCleaner cleaned up your temporary files successfully";
                wstring title = L"AutoTempCleanUp";
                lstrcpy(nid.szInfo, msg.c_str());
                lstrcpy(nid.szInfoTitle, title.c_str());

                nid.cbSize = sizeof(nid);
                nid.hWnd = hwnd;
                nid.uFlags = NIF_INFO;
                nid.dwInfoFlags = NIIF_INFO;
                Shell_NotifyIcon(NIM_ADD, &nid);

                break;
            }
            case ID_INFO: {
                // Opens the GitHub repo
                ShowInfo();
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


/**
* @brief puts the executable in the autostart after checking if its alreay in
**/
void Autostart() {
    // Get the path to the executable
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);

    // Open the Run key
    HKEY hKey;
    RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey);

    // Check if the executable path is already in the Run key
    TCHAR szValue[MAX_PATH];
    DWORD dwSize = sizeof(szValue);
    BOOL bExists = (RegQueryValueEx(hKey, TEXT("AutoTempCleanUp"), NULL, NULL, (LPBYTE)szValue, &dwSize) == ERROR_SUCCESS);

    if (!bExists)
    {
        // Add the executable path to the Run key
        RegSetValueEx(hKey, TEXT("AutoTempCleanUp"), 0, REG_SZ, (LPBYTE)szPath, (_tcslen(szPath) + 1) * sizeof(TCHAR));
    }

    // Close the key
    RegCloseKey(hKey);
}



/**
*@brief Entry point of the program. This function initializes the program by calling the Autostart and
        CleanUp functions, creates the main window, and enters the message loop to handle window messages.
*@param hInstance The handle to the current instance of the program.
*@param hPrevInstance The handle to the previous instance of the program.
*@param lpCmdLine The command-line arguments passed to the program.
*@param nCmdShow Controls how the window is to be shown.
*@return The exit code of the program.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Call the Autostart function to check and set up the program's autostart feature
    Autostart();

    // Call the CleanUp function to delete any temporary files 
    CleanUp();

    // Create the main window
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc; // Assign the function that will handle the window messages
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TempCleaner"; // Set the name of the window class
    RegisterClass(&wc); // Register the window class with the system

    // Create the main window with default settings
    HWND hwnd = CreateWindowW(L"TempCleaner", L"", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

    // Show the main window and enter the message loop
    ShowWindow(hwnd, SW_HIDE); // Hide the window
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Return the exit code of the program
    return (int)msg.wParam;
}