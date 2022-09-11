/***********************************************************************
   
THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  ListView.c

ABSTRACT: 
  This Windows CE application shows how to register list view and 
  header control classes, and create a list view control.

***********************************************************************/
#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"

#define ITEM_COUNT  100   // Limit of number of items in the list view
#define ID_LISTVIEW 2000  // Identifier of the list view control

HINSTANCE g_hInst;        // Handle to the application instance
HWND g_hwndCB = NULL;     // Handle to the commandbar
HWND g_hwndLV = NULL;     // Handle to the list view control
HWND g_hwndMain = NULL;   // Handle to the application main window

TCHAR g_szTitle[80];      // Main window name
TCHAR g_szClassName[80];  // Main window class name

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDialogProc (HWND, UINT, WPARAM, LPARAM);

HWND CreateListView (HINSTANCE hInstance, HWND hwndParent);
BOOL InitListView (HWND hwndListView);
void UpdateMenu (HWND, HMENU);
void SwitchView (HWND, DWORD);
void ResizeListView (HWND, HWND);
LRESULT ListViewNotify (HWND, LPARAM);


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
  wndclass.lpfnWndProc = (WNDPROC) WndProc;
  wndclass.cbClsExtra = 0;
  wndclass.cbWndExtra = 0;
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_MAINICON));
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
      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDM_MAIN_MENU, 0);
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      // Create the list view control.
      g_hwndLV = CreateListView (g_hInst, hwnd);
      
      // Initialize the list view control.
      InitListView (g_hwndLV);
      
      return 0;

    case WM_NOTIFY:
      return ListViewNotify (hwnd, lParam);
   
    case WM_SIZE:
      ResizeListView (g_hwndLV, hwnd);
      return 0;

    case WM_INITMENUPOPUP:
      UpdateMenu (g_hwndLV, CommandBar_GetMenu (g_hwndCB, 0));
      break;
   
    case WM_COMMAND:
      switch (GET_WM_COMMAND_ID (wParam, lParam))
      {
        case IDM_LARGE_ICONS:
          SwitchView (g_hwndLV, LVS_ICON);
          return 0;
         
        case IDM_SMALL_ICONS:
          SwitchView (g_hwndLV, LVS_SMALLICON);
          return 0;
         
        case IDM_LIST:
          SwitchView (g_hwndLV, LVS_LIST);
          return 0;
         
        case IDM_REPORT:
          SwitchView (g_hwndLV, LVS_REPORT);
          return 0;
         
        case IDM_EXIT:
          SendMessage (hwnd, WM_CLOSE, 0, 0);
          return 0;
         
        case IDM_ABOUT:
          DialogBox (g_hInst, MAKEINTRESOURCE(IDD_ABOUT), hwnd, 
                     AboutDialogProc);
          return 0;
      }
      break;

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
  CreateListView

PURPOSE: 
  Registers list view and header control classes, and creates a list 
  view control.

