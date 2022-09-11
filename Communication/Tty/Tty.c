/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Tty.c

ABSTRACT: 
  This Windows CE application shows how to open, configure, and close a
  serial communications port and perform read/write operations for a TTY
  terminal emulation application.

***********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include "tty.h"
#include "resource.h"

HWND hMainWnd,                          // Main window handle
     hTermWnd,                          // Terminal window handle
     hCmdBarWnd,                        // The command bar window handle
     hDTRWnd,                           // DTR check box window handle
     hRTSWnd,                           // RTS check box window handle
     hDSRWnd,                           // DSR check box window handle
     hCTSWnd,                           // CTS check box window handle
     hDCDWnd,                           // DCD check box window handle
     hRINGWnd;                          // RING check box window handle

HINSTANCE hInst;                        // hInstance of the application
TCHAR szTitle[] = TEXT("Tty");          // Main window name
TCHAR szAppName[] = TEXT("Tty Sample Application");
                                        // Main window class name
LPTSTR lpszDevName;                     // Communication port name
HFONT hFont = NULL;                     // Handle to the font to use
HANDLE hReadThread = NULL;              // Handle to the read thread
HANDLE hPort = INVALID_HANDLE_VALUE;    // Handle to the serial port

TERM_SCREEN Screen;                     

DWORD dwBaudRates[] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 38400,
                       57600, 115200};  // List of baud

#define BUTTONBAR_HEIGHT  16            // Height of the check box bar
#define BUTTON_HEIGHT     14            // Height of the check boxes
#define NUM_BAUD_RATES    (sizeof (dwBaudRates) / sizeof (DWORD))
                                        // Number of baud in the baud
                                        // list
/***********************************************************************

FUNCTION: 
  SetLightIndicators

PURPOSE: 
  Sets the check state of the check boxes. 

***********************************************************************/
VOID SetLightIndicators (DWORD dwCommModemStatus)
{
  SendMessage (hDSRWnd, BM_SETCHECK, 
               dwCommModemStatus & MS_DSR_ON, 0);
  SendMessage (hCTSWnd, BM_SETCHECK, 
               dwCommModemStatus & MS_CTS_ON, 0);
  SendMessage (hDCDWnd, BM_SETCHECK, 
               dwCommModemStatus & MS_RLSD_ON, 0);
  SendMessage (hRINGWnd, BM_SETCHECK, 
               dwCommModemStatus & MS_RING_ON, 0);
}


