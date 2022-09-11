/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeDialer.c

ABSTRACT: 
  This code sample shows how to initialize an application's use
  of TAPI, open a line device, negotiate an API version to use,
  translate an address into another format, place a call on an  
  opened line device, close an opened line device, and shut down 
  an application's use of the line abstraction of the API. 

***********************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <tapi.h>
#include "resource.h"  
#include "CeDialer.h"

HINSTANCE g_hInst = NULL;       // hInstance of the application
HWND g_hwndMain = NULL;         // Handle to the main window
HWND g_hwndDial = NULL;         // Handle to the dialing window

TCHAR g_szTitle[] = TEXT("CeDialer");
                                // CeDialer application window name
TCHAR g_szAppName[] = TEXT("CeDialer Application");
                                // Main window class name
HLINEAPP g_hLineApp = NULL;     // Application's use handle for TAPI
                                // (lineInitialize)
HCALL g_hCall = NULL;           // Handle to the open line device on 
                                // which the call is to be originated 
                                // (lineMakeCall)
LONG g_MakeCallRequestID = 0;   // Request identifier returned by 
                                // lineMakeCall
LONG g_DropCallRequestID = 0;   // Request identifier returned by 
                                // lineDrop
BOOL g_bCurrentLineAvail = TRUE;// Indicates line availability

TCHAR g_szCurrentNum[TAPIMAXDESTADDRESSSIZE + 1]; 
                                // Current phone number
TCHAR g_szLastNum[TAPIMAXDESTADDRESSSIZE + 1];         
                                // Last called phone number
DWORD g_dwNumDevs = 0;          // Number of line devices available
DWORD g_dwCurrentLineID = -1;   // Current line device identifier
DWORD g_dwCurrentLineAddr = -1; // Current line address

LINEINFO g_CurrentLineInfo;     // Contains the current line information
LINEINFO *g_lpLineInfo = NULL;  // Array that contains all the lines' 
                                // information

#define InfoBox(_s)  MessageBox (g_hwndMain, _s, TEXT("Info"), MB_OK)
#define ErrorBox(_s) MessageBox (g_hwndMain, _s, TEXT("Error"), MB_OK)

/***********************************************************************

FUNCTION: 
  MainWndProc

PURPOSE: 
  Processes messages sent to the main window.

***********************************************************************/
BOOL CALLBACK MainWndProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                           LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
    {
      HICON hIcon = LoadIcon (g_hInst, 
                              (LPCTSTR) MAKEINTRESOURCE(IDI_CEDIALER));

      // Limit text in phone number field to TAPIMAXDESTADDRESSSIZE.
      SendDlgItemMessage (hwnd, IDC_PHONENUM, CB_LIMITTEXT, 
                          (WPARAM)TAPIMAXDESTADDRESSSIZE, 0);

      // Initialize the application's use of Tapi.dll. If the function
      // fails, terminate the application.
      if (InitializeTAPI ())
      {
        InfoBox (TEXT("Fails initializing the use of Tapi.dll."));

        if (g_hLineApp)
          lineShutdown (g_hLineApp);

        DestroyWindow (hwnd);
      }

      return TRUE;
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDC_SELECTLINE:

          DialogBox (g_hInst, 
                     MAKEINTRESOURCE(IDD_CONNECTUSING), 
                     hwnd, 
                     (DLGPROC) ConnectUsingProc);
          return TRUE;

        case IDC_REDIAL:
  
          // Display the previously dialed phone number.
          SendDlgItemMessage (hwnd, IDC_PHONENUM, WM_SETTEXT, 0, 
                              (LPARAM)g_szLastNum);

        case IDC_DIAL:
        {
          // Check if there is a number entered.
          if (SendDlgItemMessage (hwnd, 
                                  IDC_PHONENUM, 
                                  WM_GETTEXTLENGTH, 0, 0) > 0)
          {
            // Set the current line if it is not already set.  
            if (g_dwCurrentLineID == -1)
            {
              DialogBox (g_hInst, 
                         MAKEINTRESOURCE(IDD_CONNECTUSING), 
                         hwnd, 
                         (DLGPROC) ConnectUsingProc);
            }

            // Get the number to be dialed.
            GetDlgItemText (hwnd, 
                            IDC_PHONENUM, 
                            (LPTSTR) g_szCurrentNum, 
                            TAPIMAXDESTADDRESSSIZE);

            // Convert the phone number into a canonical format.
            if (!MakeCanonicalNum (g_szCurrentNum))
              return TRUE;
            
            // Redisplay the phone number as a canonical format.
            SendDlgItemMessage (hwnd, IDC_PHONENUM, WM_SETTEXT, 0, 
                                (LPARAM)g_szCurrentNum);

            // Make the call.
            MakePhoneCall (g_szCurrentNum);
            
            // Set the focus on the phone number Edit control.
            SetFocus (GetDlgItem (hwnd, IDC_PHONENUM));

            // Select the phone number once it is dialed.
            SendDlgItemMessage (hwnd, IDC_PHONENUM, EM_SETSEL, 0, -1);
            
            // Save the phone number in the g_szLastNum string.
            lstrcpy (g_szLastNum, g_szCurrentNum);
          }
          return TRUE;
        }

        case IDC_BUTTON1:
        case IDC_BUTTON2:
        case IDC_BUTTON3:
        case IDC_BUTTON4:
        case IDC_BUTTON5:
        case IDC_BUTTON6:
        case IDC_BUTTON7:
        case IDC_BUTTON8:
        case IDC_BUTTON9:
        case IDC_BUTTON0:
        case IDC_BUTTONSTAR:
        case IDC_BUTTONPOUND:
        {
          int iNumOfDigits = 0;
          TCHAR *lpszBuffer;
          static const TCHAR digits[] = {'1', '2', '3', '4', 
                                         '5', '6', '7', '8', 
                                         '9', '0', '*', '#'};

          lpszBuffer = (TCHAR *)LocalAlloc (LPTR, 
                                            TAPIMAXDESTADDRESSSIZE + 1);

          // Clear the current selection.
          SendDlgItemMessage (hwnd, IDC_PHONENUM, WM_CLEAR, 0, 0);

          iNumOfDigits = SendDlgItemMessage (
                                    hwnd, 
                                    IDC_PHONENUM, 
                                    WM_GETTEXT,
                                    (WPARAM)TAPIMAXDESTADDRESSSIZE + 1,
                                    (LPARAM)lpszBuffer);

          if (iNumOfDigits < TAPIMAXDESTADDRESSSIZE)
          {
            lpszBuffer[iNumOfDigits] = 
                                  digits[LOWORD(wParam) - IDC_BUTTON1];

            SendDlgItemMessage (hwnd, IDC_PHONENUM, WM_SETTEXT, 0,
                                (LPARAM)lpszBuffer);
          }

          LocalFree (lpszBuffer);
          return TRUE;
        }

        case IDOK:
        case IDCANCEL:

          if (g_lpLineInfo)
            LocalFree (g_lpLineInfo);

          lineShutdown (g_hLineApp);

          DestroyWindow (hwnd);
          PostQuitMessage (0);
          return TRUE;
      } 
      break; 
    } 

    case WM_DESTROY:

      PostQuitMessage (0);
      return TRUE;

    default:
      return DefWindowProc (hwnd, uMsg, wParam, lParam);
  }

  return FALSE;
}