***********************************************************************/
HWND CreateListView (HINSTANCE hInstance, HWND hwndParent)
{
  DWORD dwStyle;              // Window style of the list view control
  HWND hwndListView;          // Handle to the list view control
  HIMAGELIST himlSmall;       // Handle to the small image list
  HIMAGELIST himlLarge;       // Handle to the large image list
  INITCOMMONCONTROLSEX iccex; // INITCOMMONCONTROLSEX structure

  // Initialize the INITCOMMONCONTROLSEX structure.   
  iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);

  // Load the list view and header control classes.  
  iccex.dwICC = ICC_LISTVIEW_CLASSES;  

  // Register the list view control and header classes from the common  
  // control dynamic-link library (DLL). 
  InitCommonControlsEx (&iccex);

  // Assign the list view window style.
  dwStyle = WS_TABSTOP | WS_CHILD | WS_BORDER | WS_VISIBLE | 
            LVS_AUTOARRANGE | LVS_REPORT | LVS_OWNERDATA;

  // Create the list view control.        
  hwndListView = CreateWindowEx (
            WS_EX_CLIENTEDGE,   // Extended window style
            WC_LISTVIEW,        // Class name
            TEXT(""),           // Window name
            dwStyle,            // Window style
            0,                  // Horizontal position of the window
            0,                  // Vertical position of the window
            0,                  // Window width
            0,                  // Window height
            hwndParent,         // Handle to the parent window
            (HMENU)ID_LISTVIEW, // Handle to the menu identifier
            g_hInst,            // Handle to the application instance
            NULL);              // Window-creation data

  // If it fails in creating the window, return NULL.
  if (!hwndListView)
    return NULL;

  // Resize the list view window.
  ResizeListView (hwndListView, hwndParent);

  // Create the large and small image lists.
  himlSmall = ImageList_Create (16, 16, ILC_COLOR | ILC_MASK, 1, 0);
  himlLarge = ImageList_Create (32, 32, ILC_COLOR | ILC_MASK, 1, 0);

  if (himlSmall && himlLarge)
  {
    HICON hIcon;

    // Load the small icon from the instance.
    hIcon = LoadImage (g_hInst, MAKEINTRESOURCE(IDI_DISK), IMAGE_ICON, 
                       16, 16, LR_DEFAULTCOLOR);

    // Add the icon to the image list.
    ImageList_AddIcon (himlSmall, hIcon);

    // Load the small icon from the instance.
    hIcon = LoadIcon (g_hInst, MAKEINTRESOURCE(IDI_DISK));

    // Add the icon to the image list.
    ImageList_AddIcon (himlLarge, hIcon);

    // Assign the large and small image lists to the list view control.
    ListView_SetImageList (hwndListView, himlSmall, LVSIL_SMALL);
    ListView_SetImageList (hwndListView, himlLarge, LVSIL_NORMAL);
  }

  return hwndListView;
}


/***********************************************************************

FUNCTION: 
  InitListView

PURPOSE: 
  Initilaizes the list view control. 

***********************************************************************/
BOOL InitListView (HWND hwndListView)
{
  int index;                      
  LV_COLUMN lvColumn;        
  TCHAR szString[5][20] = {TEXT("Main Column"), 
                           TEXT("Column 1"), 
                           TEXT("Column 2"), 
                           TEXT("Column 3"), 
                           TEXT("Column 4")};

  // Empty the list in list view.
  ListView_DeleteAllItems (hwndListView);

  // Initialize the columns in the list view.
  lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvColumn.fmt = LVCFMT_LEFT;
  lvColumn.cx = 120;

  // Insert the five columns in the list view.
  for (index = 0; index < 5; index++)
  {
    lvColumn.pszText = szString[index];
    ListView_InsertColumn (hwndListView, index, &lvColumn);
  }

  // Set the number of items in the list to ITEM_COUNT.
  ListView_SetItemCount (hwndListView, ITEM_COUNT);

  return TRUE;
}


/***********************************************************************

FUNCTION: 
  ResizeListView

PURPOSE: 
  Resizes the list view control.

***********************************************************************/
void ResizeListView (HWND hwndListView, HWND hwndParent)
{
  RECT rect;

  GetClientRect (hwndParent, &rect);

  MoveWindow (hwndListView, 
              rect.left,
              rect.top + CommandBar_Height (g_hwndCB),
              rect.right - rect.left,
              rect.bottom - rect.top - CommandBar_Height (g_hwndCB),
              TRUE);
}

