/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Screen.c

ABSTRACT: 
  This is a C file of the TTY Windows CE sample application. It shows
  the definitions of screen related functions.

***********************************************************************/

#include <windows.h>
#include "tty.h"

/***********************************************************************

  ClearScreen ()

***********************************************************************/
void ClearScreen ()
{
  int index;
  
  memset ((char *)Screen.lpszScreenBuff, 
          TEXT(' '),
          Screen.NumRows * (Screen.NumCols + 1) * sizeof (TCHAR));
  
  for (index = 0; index < Screen.NumRows; index++) 
  {
    Screen.lpszScreenBuff[index * (Screen.NumCols + 1) + Screen.NumCols]
      = TEXT('\0');
  }

  Screen.TopRow = 0;
  Screen.LeftCol = 0;
  Screen.CursorRow = 0;
  Screen.CursorCol = 0;

  InvalidateRect (hTermWnd, NULL, TRUE);
  UpdateWindow (hTermWnd);
}

/***********************************************************************

  FillScreen ()

***********************************************************************/
void FillScreen ()
{
  int index;
  
  memset ((char *)Screen.lpszScreenBuff, 
          TEXT(' '),
          Screen.NumRows * (Screen.NumCols + 1) * sizeof (TCHAR));
  
  for (index = 0; index < Screen.NumRows; index++) 
  {
    memcpy (&(SCREENPOS(index, 0)), 
            TEXT("Test row 00"), 
            11 * sizeof (TCHAR));

    Screen.lpszScreenBuff[index * (Screen.NumCols + 1) + 9] += 
      (index / 10) % 10;

    Screen.lpszScreenBuff[index * (Screen.NumCols + 1) + 10] += 
      index % 10;

    Screen.lpszScreenBuff[index * (Screen.NumCols + 1) + Screen.NumCols]
      = TEXT('\0');
  }

  Screen.TopRow = 0;
  Screen.LeftCol = 0;
  Screen.CursorRow = 0;
  Screen.CursorCol = 0;
  
  InvalidateRect (hTermWnd, NULL, TRUE);
  UpdateWindow (hTermWnd);
}


/***********************************************************************

  PaintScreen (HWND hWnd, HDC  hDC, RECT* lpRect)

***********************************************************************/
void PaintScreen (HWND hWnd, HDC  hDC, RECT* lpRect)
{
  int iRow;
  DWORD dwX, 
        dwY,
        dwStartCol, 
        dwEndCol;
  HFONT hOldFont = NULL;
  
  hOldFont = (HFONT)SelectObject (hDC, hFont);

  SetTextColor (hDC, RGB(0, 0, 0));
  SetBkColor (hDC, RGB(0xff, 0xff, 0xff));

  dwStartCol = (lpRect->left + Screen.CharWidth * Screen.LeftCol) /
                Screen.CharWidth;
  dwEndCol = (lpRect->right + (Screen.CharWidth + 1) * Screen.LeftCol) /
              Screen.CharWidth;

  if (dwEndCol > Screen.NumCols) 
    dwEndCol = Screen.NumCols;
  
  dwX = (dwStartCol - Screen.LeftCol) * Screen.CharWidth;

  for (iRow = Screen.TopRow, dwY = 0; 
       iRow < (Screen.TopRow + Screen.ScreenRows);
       iRow++, dwY += Screen.CharHeight) 
  {
    if ((ROWY(iRow+1) >= lpRect->top) && (ROWY(iRow) <= lpRect->bottom))
    {
      ExtTextOut (hDC, dwX, dwY, 0, 0, &(SCREENPOS(iRow, dwStartCol)),
                  dwEndCol - dwStartCol, NULL);
    }
  }

  if (!hOldFont) 
    SelectObject (hDC, hOldFont);

  if (Screen.bHaveFocus) 
  {
    if (ON_SCREEN(Screen.CursorRow, Screen.CursorCol)) 
    {
      SetCaretPos (COLX(Screen.CursorCol), ROWY(Screen.CursorRow));
      ShowCaret (hTermWnd);
    } 
    else 
      HideCaret (hTermWnd);
  }
}


