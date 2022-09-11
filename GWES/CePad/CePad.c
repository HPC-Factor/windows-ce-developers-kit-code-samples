/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CePad.c

ABSTRACT: 
  This application shows how to register a window class, and how to
  create such elements as a window, a menu, dialog boxes, and an edit 
  control. It also shows how to open and save a file, and how to search
  for and replace a text string in an edit control.
 
***********************************************************************/

#include "CePad.h"
#include <commctrl.h>

/***********************************************************************

FUNCTION: 
  AboutDialogProc

PURPOSE: 
  Processes messages sent to the About dialog box window.

***********************************************************************/
BOOL CALLBACK AboutDialogProc (
                        HWND hwndDlg,     // Handle to the dialog box
                        UINT uMsg,        // Message
                        WPARAM wParam,    // First message parameter
                        LPARAM lParam)    // Second message parameter
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
  WndProc

PURPOSE: 
  Processes messages sent to the main window.
                                                    
***********************************************************************/
LRESULT CALLBACK WndProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                          LPARAM lParam)
{
  DWORD dwStyle;        // Window style of the edit control
  int iCBHeight,        // Command bar height
      iSelBeg,          // Starting and ending character positions of 
      iSelEnd,          // the current selection in the edit control
      iSearchOffset;      
  UINT uEnable;         // Indicates whether a menu item is enabled, 
                        // disabled, or grayed
  TCHAR szFileName[_MAX_PATH] = TEXT(" (untitled)");
                        // Name of the current file
  switch (uMsg)
  {
    case WM_CREATE:
    {  
      // Specify the edit control window style.
      dwStyle = WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | 
                WS_BORDER | ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | 
                ES_AUTOHSCROLL | ES_AUTOVSCROLL;  
      
      // Create the edit control window.
      if (!(g_hwndEdit = CreateWindow (
                TEXT("edit"),   // Class name
                NULL,           // Window text
                dwStyle,        // Window style
                0,              // X-coordinate of the upper-left corner
                0,              // Y-coordinate of the upper-left corner
                CW_USEDEFAULT,  // Width of the edit control window
                CW_USEDEFAULT,  // Height of the edit control window
                hwnd,           // Window handle of parent window
                (HMENU) EDITID, // Control identifier
                g_hInst,        // Instance handle
                NULL)))         // Specify NULL for this parameter when 
                                // creating a control
      {
        return 0;
      }

      // Set the maximum file size.
      SendMessage (g_hwndEdit, EM_LIMITTEXT, MAXFILESIZE, 0L);

      CPFileInitialize (hwnd); 
      SendMessage (hwnd, WM_COMMAND, IDM_NEW, 0L);

      // Create the command bar and insert the menu.
      g_hwndCB = CommandBar_Create (g_hInst, hwnd, 1);
      CommandBar_InsertMenubar (g_hwndCB, g_hInst, IDR_CEPADMENU, 0);
      CommandBar_AddAdornments (g_hwndCB, 0, 0);

      return 0;
    }

    case WM_SETFOCUS:
      SetFocus (g_hwndEdit);
      return 0;

    case WM_SIZE: 
      // Get the command bar height.
      iCBHeight = CommandBar_Height (g_hwndCB); 

      // Resize the edit control window.
      MoveWindow (g_hwndEdit, 0, iCBHeight, LOWORD (lParam), 
                  HIWORD (lParam) - iCBHeight, TRUE);
      return 0;
    
    case WM_INITMENUPOPUP:
      switch (lParam)
      {
        case EDITID:          // Edit menu
          // Enable Undo if the edit-control operation can be undone.
          uEnable = SendMessage (g_hwndEdit, EM_CANUNDO, 0, 0L) ? \
                                 MF_ENABLED : MF_GRAYED;
          EnableMenuItem ((HMENU) wParam, IDM_UNDO, uEnable);

          // Enable Paste if text is in the Clipboard.
          uEnable = IsClipboardFormatAvailable (CF_UNICODETEXT) ? \
                                                MF_ENABLED : MF_GRAYED;
          EnableMenuItem ((HMENU) wParam, IDM_PASTE, uEnable);
            
          // Enable Cut, Copy, and Clear if text is selected.
          SendMessage (g_hwndEdit, EM_GETSEL, (WPARAM) &iSelBeg, 
                       (LPARAM) &iSelEnd);
          uEnable = iSelBeg != iSelEnd ? MF_ENABLED : MF_GRAYED;
          EnableMenuItem ((HMENU) wParam, IDM_CUT,   uEnable);
          EnableMenuItem ((HMENU) wParam, IDM_COPY,  uEnable);
          EnableMenuItem ((HMENU) wParam, IDM_CLEAR, uEnable);
          break;
      }
      return 0;

    case WM_COMMAND:
      // Messages from the edit control.
      if (lParam && LOWORD (wParam) == EDITID)
      {
        switch (HIWORD (wParam))
        {
          case EN_UPDATE:
            g_bNeedSave = TRUE;
            return 0;

          case EN_ERRSPACE:
          case EN_MAXTEXT:
            MessageBox (hwnd, TEXT("Edit control out of space."), 
                        g_szClassName, MB_OK | MB_ICONSTOP);
            return 0;
        }
        break;
      }

      // Messages from the application's main menu
      switch (LOWORD (wParam))
      {
        case IDM_NEW:
          if (g_bNeedSave && CPAskSaveFile (hwnd) == IDCANCEL)
            return 0; 

          g_bNeedSave = FALSE;
          SetWindowText (g_hwndEdit, TEXT("\0"));
          wcscpy (g_szFileName, TEXT(" (untitled)"));
          wsprintf (g_szCaption, TEXT("%s - %s"), g_szTitle, 
                    g_szFileName);
          SetWindowText (hwnd, g_szCaption);
          return 0;

        case IDM_OPEN:
          if (g_bNeedSave && CPAskSaveFile (hwnd) == IDCANCEL)
            return 0;

          if (CPFileOpenDlg (hwnd, szFileName))
          {
            if (CPFileRead (g_hwndEdit, szFileName))
            {
              g_bNeedSave = FALSE;
              wcscpy (g_szFileName, szFileName);
              wsprintf (g_szCaption, TEXT("%s - %s"), g_szTitle, 
                        g_szFileName);
              SetWindowText (hwnd, g_szCaption);
              return 0;
            }

            MessageBox (hwnd, TEXT("File cannot be opened!"), 
                        g_szClassName, MB_OK | MB_ICONSTOP);
          }
          return 0;
          
        case IDM_SAVE:
          if (wcscmp (g_szFileName, TEXT(" (untitled)")) != 0)
          {
            if (CPFileWrite (g_hwndEdit, g_szFileName))
            {
              g_bNeedSave = FALSE;
              return 1;
            }

            MessageBox (hwnd, TEXT("File cannot be saved!"), 
                        g_szClassName, MB_OK | MB_ICONSTOP);
            return 0;
          }

        case IDM_SAVEAS:
          if (CPFileSaveDlg (hwnd, szFileName))
          {
            if (CPFileWrite (g_hwndEdit, szFileName))
            {
              g_bNeedSave = FALSE;
              wcscpy (g_szFileName, szFileName);
              wsprintf (g_szCaption, TEXT("%s - %s"), 
                        g_szTitle, g_szFileName);
              SetWindowText (hwnd, g_szCaption);
              return 1;
            }

            MessageBox (hwnd, TEXT("File cannot be saved!"), 
                        g_szClassName,  MB_OK | MB_ICONSTOP);
          }

          return 0;
        
        case IDM_UNDO:
           SendMessage (g_hwndEdit, WM_UNDO, 0, 0);
           return 0;

        case IDM_CUT:
           SendMessage (g_hwndEdit, WM_CUT, 0, 0);
           return 0;

        case IDM_COPY:
           SendMessage (g_hwndEdit, WM_COPY, 0, 0);
           return 0;

        case IDM_PASTE:
           SendMessage (g_hwndEdit, WM_PASTE, 0, 0);
           return 0;

        case IDM_CLEAR:
           SendMessage (g_hwndEdit, WM_CLEAR, 0, 0);
           return 0;

        case IDM_SELECTALL:
          SendMessage (g_hwndEdit, EM_SETSEL, 0, -1);
          return 0;
        
        case IDM_FINDNEXT:
          if (wcslen (g_szFindWhat) > 0)
          {
            SendMessage (g_hwndEdit, EM_GETSEL, (WPARAM) NULL, 
                         (LPARAM) &iSearchOffset);
            CPFindText (g_hwndEdit, iSearchOffset, g_szFindWhat);
            return 0;
          }

        case IDM_FIND:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_FIND), g_hwndMain, 
                     (DLGPROC) FindDialogProc);
          return 0;
        
        case IDM_REPLACE:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_REPLACE), g_hwndMain, 
                     (DLGPROC) ReplaceDialogProc);
          return 0;
        
        case IDM_ABOUT:
          DialogBox (g_hInst, MAKEINTRESOURCE (IDD_ABOUT), g_hwndMain, 
                     AboutDialogProc);
          return 0;
        
        case IDM_EXIT:
          SendMessage (hwnd, WM_CLOSE, 0, 0);
          return 0;
      }

      SetFocus (g_hwndEdit); 
      break;

    case WM_CLOSE:
      if (!g_bNeedSave || CPAskSaveFile (hwnd) != IDCANCEL)
      {
        CommandBar_Destroy (g_hwndCB);
        DestroyWindow (hwnd);
      }
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
  wndclass.hIcon = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_CEPADICON));
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
              HINSTANCE hInstance,      // Handle to current instance
              HINSTANCE hPrevInstance,  // Handle to previous instance
              LPWSTR lpCmdLine,         // Pointer to command line
              int nCmdShow)             // Shows the state of the window
{
  MSG msg;
  HACCEL hAccel;
   
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
  
  hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE (IDR_CEPADACC));
  
  while (GetMessage (&msg, NULL, 0, 0))
  {
    if (!TranslateAccelerator (g_hwndMain, hAccel, &msg))
    {
      TranslateMessage (&msg);
      DispatchMessage (&msg);
    }
  }
  
  return msg.wParam;
}
// END OF CEPAD.C
  
