/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  DrawObj.c

ABSTRACT: 
  This is a C file of the CeGDI Windows CE application. It contains
  several functions that show the GDI features such as palette, bitmap, 
  pen, brush, and line.

***********************************************************************/

#include "CeGDI.h"
#include <math.h>

#define NUMPT  200   // Number of points in the sine wave polyline
#define PALETTEINDEX(i) ((COLORREF) (0x01000000 | (DWORD) (WORD) (i))) 
                     // Macro that accepts an index to a palette entry
                     // and returns a palette-entry specifier consisting
                     // of a 32-bit COLORREF value that specifies the 
                     // color associated with the index
int iPalSize = 0;    // Number of entries in the logical color palette

/***********************************************************************

FUNCTION: 
  DrawRandomObjects

PURPOSE: 
  Draws random objects in the context device.

***********************************************************************/
VOID DrawRandomObjects (HWND hwnd)
{
  HDC hDC;                  // Handle to the display device context 
  RECT rect;                // RECT structure
  POINT pt[4];              // Four dimensional POINT structure array
  HBRUSH hBrush,            // Handle to the new brush object 
         hOldBrush;         // Handle to the old brush object 
  TCHAR szDebug[80];        // Debuging message string

  int x1, y1, x2, y2, x3, y3, x4, y4, 
                            // Coordinates of four points
      iRed, iGreen, iBlue,  // Indicate the Red, Green, Blue component 
                            // color of the brush
      iObject;              // Integer that indicates the type of 
                            // objects
  // Retrieve the handle to the display device context.
  if (!(hDC = GetDC (hwnd)))
    return;

  // Retrieve the coordinates of a window's client area. 
  GetClientRect (hwnd, &rect);

  // Take the command bar height in consideration.
  rect.bottom -= g_iCBHeight;

  // Generate three random numbers.
  iRed = rand() % 255;
  iGreen = rand() % 255;
  iBlue = rand() % 255;

  // Create a solid brush object and selects it into the device context.
  hBrush = CreateSolidBrush (RGB(iRed, iGreen, iBlue));

  if (hOldBrush = SelectObject (hDC, hBrush))
  {
    // Randomly generate four points.
    x1 = rand() % rect.right;
    y1 = rand() % rect.bottom + g_iCBHeight;
    x2 = rand() % rect.right;
    y2 = rand() % rect.bottom + g_iCBHeight;
    x3 = rand() % rect.right;
    y3 = rand() % rect.bottom + g_iCBHeight;
    x4 = rand() % rect.right;
    y4 = rand() % rect.bottom + g_iCBHeight;

    // Randomly generate an integer that indicates the type of objects.
    iObject = rand() % 4;

    switch (iObject)
    {
      case 0:
        wsprintf (szDebug, TEXT("Rectangle(%d ,%d, %d, %d)\n"), 
                  x1, y1, x2, y2);

        // Draw a rectangle.
        Rectangle (hDC, x1, y1, x2, y2);

        break;

      case 1:
        wsprintf (szDebug, TEXT("Ellipse(%d, %d, %d, %d)\n"), 
                  x1, y1, x2, y2);

        // Draw an ellipse.
        Ellipse (hDC, x1, y1, x2, y2);

        break;

      case 2:
        wsprintf (szDebug, TEXT("RoundRect (%d, %d, %d, %d, %d, %d)\n"),
                  x1, y1, x2, y2, x3, y3);

        // Draw a rectangle with rounded corners. 
        RoundRect (hDC, x1, y1, x2, y2, x3, y3);

        break;

      case 3:
        pt[0].x = x1;
        pt[0].y = y1;
        pt[1].x = x2;
        pt[1].y = y2;
        pt[2].x = x3;
        pt[2].y = y3;
        pt[3].x = x4;
        pt[3].y = y4;

        wsprintf (szDebug, 
                  TEXT("Chord(%d, %d, %d, %d, %d, %d, %d, %d)\n"),
                  x1, y1, x2, y2, x3, y3, x4, y4);

        // Draw a polygon.
        Polygon(hDC, pt, 4);

        break;

      default:
        break;
    }

    // Select the old brush into the device context.
    SelectObject (hDC, hOldBrush);

    // Delete the brush object.
    DeleteObject (hBrush);
  }

  ReleaseDC (hwnd, hDC);
  return;
}