/***********************************************************************

  InitScreenSettings (HWND hWnd)

***********************************************************************/
BOOL InitScreenSettings (HWND hWnd)
{
  HDC hDC = GetDC (hWnd);
  RECT rect;
  HFONT hOldFont;
  LOGFONT logfont;
  SCROLLINFO ScrollInfo;
  TEXTMETRIC TextMetrics;
  
  if (!Screen.lpszScreenBuff) 
  {
    Screen.NumRows = INIT_SCREEN_ROWS;
    Screen.NumCols = INIT_SCREEN_COLS;

    Screen.lpszScreenBuff = (PTCHAR) LocalAlloc (LPTR,
                Screen.NumRows * (Screen.NumCols + 1) * sizeof (TCHAR));

    if (!Screen.lpszScreenBuff) 
      return FALSE;
    
    memset ((char *)&logfont, 0, sizeof (logfont));

    logfont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    logfont.lfHeight = 13;

    hFont = CreateFontIndirect (&logfont);
  }

  hOldFont = (HFONT)SelectObject (hDC, hFont);

  GetTextMetrics (hDC, &TextMetrics);

  Screen.CharHeight = (USHORT)TextMetrics.tmHeight;
  Screen.CharWidth = (USHORT)TextMetrics.tmMaxCharWidth;

  if (!hOldFont)  
    SelectObject (hDC,(HGDIOBJ)hOldFont);
  
  ReleaseDC (hWnd, hDC);

  Screen.TopRow = 0;
  Screen.LeftCol = 0;
  Screen.CursorRow = 0;
  Screen.CursorCol = 0;

  GetClientRect (hWnd, &rect);

  Screen.ScreenRows = rect.bottom / Screen.CharHeight;
  Screen.ScreenCols = rect.right / Screen.CharWidth;
  
  // Set up the scroll bars.
  ScrollInfo.cbSize = sizeof (SCROLLINFO);
  ScrollInfo.fMask = SIF_RANGE|SIF_POS|SIF_PAGE;
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = Screen.NumRows;
  ScrollInfo.nPage = Screen.ScreenRows;
  ScrollInfo.nPos = 0;
  SetScrollInfo (hWnd, SB_VERT, &ScrollInfo, TRUE);

  ScrollInfo.fMask = SIF_RANGE|SIF_POS|SIF_PAGE;
  ScrollInfo.nMin = 0;
  ScrollInfo.nMax = Screen.NumCols;
  ScrollInfo.nPage = Screen.ScreenCols;
  ScrollInfo.nPos = 0;
  SetScrollInfo (hWnd, SB_HORZ, &ScrollInfo, TRUE);
  
  return TRUE;
}


/***********************************************************************

  ScrollDown (int iRows)

***********************************************************************/
void ScrollDown (int iRows)
{
  DWORD dwOldTop = Screen.TopRow;
  SCROLLINFO ScrollInfo;
  RECT rect;

  // Check some limit conditions.
  if ((iRows < 0) && (-iRows > Screen.TopRow)) 
    Screen.TopRow = 0;
  else 
  {
    // Move the top row.
    Screen.TopRow += iRows;

    if (Screen.TopRow > (Screen.NumRows - Screen.ScreenRows)) 
      Screen.TopRow = Screen.NumRows - Screen.ScreenRows;
  }

  if (Screen.TopRow == dwOldTop) 
    return;

  GetClientRect (hTermWnd, &rect);
  HideCaret (hTermWnd);

  ScrollWindowEx (hTermWnd, 
                  0, 
                  (dwOldTop - Screen.TopRow) * Screen.CharHeight,
                  &rect, &rect,
                  NULL, NULL, SW_INVALIDATE|SW_ERASE);

  ScrollInfo.cbSize = sizeof (SCROLLINFO);
  ScrollInfo.fMask = SIF_POS;
  ScrollInfo.nPos = Screen.TopRow;
  SetScrollInfo(hTermWnd, SB_VERT, &ScrollInfo, TRUE);

  ShowCaret(hTermWnd);
}


