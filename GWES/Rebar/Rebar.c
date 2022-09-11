/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Rebar.c

ABSTRACT: 
  This Windows CE application shows how to register the rebar and 
  toolbar control classes, create a rebar with a toolbar and a combo
  box in it, and move the rebar up or down.

***********************************************************************/

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
  
#define NUMIMAGES     4     // Number of buttons in the toolbar           
#define IMAGEWIDTH    16    // Width of the buttons in the toolbar  
#define IMAGEHEIGHT   16    // Height of the buttons in the toolbar 
#define BUTTONWIDTH   0     // Width of the button images in the toolbar
#define BUTTONHEIGHT  0     // Height of the button images in the toolbar

  
#define ID_REBAR      1000  // Identifier of the rebar control
#define ID_TOOLBAR    2000  // Identifier of the button control
#define ID_COMBOBOX   2001  // Identifier of the combo box control
#define TOP           0x00    
#define BOTTOM        0x01    

HINSTANCE g_hInst;          // Handle to the application instance
HWND g_hwndCB = NULL;       // Handle to the command bar
HWND g_hwndRB = NULL;       // Handle to the rebar
HWND g_hwndMain = NULL;     // Handle to the application main window

WORD g_wSide;               // Indicates the position of the rebar
TCHAR g_szTitle[80];        // Main window name 
TCHAR g_szClassName[80];    // Main window class name
    
HWND CreateRebar (HWND);
void MoveRebar (HWND hwnd, HWND hwndRB);
LRESULT HandleMenuPopup (HMENU);
LRESULT DoNotify (HWND, WPARAM, LPARAM);
LRESULT HandleCommand (HWND, WPARAM, LPARAM);
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