/***********************************************************************

FUNCTION: 
  CreateScalePalette

PURPOSE: 
  Creates a palette that represents the scale values of an specified
  RGB color. This function can also create a gray scale palette.

***********************************************************************/
HPALETTE CreateScalePalette (HDC hDC, int iColor)
{
  HPALETTE hPalette = NULL;   // Handle of the palette to be created       
  LPLOGPALETTE lpMem = NULL;  // Buffer for the LOGPALETTE structure 
                              // which defines the palette
  int index,                  // Integer index
      iReserved,              // Number of reserved entries in the 
                              // system palette
      iRasterCaps;            // Raster capabilities of the display 
                              // device context
  // Retrieve the raster capabilities of the display device context.
  // Check if it is capble of specifing a palette-based device, then 
  // determine the number of entries in the logical color palette. 
  
  iRasterCaps = GetDeviceCaps (hDC, RASTERCAPS); 
  iRasterCaps = (iRasterCaps & RC_PALETTE) ? TRUE : FALSE;  

  if (iRasterCaps) 
  {
    iReserved = GetDeviceCaps (hDC, NUMRESERVED);
    iPalSize = GetDeviceCaps (hDC, SIZEPALETTE) - iReserved;
  }
  else 
    iPalSize = GetDeviceCaps (hDC, NUMCOLORS); 

  // If there can not be any entries in the logical color palette, exit.
  if (iPalSize <= 0)
  {
    MessageBox (g_hwndMain, 
                TEXT("Palette can not be created, there can not be ")
                TEXT("any entries in it."),
                TEXT("Info"), 
                MB_OK);
    goto exit;
  }

  // Allocate a buffer for the LOGPALETTE structure.
  if (!(lpMem = (LOGPALETTE *) LocalAlloc (LMEM_FIXED, 
                sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * iPalSize)))
    goto exit;
            
  lpMem->palNumEntries = (WORD) iPalSize;
  lpMem->palVersion = (WORD) 0x0300;

  switch(iColor)
  {
    case 0:            // Red color component only
      for (index = 0; index < iPalSize; index++)
      {
        lpMem->palPalEntry[index].peRed   = (BYTE) index;
        lpMem->palPalEntry[index].peGreen = 0;
        lpMem->palPalEntry[index].peBlue  = 0;
        lpMem->palPalEntry[index].peFlags = 0;
      }
      break;

    case 1:            // Green color component only
      for (index = 0; index < iPalSize; index++)
      {
        lpMem->palPalEntry[index].peRed   = 0;
        lpMem->palPalEntry[index].peGreen = (BYTE) index;
        lpMem->palPalEntry[index].peBlue  = 0;
        lpMem->palPalEntry[index].peFlags = 0;
      }
      break;

    case 2:            // Blue color component only
      for (index = 0; index < iPalSize; index++)
      {
        lpMem->palPalEntry[index].peRed   = 0;
        lpMem->palPalEntry[index].peGreen = 0;
        lpMem->palPalEntry[index].peBlue  = (BYTE) index;
        lpMem->palPalEntry[index].peFlags = 0;
      }
      break;

    case 3:            // Gray scale palette
    default:  
      for (index = 0; index < iPalSize; index++)
      {
        lpMem->palPalEntry[index].peRed   = (BYTE) index;
        lpMem->palPalEntry[index].peGreen = (BYTE) index;
        lpMem->palPalEntry[index].peBlue  = (BYTE) index;
        lpMem->palPalEntry[index].peFlags = 0;
      }
      break;
  }

  // Create the palette.
  hPalette = CreatePalette (lpMem);

  // Free the memory object lpMem. 
  LocalFree ((HLOCAL) lpMem);

exit:
  return hPalette;
}

