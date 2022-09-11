/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Toolbar.c

ABSTRACT: 
  This application shows how to register the toolbar control class, 
  create the toolbar, and add the toolbar ToolTips.

***********************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"                          

#define NUMIMAGES     11   // Number of buttons in the toolbar           
#define IMAGEWIDTH    16   // Width of the buttons in the toolbar  
#define IMAGEHEIGHT   16   // Height of the buttons in the toolbar  
#define BUTTONWIDTH   0    // Width of the button images in the toolbar
#define BUTTONHEIGHT  0    // Height of the button images in the toolbar
#define ID_TOOLBAR    2000 // Identifier of the main tool bar
#define dwTBFontStyle TBSTYLE_BUTTON | TBSTYLE_CHECK | TBSTYLE_GROUP
                           // Style of the Font toolbar
HINSTANCE hInst = NULL;    // Handle of the application instance
HWND hwndMain = NULL;      // Handle to the main window
HWND hwndTB = NULL;        // Handle to the toolbar
HWND hwndCB = NULL;        // Handle to the command bar

TCHAR szClassName[80];     // Main window class name
TCHAR szTitle[80];         // Main window name

// The TBBUTTON structure contains information the toolbar buttons.
static TBBUTTON tbButton[] =      
{
  {0, IDM_NEW,        TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {1, IDM_OPEN,       TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {2, IDM_SAVE,       TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {0, 0,              TBSTATE_ENABLED, TBSTYLE_SEP,     0, -1},
  {3, IDM_CUT,        TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {4, IDM_COPY,       TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {5, IDM_PASTE,      TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {6, IDM_PRINT,      TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {0, 0,              TBSTATE_ENABLED, TBSTYLE_SEP,     0, -1},
  {7, IDM_ABOUT,      TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {0, 0,              TBSTATE_ENABLED, TBSTYLE_SEP,     0, -1},
  {8, IDM_BOLD,       TBSTATE_ENABLED, dwTBFontStyle,   0, -1},
  {9, IDM_ITALIC,     TBSTATE_ENABLED, dwTBFontStyle,   0, -1},
  {10,IDM_UNDERLINE,  TBSTATE_ENABLED, dwTBFontStyle,   0, -1}
};

// Toolbar ToolTips
TCHAR * szToolTips[] = 
{
  TEXT("New File"), 
  TEXT("Open File"), 
  TEXT("Save File"), 
  TEXT("Cut"), 
  TEXT("Copy"), 
  TEXT("Paste"), 
  TEXT("Print"), 
  TEXT("About"), 
  TEXT("Bold"), 
  TEXT("Italic"), 
  TEXT("Underline")
};


/***********************************************************************

FUNCTION: 
  CreateToolbar

PURPOSE: 
  Registers the TOOLBAR control class and creates a toolbar.

***********************************************************************/
HWND WINAPI CreateToolbar (HWND hwnd)
{
  int iCBHeight;              // Command bar height 
  DWORD dwStyle;              // Style of the toolbar
  HWND hwndTB = NULL;         // Handle of the command bar control 
  RECT rect,                  // Contains the coordinates of the main 
                              // window's client area         
       rectTB;                // Contains the dimensions of the bounding
                              // rectangle of the toolbar control
  INITCOMMONCONTROLSEX iccex; // INITCOMMONCONTROLSEX structure
  
  iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
  iccex.dwICC = ICC_BAR_CLASSES;

  // Registers TOOLBAR control classes from the common control 
  // dynamic-link library (DLL). 
  InitCommonControlsEx (&iccex);

  //  Create the toolbar control.
  dwStyle = WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS | 
            CCS_NOPARENTALIGN;
  
  if (!(hwndTB = CreateToolbarEx (hwnd,
                                  dwStyle,
                                  (UINT) ID_TOOLBAR, 
                                  NUMIMAGES,
                                  hInst,
                                  IDB_TOOLBAR,
                                  tbButton,
                                  sizeof (tbButton) / sizeof (TBBUTTON),
                                  BUTTONWIDTH,
                                  BUTTONHEIGHT,
                                  IMAGEWIDTH,
                                  IMAGEHEIGHT,
                                  sizeof (TBBUTTON))))
  {
    return NULL;
  }
  
  // Add ToolTips to the toolbar.
  SendMessage (hwndTB, TB_SETTOOLTIPS, (WPARAM) NUMIMAGES, 
               (LPARAM) szToolTips);

  // Reposition the toolbar.
  GetClientRect (hwnd, &rect);
  GetWindowRect (hwndTB, &rectTB);
  iCBHeight = CommandBar_Height (hwndCB);
  MoveWindow (hwndTB, 
              0, 
              iCBHeight - 2, 
              rect.right - rect.left, 
              rectTB.bottom - rectTB.top,
              TRUE);

  return hwndTB;
}

/***********************************************************************

FUNCTION: 
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (
                         HWND hwndDlg,  // Handle to the dialog box
                         UINT uMsg,     // Message
                         WPARAM wParam, // First message parameter
                         LPARAM lParam) // Second message parameter
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      return TRUE;  

    case WM_COMMAND:
      switch (LOWORD (wParam))
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
LRESULT CALLBACK WndProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch (msg)
  {
    case WM_CREATE:
    {
      // Create the command bar and insert the menu.
      hwndCB = CommandBar_Create (hInst, hwnd, 1);
      CommandBar_InsertMenubar (hwndCB, hInst, IDM_MAIN_MENU, 0);
      CommandBar_AddAdornments (hwndCB, 0, 0);

      // Creates the toolbar.
      hwndTB = CreateToolbar (hwnd);
      
      return 0;
    }    

    case WM_COMMAND:
      switch (GET_WM_COMMAND_ID (wp,lp))
      {  
        case IDM_NEW:
          MessageBox (hwnd, TEXT("New File"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_OPEN:
          MessageBox (hwnd, TEXT("Open File"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_SAVE:
          MessageBox (hwnd, TEXT("Save File"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_SAVEAS:
          MessageBox (hwnd, TEXT("Save As..."), TEXT("Info"), MB_OK);
          return 0;

        case IDM_PRINT:
          MessageBox (hwnd, TEXT("Print"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_CUT:
          MessageBox (hwnd, TEXT("Cut"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_COPY:
          MessageBox (hwnd, TEXT("Copy"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_PASTE:
          MessageBox (hwnd, TEXT("Paste"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_BOLD:
          MessageBox (hwnd, TEXT("Bold"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_ITALIC:
          MessageBox (hwnd, TEXT("Italic"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_UNDERLINE:
          MessageBox (hwnd, TEXT("Underline"), TEXT("Info"), MB_OK);
          return 0;

        case IDM_ABOUT:
          DialogBox (hInst, MAKEINTRESOURCE (IDD_ABOUT), hwndMain, 
                     AboutDialogProc);
          return 0;

        case IDM_EXIT:
          SendMessage (hwnd, WM_CLOSE, 0, 0);
          return 0;
      }
      break;
  
    case WM_HELP:
      MessageBox (hwnd, TEXT("Help"), TEXT("Info"), MB_OK);
      return 0;

    case WM_CLOSE:
      CommandBar_Destroy (hwndCB);
      DestroyWindow (hwnd);
      return 0;

    case WM_DESTROY:
      PostQuitMessage (0);
      return 0;
  }
  return DefWindowProc (hwnd, msg, wp, lp);
}


/***********************************************************************

FUNCTION: 
  InitInstance

PURPOSE: 
  Creates and displays the main window.

***********************************************************************/
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  hInst = hInstance;

  hwndMain = CreateWindow (
                szClassName,    // Registered class name                 
                szTitle,        // Application window name
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
  if (!hwndMain)
    return FALSE;

  ShowWindow (hwndMain, nCmdShow);
  UpdateWindow (hwndMain);
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
  WNDCLASS wc;

  LoadString (hInstance, IDS_CLASSNAME, szClassName, 
              sizeof (szClassName) / sizeof (TCHAR));

  LoadString (hInstance, IDS_TITLE, szTitle, 
              sizeof (szTitle) / sizeof (TCHAR));

  wc.style = CS_HREDRAW | CS_VREDRAW ;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hIcon = NULL;
  wc.hInstance = hInstance;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = szClassName;
  
  return RegisterClass (&wc);
}

/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  Called by the system as the initial entry point for this application.

***********************************************************************/
int WINAPI WinMain (
                HINSTANCE hInstance,      // Handle to current instance
                HINSTANCE hPrevInstance,  // Handle to previous instance
                LPWSTR lpCmdLine,         // Pointer to command line
                int nCmdShow)             // Show state of window
{
  MSG msg;

  if (!hPrevInstance)
  {
    if (!InitApplication (hInstance))
      return FALSE; 
  }

  if (!InitInstance (hInstance, nCmdShow))
    return FALSE;

  while (GetMessage (&msg, NULL, 0,0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }

  return msg.wParam;
}
// END TOOLBAR.C




