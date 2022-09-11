/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeFile.c

ABSTRACT: 
  This is a C file of the CePad Windows CE sample application. It 
  contains functions for opening and saving files.
 
***********************************************************************/

#include "CePad.h"
#include <commdlg.h>

static HANDLE hFile;      // File handle for opening or saving files
static OPENFILENAME ofn;  // Contains information that GetOpenFileName 
                          // and GetSaveFileName use to initialize 
                          // an Open or Save As common dialog box

/***********************************************************************

FUNCTION: 
  CPFileInitialize

PURPOSE: 
  Initializes the OPENFILENAME structure.

***********************************************************************/
void CPFileInitialize (HWND hwnd)
{
  static TCHAR szFilter[] = TEXT("Text Files (*.TXT)\0*.txt\0")  \
                            TEXT("ASCII Files (*.ASC)\0*.asc\0") \
                            TEXT("All Files (*.*)\0*.*\0\0");

  ofn.lStructSize       = sizeof (OPENFILENAME);
  ofn.hwndOwner         = hwnd;
  ofn.hInstance         = NULL;
  ofn.lpstrFilter       = szFilter;
  ofn.lpstrCustomFilter = NULL;
  ofn.nMaxCustFilter    = 0;
  ofn.nFilterIndex      = 0;
  ofn.lpstrFile         = NULL;          
  ofn.nMaxFile          = _MAX_PATH;
  ofn.lpstrFileTitle    = NULL;          
  ofn.nMaxFileTitle     = _MAX_FNAME + _MAX_EXT;
  ofn.lpstrInitialDir   = NULL;
  ofn.lpstrTitle        = NULL;
  ofn.Flags             = 0;             
  ofn.nFileOffset       = 0;
  ofn.nFileExtension    = 0;
  ofn.lpstrDefExt       = TEXT("txt");
  ofn.lCustData         = 0L;
  ofn.lpfnHook          = NULL;
  ofn.lpTemplateName    = NULL;
}

/***********************************************************************

FUNCTION: 
  CPFileOpenDlg

PURPOSE: 
  Creates a system-defined dialog box that enables a user to select a 
  file to open. 

***********************************************************************/
BOOL CPFileOpenDlg (HWND hwnd, LPWSTR lpwstrFile)
{
  ofn.hwndOwner         = hwnd;
  ofn.lpstrFile         = lpwstrFile;
  ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT;
  return GetOpenFileName (&ofn);
}

/***********************************************************************

FUNCTION: 
  CPFileSaveDlg

PURPOSE: 
  Creates a system-defined dialog box that enables a user to select a 
  file to save. 

***********************************************************************/
BOOL CPFileSaveDlg (HWND hwnd, LPWSTR lpwstrFile)
{
  ofn.hwndOwner         = hwnd;
  ofn.lpstrFile         = lpwstrFile;
  ofn.Flags             = OFN_OVERWRITEPROMPT;
  return GetSaveFileName (&ofn);
}


/***********************************************************************

FUNCTION: 
  CPAskSaveFile

PURPOSE: 
  Ask a user if he or she wants to save the file. Saves the file if the 
  answer is yes.

***********************************************************************/
int CPAskSaveFile (HWND hwnd)
{
  int iReturn;              
  TCHAR szBuffer[_MAX_PATH + 100];
  
  wsprintf (szBuffer, TEXT("Save current changes in %s?"), 
            g_szFileName);

  iReturn = MessageBox (hwnd, szBuffer, g_szClassName, 
                        MB_YESNOCANCEL | MB_ICONQUESTION);
  if (iReturn == IDYES)
  {
    if (!SendMessage (hwnd, WM_COMMAND, IDM_SAVE, 0L))
       iReturn = IDCANCEL;
  }
  return iReturn;
}


/***********************************************************************

FUNCTION: 
  CPFileRead

PURPOSE: 
  Reads a file and displays the content in the edit control.

***********************************************************************/
BOOL CPFileRead (HWND hwndEdit, LPWSTR lpwstrFileName)
{
  DWORD dwBytesRead,    // Number of bytes read
        dwFileSize,     // Low-order word for file size 
        dwFileSizeHigh; // High-order word for file size

  LPWSTR lpwstrBuffer;  // Buffer that receives data read from the file
  BOOL bReturn = FALSE; // Return value of the function
  
  // Open the existing file. 
  if ((hFile = CreateFile (
                  lpwstrFileName,         // File name
                  GENERIC_READ,           // Open the file for reading
                  0,                      // Do not share
                  NULL,                   // No security attributes
                  OPEN_EXISTING,          // Open the existing file only
                  FILE_ATTRIBUTE_NORMAL,  // Normal file attributes
                  NULL)) == INVALID_HANDLE_VALUE) 
                                          // No template file
  {
    goto exit;
  }

  dwFileSize = GetFileSize (hFile, &dwFileSizeHigh) + 2;

  // If the file size exceeded the maximum size allowed, go to exit.
  if (dwFileSizeHigh > 0 || dwFileSize > MAXFILESIZE * 2)
    goto exit;

  // If there is not enough memory, go to exit.
  if (! (lpwstrBuffer = (LPWSTR) LocalAlloc (LPTR, dwFileSize)))
    goto exit;

  if (ReadFile (hFile, lpwstrBuffer, dwFileSize, &dwBytesRead, NULL)) 
  {
    if (SetWindowText (hwndEdit, lpwstrBuffer))
      bReturn = TRUE;
  }

  // Set the cursor to the end of the file.
  SendMessage (hwndEdit, EM_SETSEL, (WPARAM) (dwFileSize/2), 
               (LPARAM) (dwFileSize/2));

  // Free the memory.
  LocalFree (lpwstrBuffer);

exit:

  CloseHandle (hFile);
  return bReturn;
}

/***********************************************************************

FUNCTION: 
  CPFileWrite

PURPOSE: 
  Writes the contents of the edit control to a file.

***********************************************************************/
BOOL CPFileWrite (HWND hwndEdit, LPWSTR lpwstrFileName)
{
  DWORD dwFileSize,     // Low-order word for file size
        dwBytesWritten; // Number of bytes written
  LPWSTR lpwstrBuffer;  // Buffer receives data from the edit control
  BOOL bReturn = FALSE; // Return value of the function
  
  // Open the existing file. 
  if ((hFile = CreateFile (
                  lpwstrFileName,         // File name
                  GENERIC_WRITE,          // Open the file for writing
                  0,                      // Do not share 
                  NULL,                   // No security 
                  CREATE_ALWAYS,          // Always create a new file
                  FILE_ATTRIBUTE_NORMAL,  // Normal file 
                  NULL)) == INVALID_HANDLE_VALUE) 
                                          // No template file
  {
    goto exit;
  }

  dwFileSize = 2 * GetWindowTextLength (hwndEdit);

  // If the file size exceeded the maximum size allowed, go to exit.
  if (dwFileSize > MAXFILESIZE)
    goto exit;

  // If there is not enough memory, go to exit.
  if (! (lpwstrBuffer = (LPWSTR) LocalAlloc (LPTR, dwFileSize + 2)))
    goto exit;

  if (GetWindowText (hwndEdit, lpwstrBuffer, (int)dwFileSize))
  {
    if (WriteFile (hFile, lpwstrBuffer, dwFileSize, &dwBytesWritten, 
                   NULL))
      bReturn = TRUE; 
  }

  // Free the memory.
  LocalFree (lpwstrBuffer);

exit:

  CloseHandle (hFile);
  return bReturn;
}
// END OF CEFILE.C