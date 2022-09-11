/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Cmdbar.c

ABSTRACT:
  This Windows CE application shows some common buttons on the 
  command bar and how to create, add bitmaps to, and destroy 
  the command bar. It also shows InsertMenubar function.

  The user can use the menu to select from 3 different sets of 
  buttons to be displayed from the 2 standard bitmaps added to 
  the command bar.

***********************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"  

HINSTANCE hInst = NULL;   // Handle to the application instance
HWND hwndMain = NULL;     // Handle to the application main window
HWND hwndCB =NULL;        // Handle to a command bar
HMENU hMenu =NULL;        // Handle to a command bar menu
DWORD dPrevMenu=NULL;     // Handle to the previously checked menu item
enum BUTTONVIEWS {STD, VIEW, MIX};
                          // Values of command bar view
BUTTONVIEWS buttonview;   // Value of current command bar view

TCHAR szClassName[80];    // Main window class name
TCHAR szTitle[80];        // Main window name
UINT uNumSmallTips = 16;  // Number of buttons in the standard toolbar
UINT uNumViewTips = 11;   // Number of buttons in the view toolbar
UINT uNumMixTips = 15;    // Number of buttons in the mixture toolbar

// The TBBUTTON structure contains information of about the standard 
// toolbar 
static TBBUTTON tbSTDButton[] = {
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1},
{STD_FILENEW,   IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_FILEOPEN,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_FILESAVE,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1}, 
{STD_CUT,       IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0,  0}, 
{STD_COPY,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_PASTE,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{STD_UNDO,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_REDOW,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{STD_FIND,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_REPLACE,   IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_DELETE,    IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{STD_PRINTPRE,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_PRINT,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{STD_PROPERTIES,IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_HELP,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}
};

// The standard toolbar tips
TCHAR * szSmallTips[] = {
NULL,                     // Skip the menu.
TEXT("STD_FILENEW"), 
TEXT("STD_FILEOPEN"), 
TEXT("STD_FILESAVE"), 
TEXT("STD_CUT"), 
TEXT("STD_COPY"), 
TEXT("STD_PASTE"), 
TEXT("STD_UNDO"), 
TEXT("STD_REDOW"), 
TEXT("STD_FIND"), 
TEXT("STD_REPLACE"), 
TEXT("STD_DELETE"), 
TEXT("STD_PRINTPRE"), 
TEXT("STD_PRINT"), 
TEXT("STD_PROPERTIES"), 
TEXT("STD_HELP")
};

// The TBBUTTON structure contains information about the view toolbar 
// buttons.
static TBBUTTON tbVIEWButton[] = {
{VIEW_SORTNAME, IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTSIZE, IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTDATE, IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTTYPE, IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{VIEW_DETAILS,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_LIST,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SMALLICONS,  
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_LARGEICONS,  
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{VIEW_PARENTFOLDER,  
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_NETCONNECT,    
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_NETDISCONNECT,  
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_NEWFOLDER,IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}
};

// The view toolbar tips
TCHAR * szViewTips[] = {
NULL,                     // Skip the menu.
TEXT("VIEW_SORTNAME"), 
TEXT("VIEW_SORTSIZE"), 
TEXT("VIEW_SORTDATE"), 
TEXT("VIEW_SORTTYPE"), 
TEXT("VIEW_DETAILS"), 
TEXT("VIEW_LIST"), 
TEXT("VIEW_SMALLICONS"), 
TEXT("VIEW_LARGEICONS"), 
TEXT("VIEW_PARENTFOLDER"), 
TEXT("VIEW_NETCONNECT"), 
TEXT("VIEW_NETDISCONNECT"), 
TEXT("VIEW_NEWFOLDER")
};

// The TBBUTTON structure contains information about the mixture toolbar
// buttons. The array tbMIXButtons contains 8 buttons from 
// IDB_STD_SMALL_COLOR and 6 from IDB_VIEW_SMALL_COLOR. The size of the
// first bitmap added with the function Commandbar_AddBitmap must be 
// added to the index value of the buttons of the second bitmap added to
// show the proper new index value of the buttons added with the second 
// bitmap.
static TBBUTTON tbMIXButton[] = {
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1}, 
{STD_FILENEW,   IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_FILEOPEN,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_FILESAVE,  IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0, -1}, 
{STD_CUT,       IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0,  0}, 
{STD_COPY,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_PASTE,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{STD_UNDO,      IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{STD_REDOW,     IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{VIEW_SORTNAME + 15, 
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTSIZE + 15, 
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTDATE + 15, 
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_SORTTYPE + 15, 
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{0,             0,        TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0, 0,  0}, 
{VIEW_DETAILS + 15,  
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}, 
{VIEW_LIST + 15,
                IDC_NONE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1}
};

// The mixture toolbar tips
TCHAR * szMIXTips[] = {
NULL,                     // Skip the menu.
TEXT("STD_FILENEW"), 
TEXT("STD_FILEOPEN"), 
TEXT("STD_FILESAVE"), 
TEXT("STD_CUT"), 
TEXT("STD_COPY"), 
TEXT("STD_PASTE"), 
TEXT("STD_UNDO"), 
TEXT("STD_REDOW"), 
TEXT("VIEW_SORTNAME + 15"), 
TEXT("VIEW_SORTSIZE + 15"), 
TEXT("VIEW_SORTDATE + 15"), 
TEXT("VIEW_SORTTYPE + 15"), 
TEXT("VIEW_DETAILS + 15"), 
TEXT("VIEW_LIST + 15")
};

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
    // Create the command bar with buttons from the bitmap 
    // IDB_STD_SMALL_COLOR.
    case WM_CREATE:
      // Create a command bar. 
      hwndCB = CommandBar_Create (hInst, hwnd, 1);

      // Add ToolTips string to the command bar. 
      CommandBar_AddToolTips (hwndCB, uNumSmallTips, szSmallTips);

      // Add 15 images to the list of button images available for use
      // in the command bar. 
      CommandBar_AddBitmap (hwndCB, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 
                            15, 16, 16);

      // Insert the menu bar into the command bar.
      CommandBar_InsertMenubar (hwndCB, hInst, IDM_MAIN_MENU, 0);

      // Add the buttons in tbSTDButton to the command bar.
      CommandBar_AddButtons (hwndCB, 
                             sizeof (tbSTDButton) / sizeof (TBBUTTON), 
                             tbSTDButton);

      // Add help, OK, and exit buttons to the command bar.
      CommandBar_AddAdornments (hwndCB, WM_HELP | CMDBAR_OK, 0);

      // Set the initial view.
      buttonview = STD;

      // Get the menu handle.
      hMenu=CommandBar_GetMenu (hwndCB, 0);

      // Set dPrevMenu to default.
      dPrevMenu= IDM_SHOW_STD;

      // Check the default menu item.
      CheckMenuItem (hMenu, IDM_SHOW_STD, MF_CHECKED);

      return 0;
  
    case WM_COMMAND:
      switch (GET_WM_COMMAND_ID (wp, lp))
      {
        case IDM_SHOW_STD:
          // If current buttonview is not STD, then change the 
          // command bar.
          if (buttonview != STD)
          {
            CommandBar_Destroy (hwndCB);
            hwndCB = CommandBar_Create (hInst, hwnd, 1);
            CommandBar_AddToolTips (hwndCB, uNumSmallTips, szSmallTips);
            CommandBar_AddBitmap (hwndCB, HINST_COMMCTRL, 
                                  IDB_STD_SMALL_COLOR, 15, 16, 16);
            CommandBar_InsertMenubar (hwndCB, hInst, IDM_MAIN_MENU, 0);
            CommandBar_AddButtons (
                              hwndCB, 
                              sizeof (tbSTDButton) / sizeof (TBBUTTON),
                              tbSTDButton);
            CommandBar_AddAdornments (hwndCB, WM_HELP | CMDBAR_OK, 0);
            SendMessage (hwnd, WM_PAINT, NULL, NULL);
            buttonview = STD;
            hMenu=CommandBar_GetMenu (hwndCB, 0);

            // Uncheck the previous menu item.
            CheckMenuItem (hMenu, dPrevMenu, MF_UNCHECKED);

            // Check the new menu item.
            CheckMenuItem (hMenu, IDM_SHOW_STD, MF_CHECKED);
          }
          return 0;

        case IDM_SHOW_VIEW:
          // If the current button view is not VIEW, then change 
          // command bar.
          if (buttonview != VIEW)
          {
            CommandBar_Destroy (hwndCB);
            hwndCB = CommandBar_Create (hInst, hwnd, 1);
            CommandBar_AddToolTips (hwndCB, uNumViewTips, szViewTips);
            CommandBar_AddBitmap (hwndCB, HINST_COMMCTRL, 
                                  IDB_VIEW_SMALL_COLOR, 12, 16, 16);
            CommandBar_InsertMenubar (hwndCB, hInst, IDM_MAIN_MENU, 0);
            CommandBar_AddButtons (
                              hwndCB, 
                              sizeof (tbVIEWButton) / sizeof (TBBUTTON),
                              tbVIEWButton);
            CommandBar_AddAdornments (hwndCB, WM_HELP | CMDBAR_OK, 0);
            SendMessage (hwnd, WM_PAINT, NULL, NULL);
            buttonview = VIEW;
            hMenu=CommandBar_GetMenu (hwndCB, 0);
            CheckMenuItem (hMenu, dPrevMenu, MF_UNCHECKED);
            CheckMenuItem (hMenu, IDM_SHOW_VIEW, MF_CHECKED);
          }
          return 0;

        case IDM_SHOW_MIX:
          // If the current button view is not MIX, then change the 
          // command bar.
          if (buttonview != MIX)
          {
            CommandBar_Destroy (hwndCB);
            hwndCB = CommandBar_Create (hInst, hwnd, 1);
            CommandBar_AddToolTips (hwndCB, uNumMixTips, szMIXTips);
            CommandBar_AddBitmap (hwndCB, HINST_COMMCTRL, 
                                  IDB_STD_SMALL_COLOR, 15, 16, 16);
            CommandBar_AddBitmap (hwndCB, HINST_COMMCTRL, 
                                  IDB_VIEW_SMALL_COLOR, 12, 16, 16);
            CommandBar_InsertMenubar (hwndCB, hInst, IDM_MAIN_MENU, 0);
            CommandBar_AddButtons (
                              hwndCB, 
                              sizeof (tbMIXButton) / sizeof (TBBUTTON), 
                              tbMIXButton);
            CommandBar_AddAdornments (hwndCB, WM_HELP | CMDBAR_OK, 0);
            SendMessage (hwnd, WM_PAINT, NULL, NULL);
            buttonview = MIX;
            hMenu=CommandBar_GetMenu (hwndCB, 0);
            CheckMenuItem (hMenu, dPrevMenu, MF_UNCHECKED);
            CheckMenuItem (hMenu, IDM_SHOW_MIX, MF_CHECKED);
          }
          return 0;

        case IDM_EXIT:
          SendMessage (hwnd, WM_CLOSE, 0, 0);
          return 0;
      }
      break;
  
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
                  0,              // Horizontal position of window
                  0,              // Vertical position of window
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
            HINSTANCE hInstance,      // Handle to the current instance
            HINSTANCE hPrevInstance,  // Handle to the previous instance
            LPWSTR lpCmdLine,         // Pointer to the command line
            int nCmdShow)             // Show state of the window
{
  MSG msg;

  if (!hPrevInstance)
  {
    if (!InitApplication (hInstance))
      return FALSE; 
  }

  if (!InitInstance (hInstance, nCmdShow))
    return FALSE;

  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }

  return msg.wParam;
}
// END CMDBAR.CPP