/***********************************************************************

FUNCTION: 
  DisplayPalette

PURPOSE: 
  Shows how to select a logical palette and map the palette entries from
  the logical palette to the system palette.

***********************************************************************/
VOID DisplayPalette (HWND hwnd)
{
  HDC hDC;              // Handle to a display device context  
  RECT rect,            // RECT structure for the small rectangles
       rectClient;      // RECT structure for the window's client area
  HBRUSH hBrush,        // Handle to the new brush object  
         hBrushOld;     // Handle to the old brush object  
  HPALETTE hPal,        // Handle to a palette object  
           hSysPal;     // Handle to the system palette  
  int index,            // Integer index
      iColor,           // Indicates the color of the palette
      iStep,            // Width of the small rectangles
      iWidth, iHeight;  // Width and height of window's client area
  
  // Retrieve the handle to the display device context for the client 
  // area of a window. 
  if (!(hDC = GetDC (hwnd)))
    return;

  // Retrieve the width and height of window's client area.
  GetClientRect (g_hwndMain, &rectClient);
  iWidth = rectClient.right - rectClient.left;
  iHeight = rectClient.bottom - rectClient.top;

  iStep = 2;
  
  for (iColor = 0; iColor <= 3; ++iColor)
  {
    // Create a logical palette.
    if ((hPal = CreateScalePalette (hDC, iColor)) == NULL)
      return;

    // Select the logical palette into the device context. 
    if (iColor ==0)
      // Save the system palette handle.
      hSysPal = SelectPalette (hDC, hPal, FALSE);
    else
      SelectPalette (hDC, hPal, FALSE);

    // Map the palette entries from the logical palette to the system 
    // palette.
    RealizePalette (hDC);
    
    for (index = 0; index < iWidth / iStep; index++)
    {
      hBrush = CreateSolidBrush (PALETTEINDEX (index % iPalSize));
      hBrushOld = SelectObject (hDC, hBrush);
      SetRect (&rect, index * iStep, 0, (index + 1) * iStep, iHeight);
      FillRect (hDC, &rect, hBrush);

      // Select the old brush into the device context.
      SelectObject (hDC, hBrushOld);
  
      // Delete the new brush.
      DeleteObject (hBrush);
    }
    
    // Pause for one second before displaying the next palette.
    Sleep (1000);

    DeleteObject (hPal);
  }

  // Select the system palette.
  SelectPalette (hDC, hSysPal, FALSE);
  RealizePalette (hDC);
    
  ReleaseDC (hwnd, hDC);
  return;
}

/***********************************************************************

FUNCTION: 
  BitmapDemo

PURPOSE: 
  Shows how to create a bitmap compatible with the device that is
  associated with the memory device context.

***********************************************************************/
VOID BitmapDemo (HWND hwnd)
{
  
  HDC hDC,                  // Handle to the display device context 
      hDCMem;               // Handle to the memory device context
  HBITMAP hBitmap,          // Handle to the new bitmap
          hOldBitmap;       // Handle to the old bitmap
  static int iCoordinate[200][4];  
  int i, j,                
      iXSrc, iYSrc,         // X-coordinate and y-coordinate of the  
                            // source rectangle's upper-left corner
      iXDest, iYDest,       // X-coordinate and y-coordinate of the  
                            // destination rectangle's upper-left corner
      iWidth, iHeight;      // Width and height of the bitmap

  // Retrieve the handle to the display device context for the client 
  // area of a window.
  if (!(hDC = GetDC (hwnd)))
    return;

  // Create a memory device context that is compatible with the device.
  hDCMem = CreateCompatibleDC (hDC);

  // Retrieve the width and height of the bitmap.
  iWidth = GetSystemMetrics (SM_CXSCREEN) / 10;
  iHeight = GetSystemMetrics (SM_CYSCREEN) / 10;

  // Create a bitmap that is compatible with the device associated with 
  // the memory device context.
  hBitmap = CreateCompatibleBitmap (hDC, iWidth, iHeight);

  // Select the bitmap object into the memory device context. 
  hOldBitmap = SelectObject (hDCMem, hBitmap);

  for (i = 0; i < 2; i++)
  {
    for (j = 0; j < 200; j++)
    {
      if (i == 0)
      {
        iCoordinate[j][0] = iXDest = iWidth * (rand () % 10);
        iCoordinate[j][1] = iYDest = iHeight * (rand () % 10);
        iCoordinate[j][2] = iXSrc = iWidth * (rand () % 10);
        iCoordinate[j][3] = iYSrc = iHeight * (rand () % 10);
      }
      else
      {
        iXDest = iCoordinate[200 - 1 - j][0];
        iYDest = iCoordinate[200 - 1 - j][1];
        iXSrc = iCoordinate[200 - 1 - j][2];
        iYSrc = iCoordinate[200 - 1 - j][3];
      }

      // Transfer pixels from the source rectangle to the destination
      // rectangle.
      BitBlt (hDCMem, 0, 0, iWidth, iHeight, hDC,  iXDest, iYDest, 
              SRCCOPY);
      BitBlt (hDC,  iXDest, iYDest, iWidth, iHeight, hDC,  iXSrc, iYSrc,
              SRCCOPY);
    }
  }

  // Select the old brush into the device context.
  SelectObject (hDC, hOldBitmap);

  // Delete the bitmap object. 
  DeleteObject (hBitmap);

  // Delete the memory device context and the display device context.
  DeleteDC (hDCMem);
  DeleteDC (hDC);

  return;  
}