/***********************************************************************

FUNCTION: 
  CommDlgProc

PURPOSE: 
  Processes messages sent to the Communications dialog box window. 

***********************************************************************/
LRESULT CALLBACK CommDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
  LRESULT lResult;
  HWND hCtrlWnd;
  DCB PortDCB;
  int index;
  TCHAR szTmpString[80];
  
  switch (uMsg) 
  {
    case WM_INITDIALOG:
      wsprintf (szTmpString, TEXT("Communications settings for '%s'"),
                lpszDevName);
      SetWindowText (GetDlgItem (hDlg, IDC_PORTLABEL), szTmpString);

      PortDCB.DCBlength = sizeof (DCB);
      
      // Get the default port setting information.
      GetCommState (hPort, &PortDCB);

      // Add strings to the baud combo box and select an item.
      hCtrlWnd = GetDlgItem (hDlg, IDC_BAUDRATE);

      for (index = 0; index < NUM_BAUD_RATES; ++index) 
      {
        wsprintf (szTmpString, TEXT("%d"), dwBaudRates[index]);
        SendMessage (hCtrlWnd, CB_INSERTSTRING, index, (LPARAM)szTmpString);
      }

      wsprintf (szTmpString, TEXT("%d"), PortDCB.BaudRate);
      lResult = SendMessage (hCtrlWnd, CB_FINDSTRINGEXACT, 0, 
                             (LPARAM)szTmpString);
      SendMessage (hCtrlWnd, CB_SETCURSEL, lResult, 0);

      // Add strings to the data bits combo box and select an item.
      hCtrlWnd = GetDlgItem (hDlg, IDC_DATABITS);

      for (index = 4; index <= 8; index++) 
      {
        wsprintf (szTmpString, TEXT("%d"), index);
        SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)szTmpString);
      }

      wsprintf (szTmpString, TEXT("%d"), PortDCB.ByteSize);
      lResult = SendMessage (hCtrlWnd, CB_FINDSTRINGEXACT, 0, 
                             (LPARAM)szTmpString);
      SendMessage (hCtrlWnd, CB_SETCURSEL, lResult, 0);
    
      // Add strings to the stop bits combo box and select an item.
      hCtrlWnd = GetDlgItem (hDlg, IDC_STOPBITS);
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("1"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("1.5"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("2"));
      SendMessage (hCtrlWnd, CB_SETCURSEL, (WPARAM)PortDCB.StopBits, 0);
    
      // Add strings to the parity combo box and select an item.
      hCtrlWnd = GetDlgItem (hDlg, IDC_PARITY);
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("None"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Odd"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Even"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Mark"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("Space"));
      SendMessage (hCtrlWnd, CB_SETCURSEL, (WPARAM)PortDCB.Parity, 0);
    
      // Add strings to the flow control combo box and select an item.
      hCtrlWnd = GetDlgItem (hDlg, IDC_FLOWCTRL);
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("None"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("RTS/CTS"));
      SendMessage (hCtrlWnd, CB_ADDSTRING, 0, (LPARAM)TEXT("XON/XOFF"));

      if (PortDCB.fOutX) 
        index = 2;
      else if (PortDCB.fOutxCtsFlow) 
        index = 1;
      else
        index = 0;

      SendMessage (hCtrlWnd, CB_SETCURSEL, (WPARAM)index, 0);

      // Set the state of DSR Sensitivity check box.
      SendMessage (GetDlgItem (hDlg, IDC_DSR_SENS), 
                   BM_SETCHECK,
                   (WPARAM)(PortDCB.fDsrSensitivity ? 1 : 0),
                   (LPARAM)0);

      return TRUE;

    case WM_COMMAND:

      switch (LOWORD(wParam)) 
      {
        case IDCANCEL:
          EndDialog (hDlg, FALSE);
          return TRUE;

        case IDOK:
          // Save the settings for the serial communications device.

          // Save the baud.
          GetWindowText (GetDlgItem (hDlg, IDC_BAUDRATE),
                         szTmpString, 
                         sizeof (szTmpString) / sizeof (TCHAR));
          PortDCB.BaudRate = _ttol (szTmpString);

          // Save the data bits.
          GetWindowText (GetDlgItem (hDlg, IDC_DATABITS), 
                         szTmpString,
                         sizeof (szTmpString) / sizeof (TCHAR));
          PortDCB.ByteSize = (BYTE)_ttol (szTmpString);
      
          // Save the stop bits.
          PortDCB.StopBits = (BYTE)SendMessage (
                                        GetDlgItem (hDlg, IDC_STOPBITS),
                                        CB_GETCURSEL,
                                        (WPARAM)0, 
                                        (LPARAM)0);
          // Save the parity.
          PortDCB.Parity = (BYTE)SendMessage (
                                        GetDlgItem (hDlg, IDC_PARITY),
                                        CB_GETCURSEL,
                                        (WPARAM)0, 
                                        (LPARAM)0);

          // Set to no flow control.
          lResult = SendMessage (GetDlgItem (hDlg, IDC_FLOWCTRL),
                                 CB_GETCURSEL,
                                 (WPARAM)0, 
                                 (LPARAM)0);
          PortDCB.fOutX = FALSE;
          PortDCB.fInX = FALSE;
          PortDCB.fOutxCtsFlow = FALSE;
          PortDCB.fRtsControl = RTS_CONTROL_ENABLE;
          
          if (lResult == 1) 
          {
            PortDCB.fOutxCtsFlow = TRUE;
            PortDCB.fRtsControl = RTS_CONTROL_HANDSHAKE;
          }

          if (lResult == 2) 
          {
            PortDCB.fOutX = TRUE;
            PortDCB.fInX = TRUE;
          } 

          // Save the DSR Sensitivity.
          lResult = SendMessage (GetDlgItem (hDlg, IDC_DSR_SENS), 
                                 BM_GETCHECK,
                                 (WPARAM)0, 
                                 (LPARAM)0);
          PortDCB.fDsrSensitivity = (lResult) ? 1 : 0;

          // Configure the port according to the DCB structure.
          SetCommState (hPort, &PortDCB);

          EndDialog (hDlg, TRUE);
          return TRUE;
      }
      break;
  }

  return FALSE;
}


