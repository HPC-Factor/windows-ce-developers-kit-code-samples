/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeHttp.cpp

ABSTRACT:  
  This sample application demonstrates how to create and submit a HTTP 
  request. It requests the default HTML document from the server and 
  then display it along with the HTTP transaction headers.

***********************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <wininet.h>
#include <commctrl.h>
#include "resource.h"
  
#define   ID_EDIT             100
#define   WM_PUTTEXT          WM_USER + 1

HINSTANCE g_hInst = NULL;     // Handle to the application instance
HWND g_hwndMain = NULL;       // Handle to the application main window
HWND g_hwndCB = NULL;         // Handle to the command bar
HWND g_hwndEdit = NULL;       // Handle to the Edit Control window
  
TCHAR g_szTitle[80];          // Main window name
TCHAR g_szClassName[80];      // Main window class name

TCHAR g_szCaption[MAX_PATH];  // Caption of the application main window
TCHAR g_szIAddName[MAX_PATH]; // Name and path of the current file
TCHAR g_szProxySvr[MAX_PATH]; // Name of the proxy server
                              // For example: TEXT("http://proxy:80")
BOOL g_bProxy = FALSE,
     g_bOpenURL = FALSE;


/***********************************************************************

FUNCTION: 
  GetInternetFile

PURPOSE: 
  This function demonstrates how to create and submit a HTTP request.
  It requests the default HTML document from the server and then display
  it along with the HTTP transaction headers.
 
***********************************************************************/
BOOL GetInternetFile (LPTSTR lpszServer, LPTSTR lpszProxyServer)
{
  BOOL bReturn = FALSE;

  HINTERNET hOpen = NULL, 
            hConnect = NULL, 
            hRequest = NULL;

  DWORD dwSize = 0, 
        dwFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE; 

  TCHAR szErrMsg[200];
  
  char *lpBufferA,
       *lpHeadersA;

  TCHAR *lpBufferW,
        *lpHeadersW;

  LPTSTR AcceptTypes[2] = {TEXT("*/*"), NULL}; 

 
  // Initialize the use of the Windows CE Internet functions.
  if (g_bProxy)
  {
    hOpen = InternetOpen (TEXT("CeHttp"), INTERNET_OPEN_TYPE_PROXY, 
                          lpszProxyServer, 0, 0);
  }
  else
  {
    hOpen = InternetOpen (TEXT("CeHttp"), INTERNET_OPEN_TYPE_PRECONFIG,
                          NULL, 0, 0);
  }

  if (!hOpen)
  {
    wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("InternetOpen Error"), 
              GetLastError());
    return FALSE;
  }
  
  if (g_bOpenURL)
  {
    if (!(hRequest = InternetOpenUrl (hOpen, lpszServer, NULL, 0, 
                                      INTERNET_FLAG_RELOAD, 0)))
    {
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("InternetOpenUrl Error"),
                GetLastError());
      goto exit;
    }
  }
  else
  {
    // Open a HTTP session for a given site by lpszServer. 
    if (!(hConnect = InternetConnect (hOpen, 
                                      lpszServer, 
                                      INTERNET_INVALID_PORT_NUMBER, 
                                      NULL, NULL, 
                                      INTERNET_SERVICE_HTTP, 
                                      0, 0)))
    {
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("InternetConnect Error"),
                GetLastError());
      goto exit;
    }

    // Open a HTTP request handle. 
    if (!(hRequest = HttpOpenRequest (hConnect, 
                                      TEXT("GET"), 
                                      NULL, 
                                      HTTP_VERSION, 
                                      NULL, 
                                      (LPCTSTR*)AcceptTypes, 
                                      dwFlags, 0)))
    {
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("HttpOpenRequest Error"),
                GetLastError());
      goto exit;
    }

    // Send a request to the HTTP server. 
    if (!HttpSendRequest (hRequest, NULL, 0, NULL, 0))
    {
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("HttpSendRequest Error"),
                GetLastError());
      goto exit;
    }
  }
  
  // Call HttpQueryInfo to find out the size of the headers.
  HttpQueryInfo (hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, NULL, &dwSize,
                 NULL);

  // Allocate a block of memory for lpHeadersA.
  lpHeadersA = new CHAR [dwSize];

  // Call HttpQueryInfo again to get the headers.
  if (!HttpQueryInfo (hRequest, 
                      HTTP_QUERY_RAW_HEADERS_CRLF, 
                      (LPVOID) lpHeadersA, &dwSize, NULL))
  {
    wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("HttpQueryInfo"), 
              GetLastError());
    goto exit;
  }
  else
  {
    // Clear all of the existing text in the edit control and prepare  
    // to put the new information in it.
    SendMessage (g_hwndEdit, EM_SETSEL, 0, -1);
    SendMessage (g_hwndEdit, WM_CLEAR, 0, 0);
    SendMessage (g_hwndEdit, WM_PAINT, TRUE, 0);
  }

  // Terminate headers with NULL.
  lpHeadersA [dwSize] = '\0';
  
  // Get the required size of the buffer that receives the Unicode 
  // string. 
  dwSize = MultiByteToWideChar (CP_ACP, 0, lpHeadersA, -1, NULL, 0);
  
  // Allocate a block of memory for lpHeadersW.
  lpHeadersW = new TCHAR [dwSize];

  // Convert headers from ASCII to Unicode
  MultiByteToWideChar (CP_ACP, 0, lpHeadersA, -1, lpHeadersW, dwSize);  
  
  // Put the headers in the edit control.
  SendMessage (g_hwndMain, WM_PUTTEXT, NULL, (LPARAM) lpHeadersW);
   
  // Free the blocks of memory.
  delete[] lpHeadersA;
  delete[] lpHeadersW;

  // Allocate a block of memory for lpHeadersW.
  lpBufferA = new CHAR [32000];

  do
  {
    if (!InternetReadFile (hRequest, (LPVOID)lpBufferA, 32000, &dwSize))
    {
      wsprintf(szErrMsg, TEXT("%s: %x"), TEXT("InternetReadFile Error"), 
               GetLastError());
      goto exit;
    }

    if (dwSize != 0)    
    {
      // Terminate headers with NULL.
      lpBufferA [dwSize] = '\0';                 

      // Get the required size of the buffer which receives the Unicode
      // string. 
      dwSize = MultiByteToWideChar (CP_ACP, 0, lpBufferA, -1, NULL, 0);
      
      // Allocate a block of memory for lpBufferW.
      lpBufferW = new TCHAR [dwSize];
      
      // Convert the buffer from ASCII to Unicode.
      MultiByteToWideChar (CP_ACP, 0, lpBufferA, -1, lpBufferW, dwSize);  

      // Put the buffer in the edit control.
      SendMessage (g_hwndMain, WM_PUTTEXT, NULL, (LPARAM) lpBufferW);      

      // Free the block of memory.
      delete[] lpBufferW;  
    }    
  } while (dwSize);

  // Free the block of memory.
  delete[] lpBufferA;  

  bReturn = TRUE;