/***********************************************************************

FUNCTION: 
  ListViewNotify

PURPOSE: 
  Handles the WM_NOTIFY messages.

***********************************************************************/
LRESULT ListViewNotify (HWND hwnd, LPARAM lParam)
{
  LPNMHDR lpnmh  = (LPNMHDR) lParam;  // Contains information about the 
                                      // notification message
  HWND hwndListView  = GetDlgItem (hwnd, ID_LISTVIEW);        
                                      // Handle of the list view control        
  switch (lpnmh->code)
  {
    case LVN_GETDISPINFO:
    {
      TCHAR szString[MAX_PATH];
      LV_DISPINFO *lpdi = (LV_DISPINFO *) lParam;
      
      // The message LVN_GETDISPINFO is sent by the list view control to
      // its parent window. It is a request for the parent window to 
      // provide the information needed to display or sort a list view 
      // item.

      if (lpdi->item.iSubItem)
      {
        if (lpdi->item.mask & LVIF_TEXT)
        {
          wsprintf (szString, TEXT("Item %d - Column %d"), 
                    lpdi->item.iItem + 1, lpdi->item.iSubItem);
          lstrcpy (lpdi->item.pszText, szString);
        }
      }
      else
      {
        if (lpdi->item.mask & LVIF_TEXT)
        {
          wsprintf (szString, TEXT("Item %d"), lpdi->item.iItem + 1);
          lstrcpy (lpdi->item.pszText, szString);
        }

        if (lpdi->item.mask & LVIF_IMAGE)
          lpdi->item.iImage = 0;
      }
      return 0;
    }

    case LVN_ODCACHEHINT:
    {
      LPNMLVCACHEHINT lpCacheHint = (LPNMLVCACHEHINT)lParam;
      
      // The message LVN_ODCACHEHINT is sent by the list view control 
      // when the contents of its display area have changed. For 
      // example, a list view control sends this notification when the 
      // user scrolls the control's display. 

      return 0;
    }

    case LVN_ODFINDITEM:
    {
      LPNMLVFINDITEM lpFindItem = (LPNMLVFINDITEM)lParam;
      
      // The message LVN_ODFINDITEM is sent by the list view control
      // when it needs the owner to find a particular callback item. 
      // Return -1 if the item is not found.

      return 0;
    }
  }

  return 0;
}


/***********************************************************************

FUNCTION: 
  SwitchView

PURPOSE: 
  Switches the window style of the list view control.

***********************************************************************/
void SwitchView (HWND hwndListView, DWORD dwView)
{
  DWORD dwStyle = GetWindowLong (hwndListView, GWL_STYLE);

  SetWindowLong (hwndListView, GWL_STYLE, 
                 (dwStyle & ~LVS_TYPEMASK) | dwView);

  ResizeListView (hwndListView, GetParent (hwndListView));
}


/***********************************************************************

FUNCTION: 
  UpdateMenu

PURPOSE: 
  Checks and unchecks the menu items.

***********************************************************************/
void UpdateMenu (HWND hwndListView, HMENU hMenu)
{
  UINT uID;
  DWORD dwStyle;

  // Uncheck all of the menu items.
  CheckMenuItem (hMenu, IDM_LARGE_ICONS, MF_BYCOMMAND | MF_UNCHECKED);
  CheckMenuItem (hMenu, IDM_SMALL_ICONS, MF_BYCOMMAND | MF_UNCHECKED);
  CheckMenuItem (hMenu, IDM_LIST,        MF_BYCOMMAND | MF_UNCHECKED);
  CheckMenuItem (hMenu, IDM_REPORT,      MF_BYCOMMAND | MF_UNCHECKED);

  // Check the appropriate view menu item.
  dwStyle = GetWindowLong (hwndListView, GWL_STYLE);

  switch (dwStyle & LVS_TYPEMASK)
  {
    case LVS_ICON:
      uID = IDM_LARGE_ICONS;
      break;
      
    case LVS_SMALLICON:
      uID = IDM_SMALL_ICONS;
      break;
      
    case LVS_LIST:
      uID = IDM_LIST;
      break;
   
    case LVS_REPORT:
      uID = IDM_REPORT;
      break;
  }

  CheckMenuRadioItem (hMenu, IDM_LARGE_ICONS, IDM_REPORT, uID,  
                      MF_BYCOMMAND | MF_CHECKED);
}
// END OF LISTVIEW.C