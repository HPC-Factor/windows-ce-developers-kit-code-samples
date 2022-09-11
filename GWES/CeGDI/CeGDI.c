/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeGDI.c

ABSTRACT: 
  This Windows CE application shows how to create and use Graphical 
  Device Interface (GDI) objects such as pens, brushes, palettes, 
  bitmaps, and regions. It also shows how to enumerate fonts, select
  a font type, and display some text by using the selected font.

***********************************************************************/

#include "CeGDI.h"    

UINT uIDTimer = 100;          // Timer identifier
UINT uElapse = 100;           // Time-out value
 
/***********************************************************************

FUNCTION: 
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (
                          HWND hwndDlg,   // Handle to the dialog box
                          UINT uMsg,      // Message
                          WPARAM wParam,  // First message parameter
                          LPARAM lParam)  // Second message parameter
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      return TRUE;  

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
        case IDCANCEL:
          EndDialog (hwndDlg, 0);
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
      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDR_CEGDIMENU, 0);
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      // Get the command bar height.
      g_iCBHeight = CommandBar_Height (g_hwndCB);
      return 0;
      
    case WM_COMMAND:
      // If there is a running timer, then kill it.
      if (uIDTimer != 0)
      {
        KillTimer (hwnd, uIDTimer);
        uIDTimer = 0;
      }

      switch (LOWORD(wParam))
      {
        case IDM_DRAWOBJ:
        case IDM_PALETTE:
        case IDM_BITMAP:
        case IDM_LINE:
        case IDM_TEXT:
          uIDTimer = SetTimer (hwnd, LOWORD(wParam), uElapse, NULL);
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

    case WM_TIMER:
      if (wParam == IDM_DRAWOBJ)
      {
        DrawRandomObjects (hwnd);
        return 0;
      }
      
      // If there is a running timer, then kill it.
      if (uIDTimer != 0)
      {
        KillTimer (hwnd, uIDTimer);
        uIDTimer = 0;
      }

      if (wParam == IDM_PALETTE)
        DisplayPalette (hwnd);

      if (wParam == IDM_BITMAP)
        BitmapDemo (hwnd);

      if (wParam == IDM_LINE)
        DisplayLine (hwnd);

      if (wParam == IDM_TEXT)
        DisplayText (hwnd);

      return 0;

    case WM_CLOSE:
      // If there is a running timer, then kill it.
      if (uIDTimer != 0)
      {
        KillTimer (hwnd, uIDTimer);
        uIDTimer = 0;
      }
      
      LocalFree (g_lpEnumLogFont);  
      LocalFree (g_lpNewTextMetric);  
      LocalFree (g_lpintFontIndexType);  
      
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
                  WS_VISIBLE,     // Window style
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
    return FALSE;

  ShowWindow (g_hwndMain, nCmdShow);
  UpdateWindow (g_hwndMain);
  return TRUE;
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
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_CEGDIICON));
  wndclass.hInstance = hInstance;
  wndclass.hCursor = NULL;
  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wndclass.lpszMenuName = NULL;
  wndclass.lpszClassName = g_szClassName;
  
  return RegisterClass (&wndclass);
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
              LPWSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
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
// END OF CEGDI.C
  