/***********************************************************************

FUNCTION: 
  TermWndProc

PURPOSE: 
  Processes messages sent to the terminal window.

***********************************************************************/
LRESULT CALLBACK TermWndProc (HWND hWnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
  HDC hDC;
  PAINTSTRUCT ps;

  switch (uMsg)
  {
    case WM_SETFOCUS:
      Screen.bHaveFocus = TRUE;
      CreateCaret (hWnd, NULL, Screen.CharWidth, Screen.CharHeight);

      if (ON_SCREEN (Screen.CursorRow, Screen.CursorCol))
      {
        SetCaretPos (COLX(Screen.CursorCol), ROWY(Screen.CursorRow));
        ShowCaret (hWnd);    
      }
      break;

    case WM_KILLFOCUS:
      Screen.bHaveFocus = FALSE;
      HideCaret (hWnd);
      DestroyCaret ();
      break;

    case WM_SIZE:
      // Resize everything.
      InitScreenSettings (hWnd);
      break;

    case WM_PAINT:
      HideCaret (hTermWnd);
      hDC = BeginPaint (hWnd, &ps);
      PaintScreen (hWnd, hDC, &(ps.rcPaint));
      EndPaint (hWnd, &ps);
      ShowCaret (hTermWnd);
      break;

    case WM_CHAR:
      if (hPort != INVALID_HANDLE_VALUE)
        // Writing to the port.
        PortWrite ((BYTE)(wParam & 0xFF));
      else 
        // Display on the screen.
        ProcessChar ((BYTE)(wParam & 0xFF));
      break;

    case WM_VSCROLL:
      switch (LOWORD(wParam))
      {
        case SB_LINEUP:
          ScrollDown (-1);
          break;

        case SB_LINEDOWN:
          ScrollDown (1);
          break;

        case SB_PAGEUP:
          ScrollDown (-(Screen.ScreenRows - 1));
          break;

        case SB_PAGEDOWN:
          ScrollDown (Screen.ScreenRows - 1);
          break;

        case SB_TOP:
          ScrollDown (-Screen.NumRows);
          break;

        case SB_BOTTOM:
          ScrollDown (Screen.NumRows);
          break;

        case SB_THUMBPOSITION:
          ScrollDown (HIWORD(wParam) - Screen.TopRow);
          break;

        case SB_THUMBTRACK:
          break;

        case SB_ENDSCROLL:
          break;
      }
      break;

    case WM_HSCROLL:
      switch (LOWORD(wParam))
      {
        case SB_LINELEFT:
          ScrollRight (-1);
          break;

        case SB_LINERIGHT:
          ScrollRight (1);
          break;

        case SB_PAGELEFT:
          ScrollRight (-(Screen.ScreenCols - 1));
          break;

        case SB_PAGERIGHT:
          ScrollRight (Screen.ScreenCols - 1);
          break;

        case SB_LEFT:
          ScrollRight (-Screen.NumCols);
          break;

        case SB_RIGHT:
          ScrollRight (Screen.NumCols);
          break;

        case SB_THUMBPOSITION:
          ScrollRight (HIWORD(wParam) - Screen.LeftCol);
          break;

        case SB_THUMBTRACK:
          break;

        case SB_ENDSCROLL:
          break;
      }
      break;

    case WM_DESTROY:
      PostQuitMessage (0);
      break;
  }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}


