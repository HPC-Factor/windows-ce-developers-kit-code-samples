/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Fonts.c

ABSTRACT: 
  This is a C file of the CeGDI Windows CE application. It contains 
  several functions for enumerating fonts, selecting a font, and
  displaying some text by using the selected font.

***********************************************************************/

#include "CeGDI.h"
#include <wingdi.h>

int iNumOfFonts = 0,    // Total number of fonts
    iCurrItem = -1;     // Index of currently added or selected font
LOGFONT CurrLogFont;    // Currently selected font

/***********************************************************************

FUNCTION: 
  FontListDlgProc

PURPOSE: 
  Processes messages sent to the List of Fonts dialog window.

***********************************************************************/
BOOL CALLBACK FontListDlgProc (HWND hwndDlg, UINT uMsg, WPARAM wParam, 
                               LPARAM lParam)
{
  int index = 1,
      iFontIndex;           // Font list index
  TCHAR szFontName[80],     // For the currently enumerated font name
        szFontNamePrev[80]; // For the previously enumerated font name
  HWND hwndFontListCtrl;    // Handle to the font list box control
  
  // Get the window handle of the font list box control in the dialog
  // box.
  hwndFontListCtrl = GetDlgItem (hwndDlg, IDC_FONTLIST);
            
  switch (uMsg)
  {
    case WM_INITDIALOG:
      for (iFontIndex = 0; iFontIndex < iNumOfFonts; ++iFontIndex)
      {
        wsprintf (szFontName, TEXT("%s"), 
                  g_lpEnumLogFont[iFontIndex].elfFullName); 
        
        if (wcscmp (szFontName, szFontNamePrev) == 0)
        {
          index += 1;
          wsprintf (szFontName, TEXT("%s ----- %d"), szFontName, index);
        }
        else
          index = 1;

        wsprintf (szFontNamePrev, TEXT("%s"), 
                  g_lpEnumLogFont[iFontIndex].elfFullName); 
        
        // Add the font name to the list control.
        iCurrItem = SendMessage (hwndFontListCtrl, LB_ADDSTRING, 0, 
                                 (LPARAM)(LPCTSTR) szFontName);

        // Set a 32-bit value, (LPARAM) iFontIndex, associated with the
        // newly added item in the list control. 
        SendMessage (hwndFontListCtrl, LB_SETITEMDATA, iCurrItem, 
                     iFontIndex);
      }

      // Select the first font name in the list control.
      SendMessage (hwndFontListCtrl, LB_SETCURSEL, 0, 0);

      // Move the dialog box to the center of the main window.
      {
        RECT rectMain,
             rectDlg;
        LONG lMainWidth,
             lDlgWidth,
             lMainHeight,
             lDlgHeight;

        GetWindowRect (g_hwndMain, &rectMain);
        GetWindowRect (hwndDlg, &rectDlg);

        rectMain.top += g_iCBHeight;
        lMainWidth = rectMain.right - rectMain.left;
        lDlgWidth = rectDlg.right - rectDlg.left;
        lMainHeight = rectMain.bottom - rectMain.top;
        lDlgHeight = rectDlg.bottom - rectDlg.top;

        MoveWindow (hwndDlg,
                    rectMain.left + (lMainWidth - lDlgWidth) / 2,
                    rectMain.top + (lMainHeight - lDlgHeight) / 2,
                    lDlgWidth,
                    lDlgHeight,
                    FALSE);
      }

      return TRUE;  

    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case IDOK:
          // Retrieve the index of the currently selected font.
          if ((iCurrItem = SendMessage (hwndFontListCtrl, LB_GETCURSEL, 
                                        0, 0)) != LB_ERR)
          {
            iFontIndex = SendMessage (hwndFontListCtrl, LB_GETITEMDATA, 
                                      iCurrItem, 0);

            CurrLogFont = g_lpEnumLogFont[iFontIndex].elfLogFont;

            EndDialog (hwndDlg, 0);
            return TRUE;
          }

        case IDCANCEL:
          iCurrItem = -1;
          EndDialog (hwndDlg, 0);
          return TRUE;
      }
      break;
  }
  return FALSE;
}