/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  Called by the system as the initial entry point for this Windows 
  CE-based application.

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, 
                    int nCmdShow)
{
  MSG msg;
  WNDCLASS wc;
  HANDLE hwndCeDialer;

  g_hInst = hInstance;        
  
  // If CeDialer.exe is running, then give the focus to the window.
  hwndCeDialer = FindWindow (g_szAppName, g_szTitle); 
  
  if (hwndCeDialer) 
  {
    SetForegroundWindow (hwndCeDialer);    
    return 0;
  }

  // Register the window class.
  wc.style = CS_DBLCLKS;    
  wc.lpfnWndProc = DefDlgProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = DLGWINDOWEXTRA;
  wc.hInstance = g_hInst;
  wc.hIcon = LoadIcon (g_hInst, MAKEINTRESOURCE(IDI_CEDIALER));
  wc.hCursor = NULL;      
  wc.hbrBackground = (HBRUSH) GetStockObject (COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = g_szAppName;
  RegisterClass (&wc);

  // Create the dialog box. If it fails, return 0.
  if (!(g_hwndMain = CreateDialog (g_hInst, 
                                   MAKEINTRESOURCE(IDD_CEDIALER),
                                   NULL, 
                                   (DLGPROC) MainWndProc)))
  {
    return 0;
  }

  ShowWindow (g_hwndMain, SW_SHOW);
  UpdateWindow (g_hwndMain);

  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);  
  }
  
  return msg.wParam;
}


