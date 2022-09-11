/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Cmdband.c

ABSTRACT: 
  This Windows CE application registers toolbar and rebar control 
  classes, creates a command band with two toolbars in it.

***********************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"  

#define ID_BAND               2000  // Command band identifier
#define ID_BAND_MENUBAR       2001  // Menu bar identifier
#define ID_BAND_TOOLBAR       2002  // Main toolbar identifier
#define ID_BAND_FONT_TOOLBAR  2003  // Font toolbar identifier
#define dwTBFontStyle   TBSTYLE_BUTTON | TBSTYLE_CHECK | TBSTYLE_GROUP
                                    // Font toolbar style
HINSTANCE hInst = NULL;             // Application instance handle
HWND hwndMain = NULL;               // Main window handle

TCHAR szClassName[80];              // Main window class name
TCHAR szTitle[80];                  // Main window name

// The TBBUTTON structure contains information about the main toolbar
// buttons.
static TBBUTTON tbButtons[] =      
{
  {0, IDM_NEW,    TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {1, IDM_OPEN,   TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {2, IDM_SAVE,   TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {0, 0,          TBSTATE_ENABLED, TBSTYLE_SEP,     0, -1},
  {3, IDM_CUT,    TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {4, IDM_COPY,   TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {5, IDM_PASTE,  TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
  {0, 0,          TBSTATE_ENABLED, TBSTYLE_SEP,     0, -1},
  {7, IDM_ABOUT,  TBSTATE_ENABLED, TBSTYLE_BUTTON,  0, -1},
};

// The TBBUTTON structure contains information about the font toolbar
// buttons.
static TBBUTTON tbFontButtons[] =  
{
  {8, IDM_BOLD,     TBSTATE_ENABLED, dwTBFontStyle, 0, -1},
  {9, IDM_ITALIC,   TBSTATE_ENABLED, dwTBFontStyle, 0, -1},
  {10,IDM_UNDERLINE,TBSTATE_ENABLED, dwTBFontStyle, 0, -1}
};

/***********************************************************************

FUNCTION: 
  CreateCmdband

PURPOSE: 
  Registers toolbar and rebar control classes, and creates a command 
  band with two toolbars in it.

***********************************************************************/
HWND WINAPI CreateCmdband (HWND hwnd)
{
  HWND hwndCBar = NULL,     // Handle to the command bar control
       hwndCBand = NULL;    // Handle to the command band control
  REBARBANDINFO rbi[3];     // REBARBANDINFO structures for the command
                            // band
  HIMAGELIST hImageList = NULL; 
                            // Handle to the image list for command band
  INITCOMMONCONTROLSEX iccex; 
                            // INITCOMMONCONTROLSEX structure
  
  iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
  iccex.dwICC = ICC_BAR_CLASSES | ICC_COOL_CLASSES;

  // Register toolbar and rebar control classes from the common control
  // dynamic-link library (DLL). 
  InitCommonControlsEx (&iccex);

  // Create the image list for the command band.
  if (!(hImageList = ImageList_LoadImage (
                  hInst, 
                  MAKEINTRESOURCE (IDB_BANDIMAGE), 
                  16, 
                  3,            
                  CLR_DEFAULT, 
                  IMAGE_BITMAP, 
                  LR_DEFAULTCOLOR)))
    return NULL;

  // Create the command band control.
  if (!(hwndCBand = CommandBands_Create (
                  hInst, 
                  hwnd, 
                  ID_BAND,         
                  RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE, 
                  hImageList)))
    return NULL;

  // REBARBANDINFO for the menu band
  rbi[0].cbSize = sizeof (REBARBANDINFO);
  rbi[0].fMask = RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE;
  rbi[0].fStyle = RBBS_CHILDEDGE | RBBS_NOGRIPPER;
  rbi[0].wID = ID_BAND_MENUBAR;
  rbi[0].cx = 150;
  
  // REBARBANDINFO for the main toolbar band
  rbi[1].cbSize = sizeof (REBARBANDINFO);
  rbi[1].fMask = RBBIM_TEXT | RBBIM_ID | RBBIM_IMAGE | RBBIM_STYLE;
  rbi[1].fStyle = RBBS_BREAK | RBBS_GRIPPERALWAYS;
  rbi[1].lpText = TEXT("Toolbar");
  rbi[1].wID = ID_BAND_TOOLBAR;
  rbi[1].iImage = 0;

  // REBARBANDINFO for the font toolbar band
  rbi[2].cbSize = sizeof (REBARBANDINFO);
  rbi[2].fMask = RBBIM_TEXT | RBBIM_ID | RBBIM_IMAGE | RBBIM_STYLE;
  rbi[2].fStyle = RBBS_GRIPPERALWAYS;
  rbi[2].lpText = TEXT("Font");
  rbi[2].wID = ID_BAND_FONT_TOOLBAR;
  rbi[2].iImage = 1;

  // Add bands to the command bands control. 
  if (!CommandBands_AddBands (hwndCBand, hInst, 3, rbi))
    return NULL;

  // Insert a menu bar into the menu command band. 
  if (hwndCBar = CommandBands_GetCommandBar (hwndCBand, 0))
    CommandBar_InsertMenubar (hwndCBar, hInst, IDM_MAIN_MENU, 0);

  // Add the buttons to the main toolbar band. 
  if (hwndCBar = CommandBands_GetCommandBar (hwndCBand, 1))
  {
    CommandBar_AddBitmap (hwndCBar, hInst, IDB_TOOLBAR, 11, 0, 0);
    CommandBar_AddButtons (
                  hwndCBar, 
                  sizeof (tbButtons) / sizeof (TBBUTTON),
                  tbButtons);
  }
  
  // Add the buttons to the font toolbar band.
  if (hwndCBar = CommandBands_GetCommandBar (hwndCBand, 2))
  {
    CommandBar_AddBitmap (hwndCBar, hInst, IDB_TOOLBAR, 11, 0, 0);
    CommandBar_AddButtons (
                  hwndCBar, 
                  sizeof (tbFontButtons) / sizeof (TBBUTTON),
                  tbFontButtons);
  }

  // Add the help and close button to the command band.
  CommandBands_AddAdornments (hwndCBand, hInst, CMDBAR_HELP, NULL);

  return hwndCBand;
}

/***********************************************************************

FUNCTION: 
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (
                     HWND hwndDlg,  // Handle to the dialog box.
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
  HWND hwndCmdband = NULL;        // Handle to the command band

  switch (msg)
  {
    case WM_CREATE:
    {
      HICON hSmallIcon = (HICON) LoadImage (
                                      hInst, 
                                      MAKEINTRESOURCE (IDI_ICON), 
                                      IMAGE_ICON,
                                      GetSystemMetrics (SM_CXSMICON),
                                      GetSystemMetrics (SM_CYSMICON),
                                      LR_DEFAULTCOLOR);

      SendMessage (hwnd, WM_SETICON, 0, (LPARAM) hSmallIcon);

      // Add command bands and command bars.
      hwndCmdband = CreateCmdband (hwnd);
      
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
      DestroyWindow (hwndCmdband);
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
                  0,              // Horizontal window position
                  0,              // Vertical window position
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

  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)WndProc;
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
// END CMDBAND.C