/***********************************************************************

FUNCTION: 
  DisplayLine

PURPOSE: 
  Creates four regions, and paints them with different color and texture
  brushes. Draws two lines in the device content. One is a straight line
  that serves as the axis and the other is a sine wave.

***********************************************************************/
VOID DisplayLine (HWND hwnd)
{
  HDC hDC;            // Handle to the display device context 
  HRGN hRgn;          // Handle to a region object  
  HPEN hPen,          // Handle to the new pen object  
       hOldPen;       // Handle to the old pen object  
  HBRUSH hBrush;      // Handle to a brush object 
  RECT rect;          // RECT structure that contains the window’s  
                      // client area coordinates
  int index;          // Integer index
  POINT ptAxis[2],    // Two dimensional POINT structure array
        ptSine[NUMPT];// 200 dimensional POINT structure array

  // Retrieve the handle to the display device context for the client 
  // area of a window.
  if (!(hDC = GetDC (hwnd)))
    return;

  // Retrieve the coordinates of the window’s client area. 
  GetClientRect (hwnd, &rect);

  // Create a rectangular region.
  hRgn = CreateRectRgn (0, 0, rect.right / 4, rect.bottom);

  // Create a solid brush.
  hBrush = CreateSolidBrush (g_crColor[0]); 

  // Fill the region by using the created brush.
  FillRgn (hDC, hRgn, hBrush);
  
  // Delete the rectangular region. 
  DeleteObject (hRgn);

  // Delete the brush object.
  DeleteObject (hBrush);

  // Create a rectangular region.
  hRgn = CreateRectRgn (rect.right / 4, 0, rect.right / 2, 
                        rect.bottom);
  
  // Create a solid brush.
  hBrush = CreateSolidBrush (g_crColor[1]); 
  
  // Fill the region by using the created brush.
  FillRgn (hDC, hRgn, hBrush);

  // Delete the rectangular region. 
  DeleteObject (hRgn);

  // Delete the brush object.
  DeleteObject (hBrush);

  // Create a rectangular region.
  hRgn = CreateRectRgn (rect.right / 2, 0, rect.right * 3 / 4, 
                        rect.bottom);
  
  // Create a solid brush.
  hBrush = CreatePatternBrush (LoadBitmap (
                                        g_hInst, 
                                        MAKEINTRESOURCE (IDB_BITMAP)));
  
  // Fill the region by using the created brush.
  FillRgn (hDC, hRgn, hBrush);
  
  // Delete the rectangular region. 
  DeleteObject (hRgn);

  // Delete the brush object.
  DeleteObject (hBrush);

  // Create a rectangular region.
  hRgn = CreateRectRgn (rect.right * 3 / 4, 0, rect.right, rect.bottom);
  
  // Create a solid brush.
  hBrush = CreateSolidBrush (g_crColor[7]); 
  
  // Fill the region by using the created brush.
  FillRgn (hDC, hRgn, hBrush);

  // Delete the rectangular region. 
  DeleteObject (hRgn);

  // Delete the brush object.
  DeleteObject (hBrush);
 
  // Assign the axis points coordinates in pixels.
  ptAxis[0].x = 0;
  ptAxis[0].y = g_iCBHeight + (rect.bottom - g_iCBHeight) / 2;
  ptAxis[1].x = rect.right - 1;
  ptAxis[1].y = ptAxis[0].y;

  // Assign the sine wave points coordinates in pixels.
  for (index = 0; index < NUMPT; ++index)
  {
    ptSine[index].x = index * rect.right / NUMPT;
    ptSine[index].y = (long) (g_iCBHeight + \
                              (rect.bottom - g_iCBHeight) / 2 * \
                              (1 - sin (8 * 3.14159 * index / NUMPT)));
  }
  
  // Create a dash pen object and select it.
  hPen = CreatePen (PS_DASH, 1, g_crColor[5]);
  hOldPen = SelectObject (hDC, hPen);

  // Draw a straight line that connects the two points.
  Polyline (hDC, ptAxis, 2);

  // Select the old brush into the device context.
  SelectObject (hDC, hOldPen);

  // Delete the pen object.
  DeleteObject (hPen);

  // Create a solid pen object and select it.
  hPen = CreatePen (PS_SOLID, 3, g_crColor[5]);
  hOldPen = SelectObject (hDC, hPen);

  // Draw a sine wave shaped polyline.
  Polyline (hDC, ptSine, NUMPT);

  // Select the old brush into the device context.
  SelectObject (hDC, hOldPen);

  // Delete the pen object.
  DeleteObject (hPen);

  // Release the device context.
  ReleaseDC (hwnd, hDC);

  return;
}
// END OF DRAWOBJ.C