/***********************************************************************

FUNCTION: 
  lineCallbackFunc

PURPOSE: 
  This is a callback function invoked to determine status and events on
  the line device, addresses, or calls. 

***********************************************************************/
VOID CALLBACK lineCallbackFunc (DWORD hDevice, 
                                DWORD dwMsg, 
                                DWORD dwCallbackInstance,
                                DWORD dwParam1, 
                                DWORD dwParam2, 
                                DWORD dwParam3)
{
  LPTSTR lpszStatus;

  switch (dwMsg)
  {
    case LINE_CALLSTATE:

      // If the CALLSTATE does not apply to the call in progress, 
      // then return.
      if (g_hCall != (HCALL) hDevice)
        return;

      // dwParam1 is the specific CALLSTATE change that is occurring.
      switch (dwParam1) 
      {
        case LINECALLSTATE_DIALTONE:
          lpszStatus = TEXT("Dial tone");
          break;

        case LINECALLSTATE_DIALING:
          lpszStatus = TEXT("Dialing");
          break;

        case LINECALLSTATE_PROCEEDING:
          lpszStatus = TEXT("Dialing has completed and the call ")
                       TEXT("is proceeding.");
          break;

        case LINECALLSTATE_RINGBACK:
          lpszStatus = TEXT("Ring back");
          break;

        case LINECALLSTATE_CONNECTED:
          lpszStatus = TEXT("Connected");
          break;
        
        case LINECALLSTATE_BUSY:
          ErrorBox (TEXT("Line busy, shutting down"));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINECALLSTATE_IDLE:
          ErrorBox (TEXT("Line is idle"));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINECALLSTATE_SPECIALINFO:
          ErrorBox (TEXT("Special Information, couldn't dial number"));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINECALLSTATE_DISCONNECTED:
        {
          LPTSTR lpszDisconnected;

          switch (dwParam2)
          {
            case LINEDISCONNECTMODE_NORMAL:
              lpszDisconnected = TEXT("Remote party disconnected");
              break;

            case LINEDISCONNECTMODE_UNKNOWN:
              lpszDisconnected = TEXT("Disconnected: Unknown reason");
              break;

            case LINEDISCONNECTMODE_REJECT:
              lpszDisconnected = TEXT("Remote Party rejected call");
              break;

            case LINEDISCONNECTMODE_PICKUP:
              lpszDisconnected = 
                TEXT("Disconnected: Local phone picked up");
              break;

            case LINEDISCONNECTMODE_FORWARDED:
              lpszDisconnected = TEXT("Disconnected: Forwarded");
              break;

            case LINEDISCONNECTMODE_BUSY:
              lpszDisconnected = TEXT("Disconnected: Busy");
              break;

            case LINEDISCONNECTMODE_NOANSWER:
              lpszDisconnected = TEXT("Disconnected: No Answer");
              break;

            case LINEDISCONNECTMODE_BADADDRESS:
              lpszDisconnected = TEXT("Disconnected: Bad address");
              break;

            case LINEDISCONNECTMODE_UNREACHABLE:
              lpszDisconnected = TEXT("Disconnected: Unreachable");
              break;

            case LINEDISCONNECTMODE_CONGESTION:
              lpszDisconnected = TEXT("Disconnected: Congestion");
              break;

            case LINEDISCONNECTMODE_INCOMPATIBLE:
              lpszDisconnected = TEXT("Disconnected: Incompatible");
              break;

            case LINEDISCONNECTMODE_UNAVAIL:
              lpszDisconnected = TEXT("Disconnected: Unavailable");
              break;

            case LINEDISCONNECTMODE_NODIALTONE:
              lpszDisconnected = TEXT("Disconnected: No dial tone");
              break;

            default:
              lpszDisconnected = TEXT("Disconnected: Unknown reason");
              break;
          }

          ErrorBox (lpszDisconnected);
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;
        } 
      }
      break;   

    case LINE_LINEDEVSTATE:

      switch (dwParam1)
      {
        case LINEDEVSTATE_RINGING:
          lpszStatus = TEXT("Ringing");
          break;

        case LINEDEVSTATE_OUTOFSERVICE:
          ErrorBox (TEXT("The line selected is out of service."));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINEDEVSTATE_DISCONNECTED:
          ErrorBox (TEXT("The line selected is disconnected."));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINEDEVSTATE_MAINTENANCE:
          ErrorBox (TEXT("The line selected is out for maintenance."));
          CurrentLineClose ();
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
          break;

        case LINEDEVSTATE_TRANSLATECHANGE:
          break;

        case LINEDEVSTATE_REMOVED:
          ErrorBox (
            TEXT("The Line device has been removed; no action taken."));
          break;

        case LINEDEVSTATE_REINIT:
        {
          // This usually means that a service provider has changed in 
          // such a way that requires TAPI to REINIT. Note that there 
          // are both soft REINITs and hard REINITs. Soft REINITs do not
          // require a full shutdown but an informational change that
          // historically required a REINIT to force the application to
          // deal with.  TAPI API Version 1.3 applications require a 
          // full REINIT for both hard and soft REINITs.

          switch(dwParam2)
          {
            // This is the hard REINIT. TAPI is waiting for everyone to
            // shut down. Our response is to immediately shut down any 
            // calls, shut down our use of TAPI and notify the user.
            case 0:
              if (MessageBox (
                      g_hwndMain, 
                      TEXT("Tapi line configuration has been changed. ")
                      TEXT("You have to shut down CeDialer to\n")
                      TEXT("re-initialize the use of tapi.dll. Do ")
                      TEXT("you want to shut down CeDialer now?"),
                      TEXT("Warning"), 
                      MB_YESNO) == IDYES)
              {
                lineShutdown (g_hLineApp);
                DestroyWindow (g_hwndMain);
              }

              break;

            case LINE_CREATE:
              lineCallbackFunc (hDevice, dwParam2, dwCallbackInstance, 
                                dwParam3, 0, 0);
              break;

            case LINE_LINEDEVSTATE:
              lineCallbackFunc (hDevice, dwParam2, dwCallbackInstance, 
                                dwParam3, 0, 0);
              break;

            // There might be other reasons to send a soft REINIT.
            // No need to shut down for these reasons.
            default:
              break;
          }
        }

        default:
          break;
      }
      break;

    case LINE_REPLY:

      // Reply from the lineMakeCall function.
      if ((LONG)dwParam1 == g_MakeCallRequestID)
      {
        // If an error occurred on making the call.
        if (dwParam2 != ERR_NONE)
        {
          if (dwParam2 == LINEERR_CALLUNAVAIL)
            ErrorBox (TEXT("The line is not available."));

          CurrentLineClose ();
          
          if (g_hwndDial)
            SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
        }
      }

      // Reply from lineDrop function.
      if ((LONG)dwParam1 == g_DropCallRequestID)
      {
        // Insert code here to deal with this situation.
        // ...
      }

      break;

    case LINE_CREATE:

      // dwParam1 is the device identifier of the new line. 
      if (dwParam1 >= g_dwNumDevs) 
      {
        DWORD dwLineID;
        LINEINFO *lpLineInfo;

        g_dwNumDevs = dwParam1 + 1;
                
        // Allocate a buffer for storing LINEINFO for all the lines.
        if (!(lpLineInfo = (LPLINEINFO) LocalAlloc (
                                      LPTR, 
                                      sizeof (LINEINFO) * g_dwNumDevs)))
        {
          break;
        }

        // Assume we just add a new line, the lines are sequential and
        // the new line is the last one.
        for (dwLineID = 0; dwLineID < dwParam1; ++dwLineID)
        {
          lpLineInfo[dwLineID] = g_lpLineInfo[dwLineID];
        }

        // Get the new line information.
        GetLineInfo (dwParam1, &lpLineInfo[dwParam1]);
           
        LocalFree (g_lpLineInfo);
        g_lpLineInfo = lpLineInfo; 
      }
      break;

    case LINE_CLOSE:
      if (g_CurrentLineInfo.hLine == (HLINE) hDevice)
      {
        CurrentLineClose ();

        if (g_hwndDial)
          SendMessage (g_hwndDial, WM_COMMAND, MAKEWPARAM(IDOK,0), 0);
      }
      break;

    case LINE_ADDRESSSTATE:
    case LINE_CALLINFO:
    case LINE_DEVSPECIFIC:
    case LINE_DEVSPECIFICFEATURE:
    case LINE_GATHERDIGITS:
    case LINE_GENERATE:
    case LINE_MONITORDIGITS:
    case LINE_MONITORMEDIA:
    case LINE_MONITORTONE:
    case LINE_REMOVE:
    case LINE_REQUEST:
    default:
      break;
  }
}