/***********************************************************************

FUNCTION: 
  MainWndProc

PURPOSE: 
  Processes messages sent to the main window.
      
***********************************************************************/
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT uMsg, WPARAM wParam,
                              LPARAM lParam)
{
  HDC hDC;
  RECT rect;
  HBRUSH hBrush;
  PAINTSTRUCT ps;

  switch (uMsg)
  {
    case WM_CREATE:
      // Create command bar and insert the menu.
      hCmdBarWnd = CommandBar_Create (hInst, hWnd, 1);
      CommandBar_InsertMenubar (hCmdBarWnd, hInst, IDR_MAIN_MENU, 0);
      CommandBar_AddAdornments (hCmdBarWnd, 0, 0);
      return 0;

    case WM_PAINT:
      hDC = BeginPaint (hWnd, &ps);

      // Draw the gray background for the button bar.
      hBrush = (HBRUSH)GetStockObject (LTGRAY_BRUSH);
      GetClientRect (hWnd, &rect);
      rect.bottom = BUTTONBAR_HEIGHT;
      FillRect (hDC, &rect, hBrush);
      DeleteObject (hBrush);

      // Draw the black background for rest of the area.
      hBrush = (HBRUSH)GetStockObject (BLACK_BRUSH);
      rect.top = BUTTONBAR_HEIGHT - 1;
      FillRect (hDC, &rect, hBrush);
      DeleteObject (hBrush);

      EndPaint (hWnd, &ps);
      return 0;

    case WM_SETFOCUS:
      // Give the focus to the terminal window.
      SetFocus (hTermWnd);
      return 0;

    case WM_SIZE:
      GetClientRect (hWnd, &rect);
      rect.top = CommandBar_Height (hCmdBarWnd);
      rect.top += BUTTONBAR_HEIGHT;
      SetWindowPos (hTermWnd, HWND_TOP, rect.left, rect.top,
                    rect.right - rect.left, rect.bottom - rect.top, 0);
      ShowWindow (hTermWnd, SW_SHOWNORMAL);
      return 0;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_FILE_EXIT:
          DestroyWindow (hWnd);
          break;

        case ID_EDIT_CLEARBUFFER:
          ClearScreen ();
          break;

        case ID_SETTINGS:
          DialogBox (hInst, MAKEINTRESOURCE(IDD_COMMUNICATIONS),
                     hWnd, CommDlgProc);
          break;

        case ID_HELP_ABOUTTTY:
          MessageBox (hWnd, 
                      TEXT("Window CE TTY Sample Application\r\n")
                      TEXT("              (c) Microsoft 1999"),
                      TEXT("About TTY"), 
                      MB_OK);
          break;

        case ID_DTR_BTN:
          if (SendMessage (hDTRWnd, BM_GETCHECK, 0, 0))
            EscapeCommFunction (hPort, SETDTR);
          else
            EscapeCommFunction (hPort, CLRDTR);
          
          SetFocus (hTermWnd);
          break;

        case ID_RTS_BTN:
          if (SendMessage (hRTSWnd, BM_GETCHECK, 0, 0))
            EscapeCommFunction (hPort, SETRTS);
          else 
            EscapeCommFunction (hPort, CLRRTS);

          SetFocus (hTermWnd);
          break;

        case ID_DSR_BTN:
        case ID_CTS_BTN:
        case ID_DCD_BTN:
        case ID_RING_BTN:
          SetFocus (hTermWnd);
          break;

        default:
          MessageBox (hWnd, 
                      TEXT("Unhandled WM_COMMAND message"),
                      TEXT("ERROR"), 
                      MB_OK);
          break;
      }
      return 0;

    case WM_DESTROY:
    {
      // Clear all events to be monitored for the port. 
      SetCommMask (hPort, 0);

      // Close the serial port.
      PortClose (hPort);

      PostQuitMessage (0);
      return 0;
    }
  }

  return (DefWindowProc (hWnd, uMsg, wParam, lParam));
}

/***********************************************************************

FUNCTION: 
  InitApplication

PURPOSE: 
  Initialize and register window class.

***********************************************************************/
BOOL InitApplication (HINSTANCE hInstance)
{
  WNDCLASS  wc;

  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC)MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = (HICON)NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = 0;
  wc.lpszClassName = szAppName;

  if (!RegisterClass (&wc))
    return FALSE;
  
  wc.style = CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc = (WNDPROC)TermWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = (HICON)NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName = 0;
  wc.lpszClassName = TEXT("TermClass");

  return RegisterClass (&wc);
}