/***********************************************************************

FUNCTION: 
  EnumFontFamProc

PURPOSE: 
  The callback function of the EnumFontFamilies function.

***********************************************************************/
int CALLBACK EnumFontFamProc (
          ENUMLOGFONT FAR *lpelf,   // Pointer to logical-font data
          NEWTEXTMETRIC FAR *lpntm, // Pointer to physical-font data
          int FontType,             // Type of font
          LPARAM lParam)            // Application-defined data address
{
  iNumOfFonts += 1;

  // Change the size of the local memory objects according to the total
  // number of fonts.
  if (iNumOfFonts <= 1)
  {
    g_lpNewTextMetric = (NEWTEXTMETRIC *) LocalAlloc (LHND, 
                                            sizeof (NEWTEXTMETRIC));

    g_lpEnumLogFont = (ENUMLOGFONT *) LocalAlloc (LHND, 
                                            sizeof (ENUMLOGFONT));

    g_lpintFontIndexType = (int *) LocalAlloc (LHND, sizeof (int));
  }
  else
  {
    g_lpNewTextMetric = (NEWTEXTMETRIC *) LocalReAlloc (
                                  g_lpNewTextMetric, 
                                  iNumOfFonts * sizeof (NEWTEXTMETRIC), 
                                  LMEM_MOVEABLE | LMEM_ZEROINIT);

    g_lpEnumLogFont = (ENUMLOGFONT *) LocalReAlloc (
                                  g_lpEnumLogFont, 
                                  iNumOfFonts * sizeof (ENUMLOGFONT),
                                  LMEM_MOVEABLE | LMEM_ZEROINIT);

    g_lpintFontIndexType = (int *) LocalReAlloc (
                                  g_lpintFontIndexType, 
                                  iNumOfFonts * sizeof (int),
                                  LMEM_MOVEABLE | LMEM_ZEROINIT);
  }

  // Save the newly enumerated font information in the global variables.
  g_lpNewTextMetric[iNumOfFonts - 1] = *lpntm;      
  g_lpEnumLogFont[iNumOfFonts - 1] = *lpelf;      
  g_lpintFontIndexType[iNumOfFonts - 1] = FontType;  

  return 1;
}

/***********************************************************************

FUNCTION: 
  EnumFontFamNameProc

PURPOSE: 
  The callback function of the EnumFontFamilies function.

***********************************************************************/
int CALLBACK EnumFontFamNameProc (
          ENUMLOGFONT FAR *lpelf,   // Pointer to logical-font data
          NEWTEXTMETRIC FAR *lpntm, // Pointer to physical-font data
          int FontType,             // Type of font
          LPARAM lParam)            // Application-defined data address
{
  HDC hDC;            // Handle to the display device context 
  HWND hwnd;          // Handle to the current window
  LOGFONT LogFont;    // A LOGFONT structure that defines the attributes 
                      // of a font 

  LogFont = lpelf->elfLogFont;
  
  hwnd = GetFocus ();

  if (!(hDC = GetDC (hwnd)))
    return 1;

  EnumFontFamilies (hDC, 
                    LogFont.lfFaceName, 
                    (FONTENUMPROC) EnumFontFamProc,
                    (LPARAM) 0);
  
  ReleaseDC (hwnd, hDC);  
  return 1;
}

/***********************************************************************

FUNCTION: 
  DisplayText

PURPOSE: 
  Selects a font and displays some text by using the selected font.

***********************************************************************/
void DisplayText (HWND hwnd)
{
  HDC hDC;              // Handle to the display device context         
  HFONT hFont;          // Handle to the selected font
  RECT rect;            // Contains the coordinates of the window's 
                        // client area
  TCHAR szString[200];  // Contains the text to be displayed

  // Retrieve the handle to the display device context.
  if (!(hDC = GetDC (hwnd)))
    return;

  // Enumerate the fonts if there are no enumerated fonts.
  if (iNumOfFonts == 0)
  {
    EnumFontFamilies (hDC, NULL, (FONTENUMPROC) EnumFontFamNameProc, 
                      (LPARAM) 0); 
  }
  
  DialogBox (g_hInst, MAKEINTRESOURCE(IDD_FONTLIST), g_hwndMain, 
             FontListDlgProc);
  
  // If it failed to select a font, then return.
  if (iCurrItem == -1)
    return;

  hFont = SelectObject (hDC, CreateFontIndirect (&CurrLogFont)); 
  
  wsprintf (szString, TEXT("%s"), TEXT("AaBbCcDdEe 012345"));

  SetTextColor (hDC, g_crColor[rand() % 20]);
  
  // Retrieve the coordinates of a window's client area. 
  GetClientRect (hwnd, &rect);

  rect.top += CommandBar_Height (g_hwndCB);

  DrawText (hDC, szString, wcslen (szString), &rect, 
            DT_CENTER | DT_TOP | DT_SINGLELINE); 

  ReleaseDC (hwnd, hDC);

  return;    
}
// END OF FONTS.C