/***********************************************************************

FUNCTION: 
  DialingProc

PURPOSE: 
  Processes messages sent to the Dialing dialog box window. 

***********************************************************************/
BOOL CALLBACK DialingProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                           LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:

      // Set the global handle to the window.
      g_hwndDial = hwnd; 
      
      // Display the current dialing phone number.
      SetDlgItemText (hwnd, IDC_PHONENUM, g_szCurrentNum); 
      
      // Set the cursor as a wait cursor.
      SetCursor (LoadCursor (NULL, IDC_WAIT));

      break;

    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        // Drop the line, close the line, and then close the dialog box.
        // This is the case when the Hangup button gets pushed.
        case IDCANCEL:        
          
          // If lineMakeCall succeeds, then drop the call.
          if (g_hCall)
          {
            g_DropCallRequestID = lineDrop (g_hCall, NULL, 0);
            lineDeallocateCall (g_hCall);
          }

          // Close the current phone line.
          CurrentLineClose ();
          
          // Close the dialing dialog box.
          EndDialog (hwnd, FALSE);

          // Invalidate the dialing dialog box window handle.
          g_hwndDial = NULL;

          // Set the cursor back to an arrow.
          SetCursor (0);

          return TRUE;

        // Close the dialog box. This is the case when the 
        // application sends a message to close the dialog box.
        case IDOK:

           // Close the dialing dialog box.
          EndDialog (hwnd, TRUE);

          // Invalidate the dialing dialog box window handle.
          g_hwndDial = NULL;

          // Set the cursor back to an arrow.
          SetCursor (0);

          return TRUE;
      }
      break;
  }
  return FALSE;
}


/***********************************************************************

FUNCTION: 
  CurrentLineClose

PURPOSE: 
  This function closes the opened line device.  

***********************************************************************/
VOID CurrentLineClose ()
{
  // Close the current line.
  if (g_CurrentLineInfo.hLine)
    lineClose (g_CurrentLineInfo.hLine);

  // Reinitialize the variables.
  g_CurrentLineInfo.hLine = NULL;
  g_bCurrentLineAvail = TRUE;
  g_hCall = NULL;
}


