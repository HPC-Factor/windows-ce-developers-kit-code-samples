/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  FileView.c

ABSTRACT: 
  This Windows CE application shows how to register tree view and 
  header control classes, and create a tree view control.

***********************************************************************/

#include "FileView.h"

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
  WndProc:

PURPOSE: 
  Processes messages sent to the main window.
                                          
***********************************************************************/
LRESULT CALLBACK WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                          LPARAM lParam)
{
  int iCBHeight;                    // Command bar height
  RECT rcClient;                    // Client area rectangle
  TCHAR szDir[MAX_PATH];          
  HTREEITEM hti,                
            htiNext;            
  
  switch (uMsg)
  {
    case WM_CREATE:
    {
      DWORD dwStyle;                // Style flags of the tree view.
      INITCOMMONCONTROLSEX  iccex;  // INITCOMMONCONTROLSEX structure

      // Initialize the INITCOMMONCONTROLSEX structure.   
      iccex.dwSize = sizeof (INITCOMMONCONTROLSEX);
      iccex.dwICC = ICC_TREEVIEW_CLASSES;

      // Register tree view control classes from the common control 
      // dynamic-link library (DLL). 
      InitCommonControlsEx (&iccex);

      // Get the client area rectangle.
      GetClientRect (hwnd, &rcClient);

      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDR_MENU, 0);
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      // Get the command bar height.
      iCBHeight = CommandBar_Height (g_hwndCB);

      // Assign the window styles for the tree view.
      dwStyle = WS_VISIBLE | WS_CHILD | TVS_HASLINES | TVS_LINESATROOT | 
                TVS_HASBUTTONS;

      // Create the tree view control.
      g_hwndTreeView = CreateWindowEx (
          0, 
          WC_TREEVIEW,          // Class name
          TEXT("TreeView"),     // Window name
          dwStyle,              // Window style
          0,                    // X-coordinate of the upper-left corner
          iCBHeight + 1,        // Y-coordinate of the upper-left corner
          rcClient.right,       // Width of the edit control window
          rcClient.bottom - (iCBHeight + 1), 
                                // Height of the edit control window  
          hwnd,                 // Parent window handle
          (HMENU) IDC_TREEVIEW, // Tree view control identifier
          g_hInst,              // Instance handle
          NULL);                // Specify NULL for this parameter when 
                                // creating a control

      // Make sure the tree view was created.
      if (!g_hwndTreeView)
        return 0;

      // Initialize the image list and add root items to the control.
      if (!InitTreeViewImageLists (g_hwndTreeView))
      {
        DestroyWindow (g_hwndTreeView);
        return 0;
      }
      else
        AddItemToTree (g_hwndTreeView, TEXT ("\\"), NULL, TRUE);

      return 0;  
    }  
    
    case WM_COMMAND:
      switch (LOWORD (wParam))
      {
        case IDM_ABOUT:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ABOUT), g_hwndMain,
                     AboutDialogProc);
          return 0;  
        
        case IDM_EXIT:
          CommandBar_Destroy (g_hwndCB);
          DestroyWindow (hwnd);
          return 0;  
      }
      break;

    case WM_NOTIFY:
    {
      LPNMHDR  pnmh = (LPNMHDR) lParam;
      
      switch (pnmh->code)
      {
        case TVN_ITEMEXPANDED:
        {
          // Cast the NMHDR into a tree view notify structure.
          LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW) pnmh;

          // Notifies a tree view control's parent window that a parent
          // item's list of child items has expanded or collapsed. This 
          // notification message is sent in the form of a WM_NOTIFY 
          // message. 
          if (pnmtv->action == TVE_COLLAPSE)
          {
            OutputDebugString (TEXT ("Item collapsed.\r\n"));

            // Remove the child items within this directory.
            hti = TreeView_GetChild (pnmh->hwndFrom, 
                                     pnmtv->itemNew.hItem);
            while (hti)
            {
              htiNext = TreeView_GetNextSibling (pnmh->hwndFrom, hti);
              TreeView_DeleteItem (pnmh->hwndFrom, hti);
              hti = htiNext;
            }
          }
          return 0;
        }
          
        case TVN_ITEMEXPANDING:
        {
          // Cast the NMHDR into a tree view notify structure.
          LPNM_TREEVIEW pnmtv = (LPNM_TREEVIEW) pnmh;

          // Notifies a tree view control's parent window that a parent 
          // item's list of child items is about to expand or collapse. 
          SetCursor (LoadCursor (NULL, IDC_WAIT));

          if (pnmtv->action == TVE_COLLAPSE)
          {
            OutputDebugString (TEXT ("Item collapsing.\r\n"));

            // Retrieve the image from the current item.
            pnmtv->itemNew.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            TreeView_GetItem (pnmh->hwndFrom, & (pnmtv->itemNew));

            // Set the item's image to the closed folder.
            pnmtv->itemNew.iImage = IMAGE_CLOSED;
            pnmtv->itemNew.iSelectedImage = IMAGE_CLOSED;
                 
            TreeView_SetItem (pnmh->hwndFrom, & (pnmtv->itemNew));
          }
          else
          {
            OutputDebugString (TEXT ("Item expanding.\r\n"));

            // Retrieve the image from the current item.
            pnmtv->itemNew.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
            TreeView_GetItem (pnmh->hwndFrom, & (pnmtv->itemNew));

            // Set the item's image to the closed folder.
            pnmtv->itemNew.iImage = IMAGE_OPEN;
            pnmtv->itemNew.iSelectedImage = IMAGE_OPEN;
            TreeView_SetItem (pnmh->hwndFrom, & (pnmtv->itemNew));

            // Fill in the subdirectory just expanded.
            BuildDirectory (pnmtv->hdr.hwndFrom, pnmtv->itemNew, szDir);

            GetDirectoryContents (pnmtv->hdr.hwndFrom, szDir,
                                  (HTREEITEM)pnmtv->itemNew.hItem);
          }

          SetCursor (NULL);
          return 0;  
        }

        case NM_CLICK:
        {
          UINT state;            
          DWORD dwPos;              // Contains cursor position
          POINT point;              // Contains cursor position
          TV_ITEM tvi;          
          TV_HITTESTINFO tvhti;    
          HTREEITEM htiItemClicked;  

          // Find out where the cursor was.
          dwPos = GetMessagePos ();
          point.x = LOWORD (dwPos);
          point.y = HIWORD (dwPos);

          ScreenToClient (g_hwndTreeView, &point);

          tvhti.pt = point;
          htiItemClicked = TreeView_HitTest (g_hwndTreeView, &tvhti);

          // Get the state information from the item and toggle it.
          if (tvhti.flags & TVHT_ONITEMSTATEICON)
          {
            // Fill out the TV_ITEM structure to get the state image 
            // mask and the item from the control.
            tvi.hItem = htiItemClicked;
            tvi.mask = TVIF_STATE;
            tvi.stateMask = TVIS_STATEIMAGEMASK;
            TreeView_GetItem (g_hwndTreeView, &tvi);

            // Toggle the state.
            state = tvi.state;
            tvi.state = (
                      state & INDEXTOSTATEIMAGEMASK (TVIS_GCNOCHECK)) ?
                      INDEXTOSTATEIMAGEMASK (TVIS_GCCHECK) :
                      INDEXTOSTATEIMAGEMASK (TVIS_GCNOCHECK);

            // Set the new state of the item back into the control.
            TreeView_SetItem (g_hwndTreeView, &tvi);
          }
        }

        default:
          return 0;    
      }
      break;
    }

    case WM_CLOSE:
      CommandBar_Destroy (g_hwndCB);
      DestroyWindow (hwnd);
      return 0;    

    case WM_DESTROY:
      if (g_hImgList)
        ImageList_Destroy (g_hImgList);

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
                  0,              // Horizontal window position
                  0,              // Vertical window position
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
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE (IDI_FILEVIEW));
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
// END OF FILEVIEW.C
  