/***********************************************************************

FUNCTION: 
  InitInstance

PURPOSE: 
  Create and display the main window.

***********************************************************************/
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  RECT rect;
  DWORD dwStyle,
        dwError;

  hMainWnd = CreateWindowEx (0, 
                             szAppName,
                             szTitle,
                             WS_CLIPCHILDREN,
                             0,0,
                             CW_USEDEFAULT, 
                             CW_USEDEFAULT,
                             NULL, 
                             NULL,
                             hInstance, 
                             NULL);
  if (!hMainWnd)
  {
    dwError = GetLastError ();
    return FALSE;
  }

  ShowWindow (hMainWnd, nCmdShow);
  UpdateWindow (hMainWnd);

  GetClientRect (hMainWnd, &rect);
  rect.top = CommandBar_Height (hCmdBarWnd) + 1;

  // Window style
  dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX;

  hDTRWnd = CreateWindow (TEXT("button"), TEXT("DTR"), dwStyle, 
                          rect.left + 5, rect.top, 50, BUTTON_HEIGHT,
                          hMainWnd, (HMENU)ID_DTR_BTN, hInstance, NULL);
              
  hRTSWnd = CreateWindow (TEXT("button"), TEXT("RTS"), dwStyle,
                          rect.left + 65, rect.top, 50, BUTTON_HEIGHT, 
                          hMainWnd, (HMENU)ID_RTS_BTN, hInstance, NULL);
  
  // Create the following check box for monitoring a set of events.
  // Because these checkboxes are used for monitoring events only, 
  // make all of them disabled upon creation.

  // Window style
  dwStyle = WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_DISABLED;

  hDSRWnd = CreateWindow (TEXT("button"), TEXT("DSR"), dwStyle, 
                          rect.left + 125, rect.top, 50, BUTTON_HEIGHT, 
                          hMainWnd, (HMENU)ID_DSR_BTN, hInstance, NULL);

  hCTSWnd = CreateWindow (TEXT("button"), TEXT("CTS"), dwStyle, 
                          rect.left + 185, rect.top, 50, BUTTON_HEIGHT, 
                          hMainWnd, (HMENU)ID_CTS_BTN, hInstance, NULL);

  hDCDWnd = CreateWindow (TEXT("button"), TEXT("DCD"), dwStyle, 
                          rect.left + 245, rect.top, 50, BUTTON_HEIGHT, 
                          hMainWnd, (HMENU)ID_DCD_BTN, hInstance, NULL);

  hRINGWnd = CreateWindow (TEXT("button"), TEXT("RING"), dwStyle, 
                          rect.left + 305, rect.top, 60, BUTTON_HEIGHT, 
                          hMainWnd, (HMENU)ID_RING_BTN, hInstance,NULL);
  
  rect.top += BUTTONBAR_HEIGHT - 1;

  // Terminal window style
  dwStyle = WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | WS_BORDER;

  // Create the terminal window.
  hTermWnd = CreateWindowEx (0, 
                             TEXT("TermClass"),
                             TEXT("H/PC TTY"),
                             dwStyle,
                             rect.left, 
                             rect.top, 
                             rect.right - rect.left,
                             rect.bottom - rect.top,
                             hMainWnd, 
                             NULL,
                             hInstance, 
                             NULL);
  if (!hTermWnd)
  {
    dwError = GetLastError ();
    return FALSE;
  }

  SetFocus (hTermWnd);

  return TRUE;
}



/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  The WinMain function of the application. It is called by the system as
  the initial entry point for this WindowsCE-based application.

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
  DWORD dwCommModemStatus;

  hInst = hInstance;

  if (!hPrevInstance)
  {
    if (!InitApplication (hInstance))
      return 0;
  }

  if (!InitInstance (hInstance, nCmdShow))
  {
    if (hTermWnd)
      DestroyWindow (hTermWnd);
    
    goto ExitMain;
  }

  // Initialize the screen.
  if (!InitScreenSettings (hTermWnd))
  {
    MessageBox (hMainWnd, 
                TEXT("Unable to allocate screen buffer"),
                TEXT("Fatal Error"), 
                MB_OK);

    DestroyWindow (hTermWnd);
    goto ExitMain;
  }

  // Clear the screen.
  ClearScreen ();

  // Assign the port name. If the port name is NULL, assign the default
  // value as "COM1:".
  if (*lpCmdLine == TEXT('\0')) 
    lpszDevName = TEXT("COM1:");
  else
    lpszDevName = lpCmdLine;

  // Initialize the port.
  if (!PortInitialize (lpszDevName))
  {
    DestroyWindow (hTermWnd);
    goto ExitMain;
  }
  
  // Set the state of the DTR and RTS check boxes.
  SendMessage (hDTRWnd, BM_SETCHECK, TRUE, 0);
  SendMessage (hRTSWnd, BM_SETCHECK, TRUE, 0);

  // Initialize the indicator lights.
  GetCommModemStatus (hPort, &dwCommModemStatus);
  SetLightIndicators (dwCommModemStatus);

  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }

ExitMain:

  if (hFont)
    DeleteObject ((HGDIOBJ)hFont);
  
  return 1;
}