/***********************************************************************

  GetLineInfo 

***********************************************************************/
DWORD GetLineInfo (DWORD dwLineID, LPLINEINFO lpLineInfo)
{
  DWORD dwSize,
        dwReturn;
  LPTSTR lpszLineName = NULL; 
  LPLINEDEVCAPS lpLineDevCaps = NULL;
  
  // Negotiate the API version number. If it fails, return to dwReturn.
  if (dwReturn = lineNegotiateAPIVersion (
        g_hLineApp,                   // TAPI registration handle
        dwLineID,                     // Line device to be queried
        TAPI_VERSION_1_0,             // Least recent API version 
        TAPI_CURRENT_VERSION,         // Most recent API version 
        &(lpLineInfo->dwAPIVersion),  // Negotiated API version 
        NULL))                        // Must be NULL; the provider-
                                      // specific extension is not 
                                      // supported on Windows CE
  {
    goto exit;
  }

  dwSize = sizeof (LINEDEVCAPS);

  // Allocate enough memory for lpLineDevCaps.
  do
  {
    if (!(lpLineDevCaps = (LPLINEDEVCAPS) LocalAlloc (LPTR, dwSize)))
    {
      dwReturn = LINEERR_NOMEM;
      goto exit;
    }

    lpLineDevCaps->dwTotalSize = dwSize;

    if (dwReturn = lineGetDevCaps (g_hLineApp,
                                   dwLineID,
                                   lpLineInfo->dwAPIVersion,
                                   0,
                                   lpLineDevCaps))
    {
      goto exit;
    }

    // Stop if the allocated memory is equal to or greater than the 
    // needed memory.
    if (lpLineDevCaps->dwNeededSize <= lpLineDevCaps->dwTotalSize)
      break;  

    dwSize = lpLineDevCaps->dwNeededSize;
    LocalFree (lpLineDevCaps);
    lpLineDevCaps = NULL;
    
  } while (TRUE);

  // Store the line information in *lpLineInfo.
  lpLineInfo->dwPermanentLineID = lpLineDevCaps->dwPermanentLineID;
  lpLineInfo->dwNumOfAddress = lpLineDevCaps->dwNumAddresses;
  lpLineInfo->bVoiceLine = 
    (lpLineDevCaps->dwMediaModes & LINEMEDIAMODE_INTERACTIVEVOICE);

  // Allocate memory for lpszLineName.
  if (!(lpszLineName = (LPTSTR) LocalAlloc (LPTR, 512)))
  {
    dwReturn = LINEERR_NOMEM;
    goto exit;
  }  
  
  // Store the line name in *lpszLineName.
  if (lpLineDevCaps->dwLineNameSize >= 512)
  {
    wcsncpy (
      lpszLineName, 
      (LPTSTR)((LPSTR)lpLineDevCaps + lpLineDevCaps->dwLineNameOffset),
      512);
  }
  else if (lpLineDevCaps->dwLineNameSize > 0)
  {
    wcsncpy (
      lpszLineName, 
      (LPTSTR)((LPSTR)lpLineDevCaps + lpLineDevCaps->dwLineNameOffset),
      lpLineDevCaps->dwLineNameSize);
  }
  else 
    wsprintf (lpszLineName, TEXT("Line %d"), dwLineID);

  // Copy lpszLineName to lpLineInfo->lpszLineName.
  lstrcpy (lpLineInfo->szLineName, lpszLineName);

  dwReturn = ERR_NONE;

exit:
    
  if (lpLineDevCaps)
    LocalFree (lpLineDevCaps);

  if (lpszLineName)
    LocalFree (lpszLineName);

  return dwReturn; 
}



/***********************************************************************

  InitializeTAPI 

***********************************************************************/
DWORD InitializeTAPI ()
{
  DWORD dwLineID,
        dwReturn,
        dwTimeCount = GetTickCount ();

  TCHAR szWarning[] = TEXT("Cannot initialize the application's use\n") 
                      TEXT("of tapi.dll. Quit all other telephony\n")
                      TEXT("programs, and then try again.");

  // Initialize the application's use of Tapi.dll. Keep trying until the
  // user cancels or stops getting LINEERR_REINIT.
  while ( (dwReturn = lineInitialize (&g_hLineApp, 
                                      g_hInst, 
                                      (LINECALLBACK) lineCallbackFunc, 
                                      g_szAppName, 
                                      &g_dwNumDevs)) == LINEERR_REINIT)
  {
    // Bring up the message box if 5 seconds have passed.
    if (GetTickCount () > 5000 + dwTimeCount)
    {
      if (MessageBox (g_hwndMain, szWarning, TEXT("Warning"), 
                      MB_OKCANCEL) == IDOK)
        break;
      
      // Reset the time counter.
      dwTimeCount = GetTickCount ();      
    }  
  }

  // If function "lineInitialize" fails, then return.
  if (dwReturn)
    return dwReturn;

  // If there is no device, then return.
  if (g_dwNumDevs == 0)
  {
    ErrorBox (TEXT("There are no line devices available."));
    return LINEERR_NODEVICE;
  }

  // Allocate buffer for storing LINEINFO for all the available lines.
  if (! (g_lpLineInfo = (LPLINEINFO) LocalAlloc (
                                      LPTR, 
                                      sizeof (LINEINFO) * g_dwNumDevs)))
  {
    return LINEERR_NOMEM;
  }

  // Fill lpLineInfo[] for every line.
  for (dwLineID = 0; dwLineID < g_dwNumDevs; ++dwLineID)
  {  
    GetLineInfo (dwLineID, &g_lpLineInfo [dwLineID]);
  }

  return ERR_NONE;
}