// The TBBUTTON structure contains information about the toolbar 
// buttons.
TBBUTTON tbButton[] =
{
  {0, IDM_VIEW_TOP,     TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
  {1, IDM_VIEW_BOTTOM,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
  {0, 0,                TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},
  {2, IDM_HELP_ABOUT,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
  {0, 0,                TBSTATE_ENABLED, TBSTYLE_SEP,    0, 0},
  {3, IDM_FILE_EXIT,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
};

// The toolbar tips
TCHAR * szToolTips[] = 
{
  TEXT("Top"), 
  TEXT("Bottom"), 
  TEXT("About"), 
  TEXT("Exit")
};

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
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_REBAR));
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
      g_wSide = TOP;

      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDR_REBAR, 0);

      // Add the close button (X) to the command bar.
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      // Create the rebar control.
      g_hwndRB = CreateRebar (hwnd);

      return 0;

    case WM_NOTIFY:
      return DoNotify (hwnd, wParam, lParam);
   
    case WM_INITMENUPOPUP:
      HandleMenuPopup ((HMENU) wParam);
      break;
   
    case WM_COMMAND:
      HandleCommand (hwnd, wParam, lParam);
      break;

    case WM_SIZE:
      MoveRebar (hwnd, g_hwndRB);
      return 0;

    case WM_CLOSE:
      CommandBar_Destroy (g_hwndCB);
      DestroyWindow (g_hwndRB);
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
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (
                         HWND hwndDlg,    // Handle to the dialog box
                         UINT uMsg,       // Message
                         WPARAM wParam,   // First message parameter
                         LPARAM lParam)   // Second message parameter
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
  CreateRebar

PURPOSE: 
  Registers the rebar and toolbar control classes, and creates a rebar 
  with a toolbar and a combo box in it.

***********************************************************************/
HWND CreateRebar (HWND hwnd)
{
  HWND hwndRB = NULL,         // Handle to the rebar control
       hwndTB = NULL,         // Handle to the toolbar
       hwndCombo = NULL;      // Handle to the combobox control
  DWORD dwStyle;              // Window style used in CreateWindowEx
  int index;                  // Integer index
  RECT rect;                  // RECT structure
  TCHAR szString[64];         // Temperary string
  HICON hIcon;                // Handle to a icon
  REBARINFO rbi;              // Contains information that describes 
                              // rebar control characteristics
  HIMAGELIST himlRB;          // Handle to an image list
  REBARBANDINFO rbbi[2];      // Contains information that defines bands
                              // in the rebar control
  INITCOMMONCONTROLSEX iccex; // Carries information used to load the  
                              // rebar control classes

  // Initialize the INITCOMMONCONTROLSEX structure.   
  iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);

  // Load rebar and toolbar control classes.  
  iccex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;

  // Register rebar and toolbar control classes from the common control 
  // dynamic-link library (DLL). 
  InitCommonControlsEx (&iccex);

  // Create rebar control.   
  dwStyle = WS_VISIBLE | WS_BORDER | WS_CHILD | WS_CLIPCHILDREN | 
            WS_CLIPSIBLINGS | RBS_VARHEIGHT | RBS_BANDBORDERS | 
            CCS_NODIVIDER | CCS_NOPARENTALIGN; 

  if (!(hwndRB = CreateWindowEx (0, 
                                 REBARCLASSNAME, 
                                 NULL, 
                                 dwStyle,
                                 0, 
                                 0, 
                                 CW_USEDEFAULT, 
                                 100, 
                                 hwnd, 
                                 (HMENU)ID_REBAR, 
                                 g_hInst, 
                                 NULL)))
  {
    return NULL;
  }
    
  // Set the characteristics of the rebar control.
  himlRB = ImageList_Create (32, 32, ILC_COLORDDB | ILC_MASK, 1, 0);
  hIcon = LoadIcon (g_hInst, MAKEINTRESOURCE (IDI_REBAR));
  ImageList_AddIcon (himlRB, hIcon);

  rbi.cbSize = sizeof (rbi);
  rbi.fMask = RBIM_IMAGELIST;
  rbi.himl = himlRB;

  if (!SendMessage (hwndRB, RB_SETBARINFO, 0, (LPARAM)&rbi))
    return NULL;

  // Create a toolbar.
  dwStyle = WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS | 
            CCS_NOPARENTALIGN | CCS_NORESIZE;
                
  if (!(hwndTB = CreateToolbarEx (hwnd,
                                  dwStyle,
                                  (UINT) ID_TOOLBAR, 
                                  NUMIMAGES,
                                  g_hInst,
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

  // Retrieve the dimensions of the bounding rectangle of the toolbar. 
  GetWindowRect (hwndTB, &rect);

  memset (&rbbi[0], 0, sizeof (rbbi[0]));
  rbbi[0].cbSize = sizeof (REBARBANDINFO);
  rbbi[0].fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_ID
                  | RBBIM_STYLE | RBBIM_TEXT | RBBIM_BACKGROUND | 0;

  rbbi[0].cxMinChild = rect.right - rect.left + 2;
  rbbi[0].cyMinChild = rect.bottom - rect.top + 2;
  rbbi[0].cx = 250;
  rbbi[0].fStyle = RBBS_BREAK | RBBS_GRIPPERALWAYS;
  rbbi[0].wID = ID_TOOLBAR;
  rbbi[0].hwndChild = hwndTB;
  rbbi[0].lpText = TEXT("Toolbar");
  rbbi[0].hbmBack = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_BKGRD));

  // Insert the toolbar band in the rebar control. 
  SendMessage (hwndRB, RB_INSERTBAND, (WPARAM)-1, 
               (LPARAM) (LPREBARBANDINFO)&rbbi[0]);
  
  // Create a combo box.
  dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | 
            WS_CLIPCHILDREN | WS_CLIPSIBLINGS | 
            CBS_AUTOHSCROLL | CBS_DROPDOWN;

  if (!(hwndCombo = CreateWindowEx (0, 
                                    TEXT("combobox"), 
                                    NULL, 
                                    dwStyle, 
                                    0, 0, 100, 200, 
                                    hwndRB, 
                                    (HMENU)ID_COMBOBOX, 
                                    g_hInst, 
                                    NULL)))
  {
    return NULL;
  }

  // Add 10 items to the combo box.
  for (index = 0; index < 10; index++)
  {
    wsprintf (szString, TEXT("Item %d"), index + 1);
    SendMessage (hwndCombo, CB_ADDSTRING, 0, (LPARAM) szString);
  }

  // Select the first item as default.
  SendMessage (hwndCombo, CB_SETCURSEL, (WPARAM)0, 0);

  // Retrieve the dimensions of the bounding rectangle of the combo box. 
  GetWindowRect (hwndCombo, &rect);

  memset (&rbbi[1], 0, sizeof (rbbi[1]));
  rbbi[1].cbSize = sizeof (REBARBANDINFO);
  rbbi[1].fMask = RBBIM_SIZE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_ID 
                  | RBBIM_STYLE | RBBIM_TEXT | RBBIM_BACKGROUND 
                  | RBBIM_IMAGE | 0;

  rbbi[1].cxMinChild = rect.right - rect.left;
  rbbi[1].cyMinChild = rect.bottom - rect.top;
  rbbi[1].cx = 100;
  rbbi[1].fStyle = RBBS_CHILDEDGE | RBBS_FIXEDBMP | 0;
  rbbi[1].wID = ID_COMBOBOX;
  rbbi[1].hwndChild = hwndCombo;
  rbbi[1].lpText = TEXT("ComboBox");
  rbbi[1].hbmBack = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_BKGRD));
  rbbi[1].iImage = 0;

  // Insert the combo box band in the rebar control. 
  SendMessage (hwndRB, RB_INSERTBAND, (WPARAM)-1, 
               (LPARAM) (LPREBARBANDINFO)&rbbi[1]);

  // Reposition the rebar control.
  MoveRebar (hwnd, hwndRB);

  return hwndRB;
}