exit:

  // Close the internet handles.
  if (hOpen)
  {
    if (!InternetCloseHandle (hOpen))
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), 
                GetLastError());
  }
  
  if (hConnect)
  {
    if (!InternetCloseHandle (hConnect))
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), 
                GetLastError());
  }

  if (hRequest)
  {
    if (!InternetCloseHandle (hRequest))
      wsprintf (szErrMsg, TEXT("%s: %x"), TEXT("CloseHandle Error"), 
                GetLastError());
  }

  return bReturn;
}

/***********************************************************************

FUNCTION: 
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, 
                               LPARAM lParam)  
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      return TRUE;  

    case WM_COMMAND:
      switch (LOWORD (wParam))
      {
        case IDOK:
          EndDialog (hwndDlg, IDOK);
          return TRUE;

        case IDCANCEL:
          EndDialog (hwndDlg, IDCANCEL);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

    
/***********************************************************************

FUNCTION: 
  DialogProc

PURPOSE: 
  Processes messages sent to the Internet Address dialog box window.

***********************************************************************/
BOOL CALLBACK DialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, 
                          LPARAM lParam)  
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      SetDlgItemText (hwndDlg, IDC_ADDRESS, g_szIAddName); 
      SetDlgItemText (hwndDlg, IDC_PROXY, g_szProxySvr);

      SendDlgItemMessage (hwndDlg, IDC_PROXY, WM_ENABLE, g_bProxy, 0);

      SendDlgItemMessage (hwndDlg, IDC_OPENURL, BM_SETCHECK, 
                          g_bOpenURL, 0);
      SendDlgItemMessage (hwndDlg, IDC_PROXYSERVER, BM_SETCHECK, 
                          g_bProxy, 0);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD (wParam))
      {
        case IDC_OPENURL:
          g_bOpenURL = (!g_bOpenURL) ? TRUE : FALSE;
          return TRUE;

        case IDC_PROXYSERVER:
          g_bProxy = (!g_bProxy) ? TRUE : FALSE;
          SendDlgItemMessage (hwndDlg, IDC_PROXY, WM_ENABLE, 
                              g_bProxy, 0);
          return TRUE;

        case IDOK:
          GetDlgItemText (hwndDlg, IDC_ADDRESS, g_szIAddName, 200);

          if (g_bProxy)
            GetDlgItemText (hwndDlg, IDC_PROXY, g_szProxySvr, 200);

          EndDialog (hwndDlg, IDOK);
          return TRUE;

        case IDCANCEL:
          g_szIAddName[0] = TEXT('\0');
          EndDialog (hwndDlg, IDCANCEL);
          return TRUE;
      }
      break;
  }

  return FALSE;
}