/***********************************************************************

  MakePhoneCall 

***********************************************************************/
VOID MakePhoneCall (LPCTSTR lpszPhoneNum)
{
  DWORD dwReturn,
        dwSizeOfTransOut = sizeof (LINETRANSLATEOUTPUT),
        dwSizeOfCallParams = sizeof (LINECALLPARAMS);

  LPLINECALLPARAMS lpCallParams = NULL;
  LPLINETRANSLATEOUTPUT lpTransOutput = NULL;
  
  TCHAR szDialablePhoneNum[TAPIMAXDESTADDRESSSIZE + 1] = {'\0'};

  // Initialize g_MakeCallRequestID.
  g_MakeCallRequestID = 0;
  
  // Open the current line.
  if (dwReturn = lineOpen (
          g_hLineApp,                 // Usage handle for TAPI
          g_dwCurrentLineID,          // Cannot use the LINEMAPPER value
          &g_CurrentLineInfo.hLine,   // Line handle
          g_CurrentLineInfo.dwAPIVersion, 
                                      // API version number
          0,                          // Must set to zero for Windows CE
          0,                          // No data passed back 
          LINECALLPRIVILEGE_NONE,     // Can only make an outgoing call
          0,                          // Media mode 
          NULL))                      // Must set to NULL for Windows CE
  {
    goto exit;
  }
  
  // Call translate address before dialing.
  do
  {
    // Allocate memory for lpTransOutput.
    if (!(lpTransOutput = (LPLINETRANSLATEOUTPUT) LocalAlloc (
                                                    LPTR,  
                                                    dwSizeOfTransOut)))
    {
      goto exit;
    }

    lpTransOutput->dwTotalSize = dwSizeOfTransOut;

    if (dwReturn = lineTranslateAddress (
          g_hLineApp,               // Usage handle for TAPI
          g_dwCurrentLineID,        // Line device identifier 
          g_CurrentLineInfo.dwAPIVersion, 
                                    // Highest TAPI version supported 
          lpszPhoneNum,             // Address to be translated
          0,                        // Must be 0 for Windows CE
          0,                        // No associated operations 
          lpTransOutput))           // Result of the address translation
    {
      goto exit;
    }
    
    if (lpTransOutput->dwNeededSize <= lpTransOutput->dwTotalSize)
      break; 
    else
    {
      dwSizeOfTransOut = lpTransOutput->dwNeededSize;
      LocalFree (lpTransOutput);
      lpTransOutput = NULL;
    }

  } while (TRUE);
     
  dwSizeOfCallParams += lpTransOutput->dwDisplayableStringSize;
  
  if (!(lpCallParams = (LPLINECALLPARAMS) LocalAlloc (
                                                  LPTR, 
                                                  dwSizeOfCallParams)))
  {
    goto exit;
  }

  // Set the call parameters.
  lpCallParams->dwTotalSize = dwSizeOfCallParams;
  lpCallParams->dwBearerMode = LINEBEARERMODE_VOICE;
  lpCallParams->dwMediaMode = LINEMEDIAMODE_DATAMODEM; 
  lpCallParams->dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;
  lpCallParams->dwAddressMode = LINEADDRESSMODE_ADDRESSID;
  lpCallParams->dwAddressID = g_dwCurrentLineAddr;
  lpCallParams->dwDisplayableAddressSize = 
                                lpTransOutput->dwDisplayableStringSize;
  lpCallParams->dwDisplayableAddressOffset = sizeof (LINECALLPARAMS);

  // Save the translated phone number for dialing.
  lstrcpy (szDialablePhoneNum, (LPTSTR)((LPSTR)lpTransOutput + \
           lpTransOutput->dwDisplayableStringOffset));

  // Set the cursor as the wait cursor.
  SetCursor (LoadCursor (NULL, IDC_WAIT));

  // Make the phone call. lpCallParams should be NULL if the default 
  // call setup parameters are requested.
  g_MakeCallRequestID = lineMakeCall (g_CurrentLineInfo.hLine,            
                                      &g_hCall,         
                                      szDialablePhoneNum, 
                                      0, 
                                      lpCallParams);    

  // Set the cursor back to an arrow.
  SetCursor (0);

  if (g_MakeCallRequestID > 0) 
  {
    g_bCurrentLineAvail = FALSE;

    DialogBoxParam (g_hInst, 
                    MAKEINTRESOURCE(IDD_DIALING), 
                    g_hwndMain, 
                    (DLGPROC) DialingProc, 0);
  }
  else 
  {
    ErrorBox (TEXT("Failed in making the phone call, function")
              TEXT("\nlineMakeCall failed."));
    CurrentLineClose ();
  }

exit :

  if (lpCallParams)
    LocalFree (lpCallParams);
  
  if (lpTransOutput)
    LocalFree (lpTransOutput);

  // If the make call did not succeed but the line was opened, 
  // then close it.
  if ((g_MakeCallRequestID <= 0) && (g_CurrentLineInfo.hLine))
    CurrentLineClose ();

  return;
}