/***********************************************************************

  ScrollRight (int iCols)

***********************************************************************/
void ScrollRight (int iCols)
{
  DWORD dwOldLeft = Screen.LeftCol;
  SCROLLINFO ScrollInfo;
  RECT rect;
  
  if ((iCols < 0) && (-iCols > Screen.LeftCol)) 
    Screen.LeftCol = 0;
  else 
  {
    Screen.LeftCol += iCols;

    if (Screen.LeftCol > (Screen.NumCols - Screen.ScreenCols)) 
      Screen.LeftCol = Screen.NumCols - Screen.ScreenCols;
  }

  if (Screen.LeftCol == dwOldLeft) 
    return;

  GetClientRect(hTermWnd, &rect);
  HideCaret(hTermWnd);

  ScrollWindowEx (hTermWnd, 
                  (dwOldLeft - Screen.LeftCol) * Screen.CharWidth,
                  0, &rect, &rect,
                  NULL, NULL, SW_INVALIDATE | SW_ERASE);

  ScrollInfo.cbSize = sizeof (SCROLLINFO);
  ScrollInfo.fMask = SIF_POS;
  ScrollInfo.nPos = Screen.LeftCol;
  SetScrollInfo (hTermWnd, SB_HORZ, &ScrollInfo, TRUE);
  ShowCaret (hTermWnd);
}


/***********************************************************************

  PutCursorOnScreen ()

***********************************************************************/
void PutCursorOnScreen ()
{
  if (ON_SCREEN(Screen.CursorRow, Screen.CursorCol)) 
    return;
  
  if (Screen.CursorRow < Screen.TopRow) 
    ScrollDown (Screen.CursorRow - Screen.TopRow);
  else 
  {
    if (Screen.CursorRow > (Screen.TopRow + Screen.ScreenRows - 1))
      ScrollDown (Screen.CursorRow - 
                  (Screen.TopRow + Screen.ScreenRows - 1));
  }

  if (Screen.CursorCol < Screen.LeftCol) 
    ScrollRight (Screen.CursorCol - Screen.LeftCol);
  else 
  {
    if (Screen.CursorCol > (Screen.LeftCol + Screen.ScreenCols - 1)) 
      ScrollRight (Screen.CursorCol - 
                   (Screen.LeftCol + Screen.ScreenCols - 1));
  }
}


/***********************************************************************

  ProcessChar (BYTE byInChar)

***********************************************************************/
void ProcessChar (BYTE byInChar)
{
  RECT rect;
  int index;
  BOOL bOnScreen = ON_SCREEN(Screen.CursorRow, Screen.CursorCol);

  if (ON_SCREEN(Screen.CursorRow, Screen.CursorCol)) 
  {
    rect.top = ROWY(Screen.CursorRow);
    rect.bottom = ROWY(Screen.CursorRow + 1);
    rect.left = COLX((Screen.CursorCol) ? (Screen.CursorCol - 1) : 0);
    rect.right = COLX(Screen.CursorCol + 1);
    InvalidateRect (hTermWnd, &rect, TRUE);
  }
    
  switch (byInChar) 
  {
    case 0x08 :
      if (Screen.CursorCol) 
      {
        Screen.CursorCol--;
        CURPOS = TEXT(' ');
      }
      break;

    case 0x0d :
      Screen.CursorCol = 0;
      break;

    case 0x0a :
      Screen.CursorRow++;
      break;

    default :
      CURPOS = byInChar;
      Screen.CursorCol++;
      break;
  }

  if (Screen.CursorCol == Screen.NumCols) 
  {
    Screen.CursorRow++;
    Screen.CursorCol = 0;
  }

  if (Screen.CursorRow == Screen.NumRows) 
  {
    // Shift data up.
    for (index = 0; index < (Screen.NumRows - 1); index++) 
    {
      memcpy (&(SCREENPOS(index, 0)),
              &(SCREENPOS(index + 1, 0)),
              sizeof (TCHAR) * (Screen.NumCols));
    }    

    memset (&(SCREENPOS(index, 0)), TEXT(' '), Screen.NumCols);
    Screen.CursorRow--;
        
    InvalidateRect (hTermWnd, NULL, TRUE);
  }

  if (bOnScreen && !ON_SCREEN(Screen.CursorRow, Screen.CursorCol)) 
    PutCursorOnScreen ();
}
