/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE:  
  CeGDI.h

ABSTRACT:  
  This is a header file of the CeGDI Windows CE application.

***********************************************************************/

#ifndef STRICT
#define STRICT
#endif

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
  
HINSTANCE g_hInst;              // Handle to the application instance
HWND g_hwndMain;                // Handle to the application main window
HWND g_hwndEdit;                // Handle to the edit control window
HWND g_hwndCB;                  // Handle to the command bar
  
TCHAR g_szTitle[80];            // Name of the main window
TCHAR g_szClassName[80];        // Name of the main window class

int g_iCBHeight;                // Command bar height

int *g_lpintFontIndexType;      // Type of font
ENUMLOGFONT *g_lpEnumLogFont;   // Pointer to the logical-font data
NEWTEXTMETRIC *g_lpNewTextMetric;  
                                // Pointer to the physical-font data
static COLORREF g_crColor[] = {
                    0x000000FF,0x0000FF00,0x00FF0000,0x0000FFFF,
                    0x00FF00FF,0x00FFFF00,0x00FFFFFF,0x00000080,
                    0x00008000,0x00800000,0x00008080,0x00800080,
                    0x00808000,0x00808080,0x000000FF,0x0000FF00,
                    0x00FF0000,0x0000FFFF,0x00FF00FF,0x00FFFF00};
                                // COLORREF data array that represents
                                // 20 RGB colors
VOID BitmapDemo (HWND);
VOID DisplayLine (HWND);
VOID DisplayText (HWND);
VOID DisplayPalette (HWND);  
VOID DrawRandomObjects (HWND);  