/***********************************************************************

FUNCTION: 
  MoveRebar

PURPOSE: 
  Moves the rebar to the top or the bottom.

***********************************************************************/
void MoveRebar (HWND hwnd, HWND hwndRB)
{
  RECT rect,          // Contains the coordinates of the main window's 
                      // client area         
      rectRB;         // Contains the dimensions of the bounding
                      // rectangle of the rebar control
  int iX, iY,         // Upper-left corner position of the bounding 
                      // rectangle of the rebar control
     iWidth, iHeight, // Width and height of the bounding rectangle 
                      // of the rebar control
     iCBHeight;       // Height of the command bar.

  // Get the height of the command bar.
  iCBHeight = CommandBar_Height (g_hwndCB);

  // Retrieve the coordinates of the main window's client area.
  GetClientRect (hwnd, &rect);

  // Retrieve the dimensions of the bounding rectangle of the rebar.
  GetWindowRect (hwndRB, &rectRB);

  switch (g_wSide)
  {
    case TOP:
    default:
      // Align the rebar along the top of the window.
      iX = 0;
      iY = iCBHeight;
      iWidth = rect.right - rect.left;
      iHeight = rect.bottom - rect.top - iCBHeight;
      break;

    case BOTTOM:
      // Align the rebar along the bottom of the window.
      iX = 0;
      iY = rect.bottom - (rectRB.bottom - rectRB.top);
      iWidth = rect.right - rect.left;
      iHeight = rectRB.bottom - rectRB.top;
      break;
  }

  // Change the position and dimensions of the rebar control. 
  MoveWindow (hwndRB, iX, iY, iWidth, iHeight, TRUE);
}


/***********************************************************************

FUNCTION: 
  DoNotify

PURPOSE: 
  Handles the WM_NOTIFY messages.

***********************************************************************/
LRESULT DoNotify (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  LPNMHDR lpNM = (LPNMHDR)lParam;

  switch (lpNM->code)
  {
    default:
      break;
  }
  return FALSE;
}


/***********************************************************************

FUNCTION: 
  HandleMenuPopup

PURPOSE: 
  Handles the WM_INITMENUPOPUP messages.

***********************************************************************/
LRESULT HandleMenuPopup (HMENU hMenu)
{
  UINT  uSelect;

  switch (g_wSide)
   {
    case TOP:
    default:
      uSelect = IDM_VIEW_TOP;
      break;

    case BOTTOM:
      uSelect = IDM_VIEW_BOTTOM;
      break;
  }

  // Check the menu item and make it a radio item. Uncheck the rest of
  // the menu items in the associated group and clear the radio-item 
  // type flag for those items.
  CheckMenuRadioItem (hMenu, IDM_VIEW_TOP, IDM_VIEW_BOTTOM, uSelect, 
                      MF_BYCOMMAND);
  return 0;
}


/***********************************************************************

FUNCTION: 
  HandleCommand

PURPOSE: 
  Handles the WM_COMMAND messages.

***********************************************************************/
LRESULT HandleCommand (HWND hwnd, WPARAM wParam, LPARAM lParam)
{
  switch (GET_WM_COMMAND_ID (wParam, lParam))
   {
    case ID_COMBOBOX:
      break;

    case IDM_VIEW_TOP:
      if (g_wSide != TOP) 
      {
        // Destroy the existing rebar.
        DestroyWindow (g_hwndRB);

        // Make sure the rebar aligns on the top.
        g_wSide = TOP;

        // Create a new rebar.
        g_hwndRB = CreateRebar (hwnd);

        // Reposition the rebar.
        MoveRebar (hwnd, g_hwndRB);
      }
      break;

    case IDM_VIEW_BOTTOM:
      if (g_wSide != BOTTOM)
      {
        // Destroy the existing rebar.
        DestroyWindow (g_hwndRB);

        // Make sure the rebar aligns on the bottom.
        g_wSide = BOTTOM;
        
        // Create a new rebar.
        g_hwndRB = CreateRebar (hwnd);
        
        // Reposition the rebar.
        MoveRebar (hwnd, g_hwndRB);
      }
      break;

    case IDM_HELP_ABOUT:
      DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ABOUTDLG), g_hwndMain, 
                 AboutDialogProc);
      break;   

    case IDM_FILE_EXIT:
      CommandBar_Destroy (g_hwndCB);
      DestroyWindow (g_hwndRB);
      DestroyWindow (hwnd);
      break;
   
    default:
      break;
   }
  return TRUE;
}
// END OF REBAR.C