/***********************************************************************

FUNCTION: 
  WndProc

PURPOSE: 
  Processes messages sent to the main window.
                                                    
***********************************************************************/
LRESULT CALLBACK WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                          LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_CREATE:
    {  
      // Specify the edit control window style.
      DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER |
                      ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL;
     
      // Create the edit control window.
      g_hwndEdit = CreateWindow (
                TEXT("edit"),   // Class name
                NULL,           // Window text
                dwStyle,        // Window style
                0,              // X-coordinate of the upper-left corner
                0,              // Y-coordinate of the upper-left corner
                CW_USEDEFAULT,  // Width of the edit control window
                CW_USEDEFAULT,  // Height of the edit control window
                hwnd,           // Window handle of parent window
                (HMENU)ID_EDIT, // Control identifier
                g_hInst,        // Instance handle
                NULL);          // Specify NULL for this parameter when 
                                // creating a control

      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDR_CEHTTPMENU, 0);
      
      // Add the close button (X) to the command bar. 
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      return 0;
    }

    case WM_SETFOCUS :
      SetFocus (g_hwndEdit);
      return 0;

    case WM_SIZE : 
      {
        // Get the command bar height.
        int iCBHeight = CommandBar_Height (g_hwndCB); 

        // Resize the Edit Control window.
        MoveWindow (g_hwndEdit, 0, iCBHeight, LOWORD (lParam), 
                    HIWORD (lParam) - iCBHeight, TRUE);
        return 0;
      }
    
    case WM_COMMAND:
      switch (LOWORD (wParam))
      {
        case IDM_OPEN:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ADDRESS), g_hwndMain,
                     DialogProc);

          if (g_szIAddName[0] != TEXT('\0'))
          {
            // Set the cursor as an hourglass.
            SetCursor (LoadCursor (NULL, IDC_WAIT));

            // Open the internet address. 
            if (!GetInternetFile (g_szIAddName, g_szProxySvr))
            {
              // Set the cursor to the normal state.
              SetCursor (0);

              MessageBox (g_hwndMain, 
                          TEXT("Cannot open the internet address"),
                          TEXT("Error"),
                          MB_OK);
            }
            else
            {
              // Include the internet address in the window title.
              wsprintf (g_szCaption, TEXT("%s - %s"), g_szTitle, 
                        g_szIAddName);

              // Set the window title.
              SetWindowText (g_hwndMain, g_szCaption);

              // Set the cursor to the normal state.
              SetCursor (0);
            }
          }

          return 0;
        
        case IDM_ABOUT:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ABOUT), g_hwndMain, 
                     AboutDialogProc);
          return 0;
        
        case IDM_EXIT:
          SendMessage (hwnd, WM_CLOSE, 0, 0);
          return 0;
      }
      break;

    case WM_PUTTEXT:
      {
        // Append new text to the current text in the edit control.
        int iTextLength = Edit_GetSel (g_hwndEdit);
        Edit_SetSel (g_hwndEdit, iTextLength, iTextLength);
        Edit_ReplaceSel (g_hwndEdit, lParam); 
        return 0;
      }

    case WM_CLOSE:
      CommandBar_Destroy (g_hwndCB);
      DestroyWindow (hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage (0);
      return 0;
  }

  return DefWindowProc (hwnd, uMsg, wParam, lParam);
}   
  
/***********************************************************************

FUNCTION: 
  InitApplication

PURPOSE: 
  Initializes and registers a windows class.

***********************************************************************/
BOOL InitApplication (HINSTANCE hInstance)
{  
  WNDCLASS wndclass;

  wndclass.style = CS_HREDRAW | CS_VREDRAW;
  wndclass.lpfnWndProc = (WNDPROC)WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_CEHTTP));
  wndclass.hInstance = hInstance;
  wndclass.hCursor = NULL;
  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = g_szClassName;
  
  return RegisterClass (&wndclass);
}

/***********************************************************************

FUNCTION: 
  InitInstance

PURPOSE: 
  Creates and displays the main window.

***********************************************************************/
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  g_hInst = hInstance;

  g_hwndMain = CreateWindow (
                  g_szClassName,  // Registered class name                 
                  g_szTitle,      // Application window name
                  WS_OVERLAPPED,  // Window style
                  0,              // Horizontal position of the window
                  0,              // Vertical position of the window
                  CW_USEDEFAULT,  // Window width
                  CW_USEDEFAULT,  // Window height
                  NULL,           // Handle to the parent window
                  NULL,           // Handle to the menu identifier
                  hInstance,      // Handle to the application instance
                  NULL);          // Pointer to the window-creation data

  // If it failed to create the window, return FALSE.
  if (!g_hwndMain)
  {
    return FALSE;
  }

  ShowWindow (g_hwndMain, nCmdShow);
  UpdateWindow (g_hwndMain);

  return TRUE;
}


/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  Called by the system as the initial entry point for this Windows 
  CE-based application.

***********************************************************************/
int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Shows the state of the window
{
  MSG msg;
   
  // Load the window and window class name strings.
  LoadString (hInstance, IDS_TITLE, g_szTitle, 
              sizeof (g_szTitle) / sizeof (TCHAR));

  LoadString (hInstance, IDS_CLASSNAME, g_szClassName, 
              sizeof (g_szClassName) / sizeof (TCHAR));

  if (!hPrevInstance)
  {
    if (!InitApplication (hInstance))
      return 0; 
  }

  if (!InitInstance (hInstance, nCmdShow))
    return 0;
  
  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }
  
  return msg.wParam;
}
// END OF CEHTTP.CPP
  
