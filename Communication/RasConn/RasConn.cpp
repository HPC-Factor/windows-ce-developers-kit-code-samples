/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  RasConn.c

ABSTRACT: 
  This code sample shows how to enumerate, create, delete, copy, and 
  rename phone book entries, and establish and close a Remote Access 
  Service (RAS) connection.

***********************************************************************/

#include <windows.h>
#include <commctrl.h>
#include <ras.h>
#include <raserror.h>
#include "resource.h"

HWND hMainWnd;                        // Main window handle
HWND hCmdBarWnd;                      // Command bar window handle
HWND hDialingWnd;                     // Dialing window handle
HMENU hMainMenu;                      // Main menu handle
HINSTANCE hInst;                      // hInstance of the application
TCHAR szTitle[] = TEXT("RasConn");    // Main window name
TCHAR szAppName[] = TEXT("RasConn Sample Application");
                                      // Main window class name
TCHAR szDomain[100];                  // Domain name
TCHAR szUserName[100];                // User name
TCHAR szPassword[100];                // Password
BOOL bUseCurrent = FALSE;             // Indicate if the current user 
                                      // name and password are used
TCHAR szRasEntryName[RAS_MaxEntryName + 1];                
                                      // Current RAS entry name
HRASCONN hRasConn = NULL;             // RAS connection handle
RASDIALPARAMS RasDialParams;          // Parameters used by RasDial 
LPRASENTRYNAME lpRasEntryName = NULL; // Contains all entry names in a 
                                      // remote access phone book 
BOOL GetPhonebookEntries (HWND);
BOOL MakeRasDial (HWND);
BOOL CALLBACK PhoneBookDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AuthDlgProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialingDlgProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);

/***********************************************************************

FUNCTION:
  RenameRasEntry

PURPOSE:
  This subroutine changes the name of an entry in the RAS phone book. 
  It first calls RasValidateEntryName to validate the new name and then 
  calls RasRenameEntry to change the name. The return value is TRUE if
  successful, and FALSE if not.

***********************************************************************/
BOOL RenameRasEntry (LPTSTR lpszOldName, LPTSTR lpszNewName)
{
  DWORD dwError;            // Returned code from functions 
  TCHAR szError[120];       // Buffer for error message 

  if (dwError = RasValidateEntryName (NULL, lpszNewName))
  {
    wsprintf (szError, TEXT("Entry name validation failed: %ld"), 
              dwError);
    return FALSE;
  }

  if (dwError = RasRenameEntry (NULL, lpszOldName, lpszNewName))
  {
    wsprintf (szError, TEXT("Unable to rename entry: %ld"), dwError);
    return FALSE;
  }

  return TRUE;
}


/***********************************************************************

FUNCTION:
  DeleteRasEntry

PURPOSE:
  This subroutine deletes an entry from the RAS phone book. The return 
  value is TRUE if successful, and FALSE if not.

***********************************************************************/
BOOL DeleteRasEntry (LPTSTR lpszName)
{
  DWORD dwError;            // Return code from function RasDeleteEntry
  TCHAR szError[100];       // Buffer for error message 

  if (dwError = RasDeleteEntry (NULL, lpszName))
  {
    wsprintf (szError, TEXT("Unable to delete entry: %ld"), dwError);
    return FALSE;
  }

  return TRUE;
}