/***********************************************************************

  ConnectUsingProc 

***********************************************************************/
BOOL CALLBACK ConnectUsingProc (HWND hwnd, UINT uMsg, WPARAM wParam, 
                                LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
    {
      // Get the list of lines into the line list box.
      InitLineCB (GetDlgItem (hwnd, IDC_LISTLINES), 
                  g_dwCurrentLineID, 
                  g_dwNumDevs);

      // Get the list of addresses into the address list box.
      InitAddrCB (GetDlgItem (hwnd, IDC_LISTLINES),
                  GetDlgItem (hwnd, IDC_LISTADDRESSES),
                  g_dwCurrentLineID, 
                  g_dwCurrentLineAddr);

      return TRUE;
    }

    case WM_COMMAND:
    {
      switch (LOWORD(wParam))
      {
        case IDC_LISTLINES:

          if (HIWORD(wParam) == CBN_SELENDOK)
          {
            // Update the address box.
            InitAddrCB (GetDlgItem (hwnd, IDC_LISTLINES),
                        GetDlgItem (hwnd, IDC_LISTADDRESSES),
                        g_dwCurrentLineID, 
                        g_dwCurrentLineAddr);
          }
          break;

        case IDOK:
        {
          long lIndex; 

          lIndex = SendDlgItemMessage (hwnd, 
                                       IDC_LISTLINES, 
                                       CB_GETCURSEL, 0, 0);

          // Update the current line identifier.
          g_dwCurrentLineID = SendDlgItemMessage (
                                       hwnd, 
                                       IDC_LISTLINES, 
                                       CB_GETITEMDATA, 
                                       (WPARAM) lIndex, 0);

          lIndex = SendDlgItemMessage (hwnd, 
                                       IDC_LISTADDRESSES, 
                                       CB_GETCURSEL, 0, 0);

          // Update the current address for the current line.
          g_dwCurrentLineAddr = SendDlgItemMessage (
                                       hwnd, 
                                       IDC_LISTADDRESSES, 
                                       CB_GETITEMDATA,
                                       (WPARAM) lIndex, 0);
          
          // Assign the g_CurrentLineInfo.
          g_CurrentLineInfo = g_lpLineInfo [g_dwCurrentLineID];  
        }

        case IDCANCEL:
          EndDialog (hwnd, FALSE);
          return TRUE;
      }
    }
  }
  return FALSE;
}


/***********************************************************************

  InitLineCB 

***********************************************************************/
BOOL InitLineCB (HWND hwndLineCB, DWORD dwCurrentLine, DWORD dwNumOfDev)
{
  DWORD dwItem,
        dwLineID,
        dwCurrentItem = (DWORD)-1;
  
  // Empty the line combo box.
  SendMessage (hwndLineCB, CB_RESETCONTENT, 0, 0); 

  // Add the device name strings.
  for (dwLineID = 0; dwLineID < dwNumOfDev; ++dwLineID)
  {
    // Add the line name string to the list box of the line combo box.
    dwItem = SendMessage (hwndLineCB,
                          CB_ADDSTRING,
                          0,
                          (LPARAM)(g_lpLineInfo[dwLineID].szLineName));

    if (dwItem == CB_ERR || dwItem == CB_ERRSPACE)
      return FALSE; 

    // Set the dwLineID associated with the dwItem item.
    SendMessage (hwndLineCB, CB_SETITEMDATA, (WPARAM)dwItem, 
                 (LPARAM)dwLineID);

    if (dwLineID == dwCurrentLine)
      dwCurrentItem = dwItem;
    else 
    {  
      // If the item we are putting is before the current item, we 
      // must increment dwCurrentItem to reflect that an item is 
      // being placed before it, due to sorting.
      if (dwCurrentItem != -1 && dwItem <= dwCurrentItem)
        ++dwCurrentItem;
    }
  }

  if (dwCurrentItem == (DWORD)-1)
    dwCurrentItem = 0;

  // Select the dwCurrentItem item in the list box of the line  
  // combo box.
  if (SendMessage (hwndLineCB, CB_GETCOUNT, 0, 0) != 0)
  {
    SendMessage (hwndLineCB, CB_SETCURSEL, (WPARAM)dwCurrentItem, 0);
    return TRUE;
  }

  return FALSE;
}


/***********************************************************************

  InitAddrCB 

***********************************************************************/
BOOL InitAddrCB (HWND hwndLineCB, HWND hwndAddrCB, 
                 DWORD dwCurrentLine, DWORD dwCurrentAddr)
{
  DWORD dwAddr, 
        dwItem, 
        dwCurrentItem = (DWORD)-1,
        dwLineCBCurrent;

  TCHAR szAddrName[512];

  if (SendMessage (hwndLineCB, CB_GETCOUNT, 0, 0) == 0)
    return FALSE;

  // Empty the address list box.
  SendMessage (hwndAddrCB, CB_RESETCONTENT, 0, 0); 
  
  // Select the current entry in the line box.
  dwLineCBCurrent = SendMessage (
                        hwndLineCB,
                        CB_GETITEMDATA,
                        SendMessage (hwndLineCB, CB_GETCURSEL, 0, 0), 
                        0);

  // Get all the addresses for this line.
  for (dwAddr = 0; 
       dwAddr < g_lpLineInfo [dwLineCBCurrent].dwNumOfAddress; ++dwAddr)
  {
    wsprintf (szAddrName, TEXT("Address %d"), dwAddr);

    // Add the address name string to the list box of the address 
    // combo box.
    dwItem = SendMessage (hwndAddrCB, CB_ADDSTRING, 0, 
                          (LPARAM)szAddrName);

    if (dwItem == CB_ERR || dwItem == CB_ERRSPACE)
      return FALSE; 

    // Set the dwAddr associated with the dwItem item.
    SendMessage (hwndAddrCB, CB_SETITEMDATA, (WPARAM)dwItem, 
                 (LPARAM)dwAddr);

    if (dwLineCBCurrent == dwCurrentLine)
    {
      if (dwAddr == dwCurrentAddr)
        dwCurrentItem = dwItem;
      else 
      {
        // If the item we are putting is before the current item, we
        // must increment dwItemCur to reflect that an item is being
        // placed before it, due to sorting.
        if (dwCurrentItem != -1 && dwItem <= dwCurrentItem)
          ++dwCurrentItem; 
      }
    }
  }
  
  if (dwLineCBCurrent != dwCurrentLine || dwCurrentItem == (DWORD)-1)
    dwCurrentItem = 0;

  // Select the dwCurrentItem item in the list box of the line 
  // combo box.
  if (SendMessage (hwndAddrCB, CB_GETCOUNT, 0, 0) != 0)
  {
    SendMessage (hwndAddrCB, CB_SETCURSEL, (WPARAM)dwCurrentItem, 0);
    return TRUE;
  }

  return FALSE;
}


