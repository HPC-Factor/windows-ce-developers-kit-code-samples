/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeSrch.c

ABSTRACT: 
  This is a C file of the CePad Windows CE sample application. It 
  contains functions for searching and replacing strings.
 
***********************************************************************/

#include "CePad.h"

#define MAX_STRING_LEN 256              // Maximum length of the string
                                        // to be searched
static char szFindText[MAX_STRING_LEN]; // String to be searched                    
static char szReplText[MAX_STRING_LEN]; // String used to replace the 
                                        // searched string

/***********************************************************************

FUNCTION: 
  CPFindText

PURPOSE: 
  Finds a string in a edit control and highlights the string found.

***********************************************************************/
BOOL CPFindText (HWND hwndEdit, int iSearchOffset, LPWSTR lpstrFindWhat)
{
  int iLength,      // Length of the text in the edit control
      iPosition;    // Current cursor position in the edit control text
  LPWSTR lpwstrDoc, // Buffer for the edit control text
         lpwstrPos; // Position of the string found in the text

  iLength = GetWindowTextLength (hwndEdit);

  // If there is not enough memory, return FALSE.
  if (!(lpwstrDoc = (LPWSTR) LocalAlloc (LPTR, 2 * iLength + 2)))
    return FALSE;  

  // Copy the text of the edit control to the lpwstrDoc buffer.
  GetWindowText (hwndEdit, lpwstrDoc, 2 * iLength + 2);

  // Search the document to find the string.
  lpwstrPos = wcsstr (lpwstrDoc + iSearchOffset, lpstrFindWhat);

  // Free the local memory object. 
  LocalFree (lpwstrDoc);

  // Return FALSE if the string cannot be found.
  if (lpwstrPos == NULL)
    return FALSE;

  // Find the starting character position and ending character position.
  iPosition = lpwstrPos - lpwstrDoc;
  iSearchOffset = iPosition + wcslen (lpstrFindWhat);

  // Select the found text.
  SendMessage (hwndEdit, EM_SETSEL, iPosition, iSearchOffset);
  SendMessage (hwndEdit, EM_SCROLLCARET, 0, 0);

  return TRUE;
}


/***********************************************************************

FUNCTION: 
  CPReplaceText

PURPOSE: 
  Replaces a string found in a edit control.

***********************************************************************/
BOOL CPReplaceText (HWND hwndEdit, int iSearchOffset, 
                    LPWSTR lpstrReplaceWith, LPWSTR lpstrFindWhat)
{
  // Find the text. If it is not found, return FALSE.
  if (!CPFindText (hwndEdit, iSearchOffset, lpstrFindWhat))
    return FALSE;

  // Replace the selection with a new string.
  SendMessage (hwndEdit, EM_REPLACESEL, 0, (LPARAM) lpstrReplaceWith);

  return TRUE;
}

/***********************************************************************

FUNCTION: 
  FindDialogProc

PURPOSE: 
  Processes messages sent to the Find dialog box window.

***********************************************************************/
BOOL CALLBACK FindDialogProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, 
                              LPARAM lParam)
{
  int iSearchOffset;

  switch(uMsg)
  {
    case WM_INITDIALOG:
      SetDlgItemText (hwndDlg, IDC_FINDWHAT, g_szFindWhat); 
      return TRUE;  

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BTNFINDNEXT:
          GetDlgItemText (hwndDlg, IDC_FINDWHAT, g_szFindWhat, 
                          MAX_STRING_LEN);

          // If the string to be searched is empty, return FALSE. 
          if (wcslen (g_szFindWhat) == 0)
            return FALSE;

          SendMessage (g_hwndEdit, EM_GETSEL, (WPARAM) NULL, 
                       (LPARAM) &iSearchOffset);

          if (!CPFindText (g_hwndEdit, iSearchOffset, g_szFindWhat))
          {
            MessageBox (g_hwndMain, TEXT("Text not found!"), 
                        g_szClassName, MB_OK | MB_ICONERROR);
            return FALSE;
          }
          return TRUE;

        case IDCANCEL:
          EndDialog(hwndDlg, 0);
          return TRUE;
      }
      break;
  }
  return FALSE;
}


/***********************************************************************

FUNCTION: 
  ReplaceDialogProc

PURPOSE: 
  Processes messages sent to the Replace dialog box window.

***********************************************************************/
BOOL CALLBACK ReplaceDialogProc (
                            HWND hwndDlg,   // Handle to the dialog box
                            UINT uMsg,      // Message
                            WPARAM wParam,  // First message parameter
                            LPARAM lParam)  // Second message parameter
{
  int iSearchOffset;

  switch(uMsg)
  {
    case WM_INITDIALOG:
      SetDlgItemText (hwndDlg, IDC_FINDWHAT, g_szFindWhat); 
      SetDlgItemText (hwndDlg, IDC_REPLACE, g_szReplaceWith); 
      return TRUE;  

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDC_BTNFINDNEXT:
          GetDlgItemText (hwndDlg, IDC_FINDWHAT,g_szFindWhat, 
                          MAX_STRING_LEN); 

          // If the string to be searched is empty, return FALSE.
          if (wcslen (g_szFindWhat) == 0)
            return FALSE;

          SendMessage (g_hwndEdit, EM_GETSEL, (WPARAM) NULL, 
                       (LPARAM) &iSearchOffset);

          if (!CPFindText (g_hwndEdit, iSearchOffset, g_szFindWhat))
          {
            MessageBox (g_hwndMain, TEXT("Text not found!"), 
                        g_szClassName, MB_OK | MB_ICONERROR);
            return FALSE;
          }

          return TRUE;

        case IDC_BTNREPLACE:
          SendMessage (hwndDlg, WM_COMMAND, IDC_BTNFINDNEXT, 0L);
          GetDlgItemText (hwndDlg, IDC_REPLACE, g_szReplaceWith, 
                          MAX_STRING_LEN); 
          SendMessage (g_hwndEdit, EM_REPLACESEL, 0, 
                       (LPARAM) g_szReplaceWith);
          return TRUE;

        case IDC_BTNREPLACEALL:

          GetDlgItemText (hwndDlg, IDC_FINDWHAT, g_szFindWhat, 
                          MAX_STRING_LEN); 

          GetDlgItemText (hwndDlg, IDC_REPLACE, g_szReplaceWith, 
                          MAX_STRING_LEN); 

          // If the string to be searched is empty, break. 
          if (wcslen (g_szFindWhat) == 0)
            return FALSE;

          iSearchOffset = 0;

          while (CPFindText (g_hwndEdit, iSearchOffset, g_szFindWhat))
          {
            SendMessage (g_hwndEdit, EM_GETSEL, (WPARAM) NULL, 
                         (LPARAM) &iSearchOffset);

            SendMessage (g_hwndEdit, EM_REPLACESEL, 0, 
                         (LPARAM) g_szReplaceWith);
          }
          return TRUE;

        case IDCANCEL:
          EndDialog(hwndDlg, 0);
          return TRUE;
      }
      break;
  }
  return FALSE;
}
// END OF CESRCH.C
