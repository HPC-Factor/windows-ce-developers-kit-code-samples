/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE:  
  CePad.h

ABSTRACT:  
  This is a header file of the CePad Windows CE sample application.

***********************************************************************/

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>
#include "resource.h"
  
#define EDITID 1                // Edit menu identifier
#define MAX_STR_LEN 256         // Maximum string length in the edit 
                                // control
#define MAXFILESIZE 32000       // Maximum file size in bytes

HINSTANCE g_hInst;              // Handle to the application instance
HWND g_hwndMain;                // Handle to the application main window
HWND g_hwndEdit;                // Handle to the edit control window
HWND g_hwndCB;                  // Handle to the command bar
  
TCHAR g_szTitle[80];            // Main window name 
TCHAR g_szClassName[80];        // Main window class name
TCHAR g_szCaption[_MAX_PATH];   // Main window caption 
TCHAR g_szFileName[_MAX_PATH];  // Name and path of the current file 
TCHAR g_szFindWhat[MAX_STR_LEN];// String to be searched
TCHAR g_szReplaceWith[MAX_STR_LEN];  
                                // String to replace with
BOOLEAN g_bNeedSave;            // Indicates if the text in the edit 
                                // control need to be saved
int CPAskSaveFile (HWND);              
void CPFileInitialize (HWND);
BOOL CPFileRead (HWND, LPWSTR);
BOOL CPFileWrite (HWND, LPWSTR);
BOOL CPFileOpenDlg (HWND, LPWSTR);
BOOL CPFileSaveDlg (HWND, LPWSTR);

BOOL CPFindText (HWND, int, LPWSTR);
BOOL CPReplaceText (HWND, int, LPWSTR, LPWSTR);
BOOL CALLBACK FindDialogProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ReplaceDialogProc (HWND, UINT, WPARAM, LPARAM);