/***********************************************************************

  MakeCanonicalNum 

***********************************************************************/
BOOL MakeCanonicalNum (LPTSTR lpszPhoneNum)
{
  int index,
      iLength = 0,
      iStartPos = 0;

  TCHAR szCanPhoneNum[TAPIMAXDESTADDRESSSIZE + 1];

  for (index = 0; index < (int) wcslen (lpszPhoneNum); ++index)
  {
    if (iswdigit (lpszPhoneNum[index]))
    {
      lpszPhoneNum[iLength] = lpszPhoneNum[index];
      iLength += 1;
    }
  }

  // Terminate the string with NULL.
  lpszPhoneNum[iLength] = '\0';

  // If the first number is 1, reduce the phone number length by 1.
  if (lpszPhoneNum[0] == '1') 
  {
    iLength -= 1;
    iStartPos = 1;
  }

  // If the phone number length is less than 7, return FALSE.
  if (iLength < 7)
  {
    if (iLength == 6 && iStartPos == 1) 
    {
      ErrorBox (TEXT("The first digit can not be 1 for a ")
                 TEXT("\nlocal phone number."));
      return FALSE;
    }

    ErrorBox (TEXT("There should be at least 7 digits for")
               TEXT("\na local phone number."));
    return FALSE;
  }

  if (iLength == 7)
  {
    //Make the phone number in the format "+1 (xxx) xxx-xxxx"
    wcscpy (szCanPhoneNum, TEXT("+1 (425) "));
    szCanPhoneNum[9] = lpszPhoneNum[iStartPos];
    szCanPhoneNum[10] = lpszPhoneNum[iStartPos + 1];
    szCanPhoneNum[11] = lpszPhoneNum[iStartPos + 2];
    szCanPhoneNum[12] = '-';
    szCanPhoneNum[13] = lpszPhoneNum[iStartPos + 3];
    szCanPhoneNum[14] = lpszPhoneNum[iStartPos + 4];
    szCanPhoneNum[15] = lpszPhoneNum[iStartPos + 5];
    szCanPhoneNum[16] = lpszPhoneNum[iStartPos + 6];
    szCanPhoneNum[17] = '\0';

    // Copy the newly created phone number back to lpszPhoneNum.
    wcscpy (lpszPhoneNum, szCanPhoneNum);

    return TRUE;
  }

  if (iLength == 10)
  {
    //Make the phone number in the format "+1 (xxx) xxx-xxxx"
    wcscpy (szCanPhoneNum, TEXT("+1 ("));
    szCanPhoneNum[4] = lpszPhoneNum[iStartPos];
    szCanPhoneNum[5] = lpszPhoneNum[iStartPos + 1];
    szCanPhoneNum[6] = lpszPhoneNum[iStartPos + 2];
    szCanPhoneNum[7] = ')';
    szCanPhoneNum[8] = ' ';
    szCanPhoneNum[9] = lpszPhoneNum[iStartPos + 3];
    szCanPhoneNum[10] = lpszPhoneNum[iStartPos + 4];
    szCanPhoneNum[11] = lpszPhoneNum[iStartPos + 5];
    szCanPhoneNum[12] = '-';
    szCanPhoneNum[13] = lpszPhoneNum[iStartPos + 6];
    szCanPhoneNum[14] = lpszPhoneNum[iStartPos + 7];
    szCanPhoneNum[15] = lpszPhoneNum[iStartPos + 8];
    szCanPhoneNum[16] = lpszPhoneNum[iStartPos + 9];
    szCanPhoneNum[17] = '\0';

    // Copy the newly created phone number back to lpszPhoneNum.
    wcscpy (lpszPhoneNum, szCanPhoneNum);

    return TRUE;
  }

  ErrorBox (TEXT("This is not a valid phone number, please check the\n")
            TEXT("phone number and enter again."));

  return FALSE;
}
// END CEDIALER.C