/***********************************************************************

FUNCTION:
  CreateRasEntry

PURPOSE:
  This function shows how to create a RAS entry.

***********************************************************************/
int CreateRasEntry (LPTSTR lpszName)
{
  DWORD dwSize,
        dwError;
  TCHAR szError[100];
  RASENTRY RasEntry;
  RASDIALPARAMS  RasDialParams;

  // Validate the format of a connection entry name.
  if (dwError = RasValidateEntryName (NULL, lpszName))
  {
    wsprintf (szError, TEXT("Unable to validate entry name.")
              TEXT(" Error %ld"), dwError);  

    return FALSE;
  }

  // Initialize the RASENTRY structure.
  memset (&RasEntry, 0, sizeof (RASENTRY));

  dwSize = sizeof (RASENTRY);
  RasEntry.dwSize = dwSize;

  // Retrieve the entry properties.
  if (dwError = RasGetEntryProperties (NULL, TEXT(""),      
                      (LPBYTE)&RasEntry, &dwSize, NULL, NULL))  
  {
    wsprintf (szError, TEXT("Unable to read default entry properties.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  // Insert code here to fill up the RASENTRY structure.
  // ...
  
  // Create a new phone book entry.
  if (dwError = RasSetEntryProperties (NULL, lpszName, 
                      (LPBYTE)&RasEntry, sizeof (RASENTRY), NULL, 0))
  {
    wsprintf (szError, TEXT("Unable to create the phonebook entry.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  // Initialize the RASDIALPARAMS structure.
  memset (&RasDialParams, 0, sizeof (RASDIALPARAMS));
  RasDialParams.dwSize = sizeof (RASDIALPARAMS);
  _tcscpy (RasDialParams.szEntryName, lpszName);
  
  // Insert code here to fill up the RASDIALPARAMS structure.
  // ...

  // Change the connection information.
  if (dwError = RasSetEntryDialParams (NULL, &RasDialParams, FALSE))
  {
    wsprintf (szError, TEXT("Unable to set the connection information.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  return TRUE;
}

/***********************************************************************

FUNCTION:
  CopyRasEntry

PURPOSE:
  This subroutine copies the RAS entry named lpszEntryName into a new
  entry named lpszEntryName1. Functions RasGetEntryProperties and
  RasSetEntryProperties are called to retrieve and save most entry
  information, while RasGetEntryDialParams and RasSetEntryDialParams 
  are called to handle user name and password information. The return 
  value is TRUE if successful, and FALSE if not.

***********************************************************************/
BOOL CopyRasEntry (LPTSTR lpszEntryName)
{
  BOOL bPasswordSaved;              
  TCHAR szNewEntryName[100];      // Name for the copied entry
  TCHAR szError[100];             // Buffer for the error message
  DWORD dwError,                  // Returned code from the functions
        dwRasEntrySize,           // Size of the RASENTRY structure
        dwDevConfigSize,          // Size of DevConfigBuf
        dwDeviceNum = 0xFFFFFFFF; // TAPI device number 
  BYTE DevConfigBuf[128];         // Buffer for device configuration 
                                  // information
  RASENTRY RasEntry;              // RASENTRY structure
  RASDIALPARAMS RasDialParams;    // RASDIALPARAMS structure 
  LPVARSTRING lpDevConfig = (LPVARSTRING)&DevConfigBuf;
                                  // Pointer to the memory location of
                                  // the device configuration structure

  // Assign the name for the copied phone book entry.
  wsprintf (szNewEntryName, TEXT("%s1"), lpszEntryName);
    
  // Validate the format of a connection entry name.
  if (dwError = RasValidateEntryName (NULL, szNewEntryName))
  {
    wsprintf (szError, TEXT("Unable to validate entry name.")
              TEXT(" Error %ld"), dwError);  

    return FALSE;
  }

  dwDevConfigSize = sizeof (DevConfigBuf);
  dwRasEntrySize = sizeof (RASENTRY);
  RasEntry.dwSize = dwRasEntrySize;

  // Retrieve the entry properties.
  if (dwError = RasGetEntryProperties (NULL,               
                                       lpszEntryName,      
                                       (LPBYTE)&RasEntry,  
                                       &dwRasEntrySize,    
                                       DevConfigBuf,       
                                       &dwDevConfigSize))  
  {
    wsprintf (szError, TEXT("Unable to read entry properties.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  memset (&RasDialParams, 0, sizeof (RasDialParams));
  RasDialParams.dwSize = sizeof (RASDIALPARAMS);
  _tcscpy (RasDialParams.szEntryName, lpszEntryName);

  // Retrieve the connection information.
  if (dwError = RasGetEntryDialParams (NULL, &RasDialParams, 
                      &bPasswordSaved))
  {
    wsprintf (szError, TEXT("Unable to get the connection information.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  // Create a new phone book entry.
  if (dwError = RasSetEntryProperties (NULL, 
                                       szNewEntryName, 
                                       (LPBYTE)&RasEntry,
                                       dwRasEntrySize, 
                                       DevConfigBuf, 
                                       dwDevConfigSize))
  {
    wsprintf (szError, TEXT("Unable to copy the phonebook entry.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }
  
  _tcscpy (RasDialParams.szEntryName, szNewEntryName);

  // Change the connection information.
  if (dwError = RasSetEntryDialParams (NULL, &RasDialParams, FALSE))
  {
    wsprintf (szError, TEXT("Unable to set the connection information.")
              TEXT(" Error %ld"), dwError);
    return FALSE;
  }

  return TRUE;
}


/***********************************************************************

FUNCTION:
  CloseRasConnections

PURPOSE:
  Checks for the open RAS connections and attempts to close any that 
  are found. The return value is an error code (0 if no error).

***********************************************************************/
DWORD CloseRasConnections ()
{
  int index;                // Integer index
  TCHAR szError[100];       // Buffer for error codes 
  DWORD dwError,            // Error code from a function call 
        dwRasConnSize,      // Size of RasConn in bytes
        dwNumConnections;   // Number of connections found 
  RASCONN RasConn[20];      // Buffer for connection state information 
                            // Assume the maximum number of entry is 20

  // Assume there are no more than 20 connections.
  RasConn[0].dwSize = sizeof (RASCONN);
  dwRasConnSize = 20 * sizeof (RASCONN);

  // Find all of the connections.
  if (dwError = RasEnumConnections (RasConn, &dwRasConnSize, 
                                    &dwNumConnections))
  {
    wsprintf (szError, TEXT("RasEnumConnections Error: %ld"), dwError);
    return dwError;
  }

  // If there are no connections, return 0.
  if (!dwNumConnections)
  {
    wsprintf (szError, TEXT("No open RAS connections"));
    return 0;
  }

  // Terminate all of the remote access connections.
  for (index = 0; index < (int)dwNumConnections; ++index)
  {
    if (dwError = RasHangUp (RasConn[index].hrasconn))
    {
      wsprintf (szError, TEXT("RasHangUp Error: %ld"), dwError);
      return dwError;
    }
  }

  return 0;
}

/***********************************************************************

FUNCTION:
  GetPhonebookEntries

PURPOSE:
  Shows how to enumerate the phone book entries and add
  all entry names to a list control.

***********************************************************************/
BOOL GetPhonebookEntries (HWND hDlgWnd)
{
  int index;
  DWORD dwSize, 
        dwEntries;
  HWND hWndListBox;

  // Allocate an array of RASENTRYNAME structures. Assume that there
  // will be no more than 20 entries configured on the windows CE 
  // device.
  if (!(lpRasEntryName = new RASENTRYNAME [20]))
  {
    MessageBox (hDlgWnd, TEXT("Not enough memory"), szTitle, MB_OK);
    return FALSE;
  }

  // You must initialize the dwSize member of the first RASENTRYNAME 
  // structure in the array to the size of the structure in order to 
  // identify the version of the structure being passed.
  lpRasEntryName[0].dwSize = sizeof (RASENTRYNAME);

  // Size of the array in bytes
  dwSize = sizeof (RASENTRYNAME) * 20;

  // List all entry names in a remote access phone book.
  if ((RasEnumEntries (
          NULL,               // Reserved, must be NULL
          NULL,               // Phone book is stored in the registry
          lpRasEntryName,     // Buffer to receive the entries
          &dwSize,            // Size of the buffer in bytes
          &dwEntries)) != 0)  // Number of entries written to the 
                              // buffer
  {
    MessageBox (hDlgWnd, TEXT("Could not obtain RAS entries"), szTitle,
                MB_OK);
    return FALSE;
  }

  // Get the HWND of the listbox control.
  hWndListBox = GetDlgItem (hDlgWnd, IDC_RASNAMES);

  // Remove all items from the listbox.
  SendMessage (hWndListBox, LB_RESETCONTENT, 0, 0);

  // Add the names of each RAS connection to the list box.
  for (index = 0; index < (int)dwEntries; ++index)
  {
    SendMessage (hWndListBox, LB_INSERTSTRING, index, 
                 (LPARAM)lpRasEntryName[index].szEntryName);
  }

  return TRUE;
}


/***********************************************************************

FUNCTION:
  MakeRasDial

PURPOSE:
  Shows how to establish a RAS connection between the windows CE device
  and the remote access server.

***********************************************************************/
BOOL MakeRasDial (HWND hDlgWnd)
{
  BOOL bPassword;
  DWORD dwError;
  TCHAR szBuffer[100];

  if (bUseCurrent)
  {
    // Get the last configuration parameters used for this connection. 
    // If the password was saved, then the login dialog box will not be
    // displayed.
    if (RasGetEntryDialParams (NULL, &RasDialParams, &bPassword) != 0)
    {
      MessageBox (hDlgWnd, 
                  TEXT("Could not get parameter details"), 
                  szTitle, 
                  MB_OK);
      return FALSE;
    }
  }
  else
  {
    // Bring up the Authentication dialog box.
    DialogBox (hInst, MAKEINTRESOURCE(IDD_AUTHDLG), hDlgWnd, 
               AuthDlgProc);

    // Set hRasConn to NULL before attempting to connect.
    hRasConn = NULL;

    // Initialize the structure.
    memset (&RasDialParams, 0, sizeof (RASDIALPARAMS));

    // Configure the RASDIALPARAMS structure. 
    RasDialParams.dwSize = sizeof (RASDIALPARAMS);
    RasDialParams.szPhoneNumber[0] = TEXT('\0');
    RasDialParams.szCallbackNumber[0] = TEXT('\0');
    wcscpy (RasDialParams.szEntryName, szRasEntryName);
    wcscpy (RasDialParams.szUserName, szUserName);
    wcscpy (RasDialParams.szPassword, szPassword);
    wcscpy (RasDialParams.szDomain, szDomain); 
  }

  // Try to establish a RAS connection.
  if ((dwError = RasDial (
          NULL,             // Extension is not supported in Windows CE
          NULL,             // Phone book is in the registry
          &RasDialParams,   // RAS configuration for the connection
          0xFFFFFFFF,       // Must use this value for Windows CE
          hDlgWnd,          // Window receives the notification message
          &hRasConn)) != 0) // Connection handle
  {
    wsprintf (szBuffer, TEXT("Could not connect using RAS. Error %x"), 
              dwError);
    MessageBox (hDlgWnd, szBuffer, szTitle, MB_OK);
    return FALSE;
  }

  wsprintf (szBuffer, TEXT("Dialing %s..."), szRasEntryName);

  // Set the dialing dialog box window name to szBuffer.
  SetWindowText (hDlgWnd, szBuffer);

  return TRUE;
}

/***********************************************************************

FUNCTION:
  GetConnStatus

PURPOSE:
  Gets the status of the current RAS connection. The return value is TRUE
  if successful, FALSE if not.

***********************************************************************/
BOOL GetConnStatus ()
{      
  DWORD dwReturn;
  TCHAR szBuffer[100];
  RASCONNSTATUS RasStatus;  

  // Check if hRasConn is a valid RAS connection handle. 
  if (hRasConn == NULL)
    return FALSE;

  // Get the connection status.
  RasStatus.dwSize = sizeof (RASCONNSTATUS);
  dwReturn = RasGetConnectStatus (hRasConn, &RasStatus);

  // If there is an error in getting the connection status, then return
  // FALSE.
  if (dwReturn)
  {
    wsprintf (szBuffer, TEXT("Failed getting connect status.\r\n")
              TEXT("Error (%ld)."), dwReturn);
    MessageBox (hMainWnd, szBuffer, TEXT("Warning"), MB_OK);
    return FALSE;
  }

  // Check if the Status dialog box has been created.
  if (!hDialingWnd)
    return TRUE;

  // Check if there is an error.
  if (RasStatus.dwError)
    wsprintf (szBuffer, TEXT("Error (%ld)"), RasStatus.dwError);
  else
    wsprintf (szBuffer, TEXT("No error"));

  // Display the error, device type, and device name.
  SetDlgItemText (hDialingWnd, IDC_ERROR, szBuffer);
  SetDlgItemText (hDialingWnd, IDC_DEVICETYPE, RasStatus.szDeviceType);
  SetDlgItemText (hDialingWnd, IDC_DEVICENAME, RasStatus.szDeviceName);

  if (RasStatus.rasconnstate == RASCS_Connected || 
      RasStatus.rasconnstate == RASCS_Disconnected)
    SetDlgItemText (hDialingWnd, IDC_PROGRESS, TEXT("DONE!"));
  else
  {
    if (RasStatus.dwError)
    {
      SetDlgItemText (hDialingWnd, IDC_PROGRESS, 
                      TEXT("DONE! ERROR OCCURED."));
      SetCursor (0);
    }
    else
      SetDlgItemText (hDialingWnd, IDC_PROGRESS, TEXT("WAIT...."));
  }

  return TRUE;
}

/***********************************************************************

FUNCTION:
  PhoneBookDlgProc

PURPOSE:
  Processes messages sent to the Phonebook Entries dialog box window.

***********************************************************************/
BOOL CALLBACK PhoneBookDlgProc (HWND hDlgWnd, UINT uMsg, WPARAM wParam, 
                                LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      GetPhonebookEntries (hDlgWnd);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_USECURRENT:
          bUseCurrent = (bUseCurrent) ? FALSE : TRUE;
          return TRUE;

        case IDC_CREATE:
          CreateRasEntry (TEXT("New Entry"));
          GetPhonebookEntries (hDlgWnd);
          break;

        case IDC_COPY:
        {
          // Get the HWND of the list box control.
          HWND hWndListBox = GetDlgItem (hDlgWnd, IDC_RASNAMES);
          
          // Find out which RAS entry was selected.
          DWORD dwSel = SendMessage (hWndListBox, LB_GETCURSEL, 0, 0);

          if (dwSel == LB_ERR)
          {
            MessageBox (hDlgWnd, 
                        TEXT("Failed in selecting a RAS connection!"),
                        TEXT("Error"), 
                        MB_OK);
            szRasEntryName[0] = TEXT('\0');
          }
          else
          {
            CopyRasEntry (lpRasEntryName[dwSel].szEntryName);
            GetPhonebookEntries (hDlgWnd);
          }

          return TRUE;
        }

        case IDC_RENAME:
        {
          // Get the HWND of the list box control.
          HWND hWndListBox = GetDlgItem (hDlgWnd, IDC_RASNAMES);
          
          // Find out which RAS entry was selected.
          DWORD dwSel = SendMessage (hWndListBox, LB_GETCURSEL, 0, 0);

          if (dwSel == LB_ERR)
          {
            MessageBox (hDlgWnd, 
                        TEXT("Failed in selecting a RAS connection!"),
                        TEXT("Error"), 
                        MB_OK);
            szRasEntryName[0] = TEXT('\0');
          }
          else
          {
            // Insert code here to prompt users to enter the new name.
            // Hard code the new entry name as RenamedEntry.
            RenameRasEntry (lpRasEntryName[dwSel].szEntryName, 
                            TEXT("Renamed Entry"));
            GetPhonebookEntries (hDlgWnd);
          }

          return TRUE;
        }

        case IDC_DELETE:
        {
          // Get the HWND of the list box control.
          HWND hWndListBox = GetDlgItem (hDlgWnd, IDC_RASNAMES);
          
          // Find out which RAS entry was selected.
          DWORD dwSel = SendMessage (hWndListBox, LB_GETCURSEL, 0, 0);

          if (dwSel == LB_ERR)
          {
            MessageBox (hDlgWnd, 
                        TEXT("Failed in selecting a RAS connection!"),
                        TEXT("Error"), 
                        MB_OK);
            szRasEntryName[0] = TEXT('\0');
          }
          else
          {
            DeleteRasEntry (lpRasEntryName[dwSel].szEntryName);
            GetPhonebookEntries (hDlgWnd);
          }

          return TRUE;
        }

        case IDOK:
        { 
          // Get the HWND of the list box control.
          HWND hWndListBox = GetDlgItem (hDlgWnd, IDC_RASNAMES);
          
          // Find out which RAS entry was selected.
          DWORD dwSel = SendMessage (hWndListBox, LB_GETCURSEL, 0, 0);

          if (dwSel == LB_ERR)
          {
            MessageBox (hDlgWnd, 
                        TEXT("Failed in selecting a RAS connection!"),
                        TEXT("Error"), 
                        MB_OK);
            szRasEntryName[0] = TEXT('\0');
          }
          else
            wcscpy (szRasEntryName, lpRasEntryName[dwSel].szEntryName);

          if (lpRasEntryName)
          {
            delete [] lpRasEntryName;
            lpRasEntryName = NULL;
          }

          EndDialog (hDlgWnd, IDOK);
          return TRUE;
        }

        case IDCANCEL:
          if (lpRasEntryName)
          {
            delete [] lpRasEntryName;
            lpRasEntryName = NULL;
          }

          EndDialog (hDlgWnd, IDCANCEL);
          return TRUE;
      }
      return 0;
  }
  return 0;
}


/***********************************************************************

FUNCTION:
  AuthDlgProc

PURPOSE:
  Processes messages sent to the Authentication dialog box window.

***********************************************************************/
BOOL CALLBACK AuthDlgProc (HWND hDlgWnd, UINT uMsg, WPARAM wParam, 
                           LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      // Set text limits on edit controls.
      SendDlgItemMessage (hDlgWnd, IDC_USERNAME, EM_LIMITTEXT, 100, 0);
      SendDlgItemMessage (hDlgWnd, IDC_PASSWORD, EM_LIMITTEXT, 100, 0);
      SendDlgItemMessage (hDlgWnd, IDC_DOMAIN, EM_LIMITTEXT, 100, 0);

      // Reset password.
      szPassword[0] = '\0';
      
      SetDlgItemText (hDlgWnd, IDC_USERNAME, szUserName);
      SetDlgItemText (hDlgWnd, IDC_PASSWORD, szPassword);
      SetDlgItemText (hDlgWnd, IDC_DOMAIN, szDomain);
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          GetDlgItemText (hDlgWnd, IDC_USERNAME, szUserName, 100);
          GetDlgItemText (hDlgWnd, IDC_PASSWORD, szPassword, 100);
          GetDlgItemText (hDlgWnd, IDC_DOMAIN, szDomain, 100);

          EndDialog (hDlgWnd, IDOK);
          return TRUE;
      }
      return TRUE;
  }

  return FALSE;
}

/***********************************************************************

FUNCTION:
  DialingDlgProc

PURPOSE:
  Processes messages sent to the Dialing dialog box window.

***********************************************************************/
BOOL CALLBACK DialingDlgProc (HWND hDlgWnd, UINT uMsg, WPARAM wParam, 
                              LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      if (!MakeRasDial (hDlgWnd))
        DestroyWindow (hDlgWnd);
      else
        SetCursor (LoadCursor (NULL, IDC_WAIT));

      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_HIDE:
          ShowWindow (hDlgWnd, FALSE);
          break;

        case IDC_HANGUP:
          CloseRasConnections ();

          EnableMenuItem (hMainMenu, IDM_CONNECTION_STATUS, 
                          MF_BYCOMMAND | MF_GRAYED);
          EnableMenuItem (hMainMenu, IDM_CONNECTION_HANGUP, 
                          MF_BYCOMMAND | MF_GRAYED);

          DestroyWindow (hDlgWnd);
          SetCursor (0);
          break;
      }
      return TRUE;

    // RAS notification message. 
    case WM_RASDIALEVENT:
    {
      LPTSTR lpszStatus;
        
      switch (wParam)
      {
        case RASCS_OpenPort:   
          lpszStatus = TEXT("Opening port");
          break;
        case RASCS_PortOpened:     
          lpszStatus = TEXT("Port opened");
          break;
        case RASCS_ConnectDevice: 
          lpszStatus = TEXT("Connecting device");
          break;
        case RASCS_DeviceConnected:  
          lpszStatus = TEXT("Device connected"); 
          break;  
        case RASCS_AllDevicesConnected: 
          lpszStatus = TEXT("All device connected");
          break;
        case RASCS_Authenticate:  
          lpszStatus = TEXT("Starting authenticating");   
          break;
        case RASCS_AuthNotify:  
          lpszStatus = TEXT("The authentication event has\noccurred"); 
          break;  
        case RASCS_AuthRetry: 
          lpszStatus = TEXT("RASCS_AuthRetry");
          break;
        case RASCS_AuthCallback: 
          lpszStatus = TEXT("Requested a callback number");  
          break;  
        case RASCS_AuthChangePassword: 
          lpszStatus = TEXT("RASCS_AuthChangePassword");   
          break; 
        case RASCS_AuthProject: 
          lpszStatus = TEXT("RASCS_AuthProject");
          break;
        case RASCS_AuthLinkSpeed: 
          lpszStatus = TEXT("RASCS_AuthLinkSpeed");  
          break;  
        case RASCS_AuthAck:  
          lpszStatus = TEXT("RASCS_AuthAck");
          break;
        case RASCS_ReAuthenticate: 
          lpszStatus = TEXT("RASCS_ReAuthenticate");
          break;
        case RASCS_Authenticated:    
          lpszStatus = TEXT("RASCS_Authenticated"); 
          break;
        case RASCS_PrepareForCallback: 
          lpszStatus = TEXT("RASCS_PrepareForCallback");
          break;
        case RASCS_WaitForModemReset: 
          lpszStatus = TEXT("RASCS_WaitForModemReset");   
          break; 
        case RASCS_WaitForCallback:   
          lpszStatus = TEXT("RASCS_WaitForCallback"); 
          break;
        case RASCS_Projected: 
          lpszStatus = TEXT("RASCS_Projected"); 
          break;
        case RASCS_Interactive: 
          lpszStatus = TEXT("RASCS_Interactive"); 
          break;
        case RASCS_RetryAuthentication: 
          lpszStatus = TEXT("RASCS_RetryAuthentication"); 
          break;
        case RASCS_CallbackSetByCaller: 
          lpszStatus = TEXT("RASCS_CallbackSetByCaller"); 
          break;
        case RASCS_PasswordExpired: 
          lpszStatus = TEXT("RASCS_PasswordExpired"); 
          break;

        case RASCS_Connected: 
          lpszStatus = TEXT("Connected"); 
          SetCursor (0);
          break;
        case RASCS_Disconnected: 
          lpszStatus = TEXT("Disconnected"); 
          SetCursor (0);
          break;
      }
      
      // Display the status of the RAS connection.
      SetDlgItemText (hDlgWnd, IDC_STATUS, lpszStatus);
      GetConnStatus ();
      return 0;
    }
  }
  return 0;
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
  switch (uMsg)
  {
    case WM_CREATE:
      // Create commandbar and insert the menu.
      hCmdBarWnd = CommandBar_Create (hInst, hWnd, 1);
      CommandBar_InsertMenubar (hCmdBarWnd, hInst, IDR_MAIN_MENU, 0);
      CommandBar_AddAdornments (hCmdBarWnd, 0, 0);

      hMainMenu = CommandBar_GetMenu (hCmdBarWnd, 0);
      EnableMenuItem (hMainMenu, IDM_CONNECTION_STATUS, 
                      MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem (hMainMenu, IDM_CONNECTION_HANGUP, 
                      MF_BYCOMMAND | MF_GRAYED);
      return 0;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDM_FILE_PHONEBOOK:
          DialogBox (hInst, MAKEINTRESOURCE(IDD_PHONEBOOK), hWnd, 
                     PhoneBookDlgProc);
          break;

        case IDM_CONNECTION_CONNECT:
        {
          // Create the Status dialog box.
          hDialingWnd = CreateDialog (hInst, 
                                      MAKEINTRESOURCE(IDD_DIALING),
                                      hWnd, 
                                      DialingDlgProc);

          // Display the Status dialog box and the connection status.
          if (hDialingWnd)
          {
            ShowWindow (hDialingWnd, TRUE);
            GetConnStatus ();
            EnableMenuItem (hMainMenu, IDM_CONNECTION_STATUS, 
                            MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem (hMainMenu, IDM_CONNECTION_HANGUP, 
                            MF_BYCOMMAND | MF_ENABLED);
          }
          break;
        }
       
        case IDM_CONNECTION_STATUS:
          if (hDialingWnd)
            ShowWindow (hDialingWnd, TRUE);
          break;
          
        case IDM_CONNECTION_HANGUP:
          SendMessage (hDialingWnd, WM_COMMAND, 
                       MAKEWPARAM(IDC_HANGUP, 0), 0);
          break;
          
        case IDM_FILE_EXIT:
          CloseRasConnections ();
          DestroyWindow (hWnd);
          break;

        case IDM_HELP_ABOUT:
          MessageBox (hWnd, 
                      TEXT("Window CE RasConn Sample Application\r\n")
                      TEXT("              (c) Microsoft 1999"),
                      TEXT("About RasConn"), 
                      MB_OK);
          break;
      }
      return 0;

    case WM_DESTROY:
      CloseRasConnections ();
      PostQuitMessage (0);
      return 0;
  }
  return DefWindowProc (hWnd, uMsg, wParam, lParam);
}

/***********************************************************************

FUNCTION: 
  InitApplication

PURPOSE: 
  Initializes and registers a windows class.

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

  return RegisterClass (&wc);
}

/***********************************************************************

FUNCTION: 
  InitInstance

PURPOSE: 
  Creates and displays the main window.

***********************************************************************/
BOOL InitInstance (HINSTANCE hInstance, int nCmdShow)
{
  hMainWnd = CreateWindowEx (0, 
                             szAppName,
                             szTitle,
                             WS_CLIPCHILDREN,
                             CW_USEDEFAULT, 0, 
                             CW_USEDEFAULT, 0,
                             NULL, 
                             NULL,
                             hInstance, 
                             NULL);
  if (!hMainWnd)
  {
    return FALSE;
  }

  ShowWindow (hMainWnd, nCmdShow);
  UpdateWindow (hMainWnd);

  return TRUE;
}

/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  Called by the system as the initial entry point for this Windows 
  CE-based application.

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
 
  hInst = hInstance;

  if (!hPrevInstance)
  {
    if (!InitApplication (hInstance))
      return 0;
  }

  if (!InitInstance (hInstance, nCmdShow))
  {
    return 0;
  }
 
  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  }

  return 1;
}
// END OF RASCONN.CPP

