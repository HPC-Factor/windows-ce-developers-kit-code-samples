
/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Tty.h

ABSTRACT: 
  This is a header file of the TTY Windows CE sample application.

************************************************************************/

#ifndef _TTY_H_
#define _TTY_H_

// Structure TERM_SCREEN definition
typedef struct _TERM_SCREEN {
  USHORT  NumRows;          // Total number of rows in the buffer
  USHORT  NumCols;          // Total number of columns in the buffer
  USHORT  TopRow;           // Top row visible on the screen
  USHORT  LeftCol;          // Leftmost column visible on the screen    
  USHORT  ScreenRows;       // Number of rows visible on the screen
  USHORT  ScreenCols;       // Number of columns visible on the screen
  USHORT  CursorRow;        // Row of the cursor
  USHORT  CursorCol;        // Column of the cursor
  USHORT  CharHeight;       // Character width
  USHORT  CharWidth;        // Character height
  BOOL    bHaveFocus;       // Indication if it is focused on the screen
  LPTSTR  lpszScreenBuff;   // Screen buffer
} TERM_SCREEN, *PTERM_SCREEN;

#define ROWY(row)          (((row) - Screen.TopRow) * Screen.CharHeight)
#define COLX(col)          (((col) - Screen.LeftCol) * Screen.CharWidth)
#define ON_SCREEN(row,col) (((row) >= Screen.TopRow) && \
          ((row) < Screen.TopRow + Screen.ScreenRows) && \
          ((col) >= Screen.LeftCol) && \
          ((col) < Screen.LeftCol + Screen.ScreenCols) ? TRUE : FALSE)
#define SCREENPOS(r,c)     (Screen.lpszScreenBuff[((r) * \
          (Screen.NumCols + 1)) + (c)])
#define CURPOS             (Screen.lpszScreenBuff[(Screen.CursorRow * \
          (Screen.NumCols + 1)) + Screen.CursorCol])

// Initial screen values
#define INIT_SCREEN_COLS    80  // Initial screen columns
#define INIT_SCREEN_ROWS    25  // Initial screen rows

// Global Variables
extern TCHAR szTitle[];         // Application window name
extern HWND hMainWnd;           // Main window handle
extern HWND hTermWnd;           // Terminal window handle
extern HFONT hFont;             // Handle to the font to use
extern HANDLE hPort;            // Serial port handle
extern HANDLE hReadThread;      // Handle to the read thread
extern LPTSTR lpszDevName;      // Communication port name
extern TERM_SCREEN Screen;      // Global screen structure

// Tty.c
VOID SetLightIndicators (DWORD);

// Port.c
BOOL PortInitialize (LPTSTR);
BOOL PortClose (HANDLE);
void PortWrite (BYTE);
DWORD PortReadThread (LPVOID);

// Screen.c
BOOL InitScreenSettings (HWND);
void FillScreen ();
void ClearScreen ();
void PaintScreen (HWND, HDC, RECT*);
void InvalidateChar (HWND);
void ProcessChar (BYTE);
void ScrollRight (int);
void ScrollDown (int);

#endif
