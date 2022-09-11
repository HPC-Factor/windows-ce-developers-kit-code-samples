/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE:  
  FileView.h

ABSTRACT:  
  This is a header file of the FileView Windows CE application.

***********************************************************************/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"

HINSTANCE g_hInst;            // Handle to the application instance
HWND g_hwndMain;              // Handle to the application main window
HWND g_hwndCB;                // Handle to the command bar
HWND g_hwndTreeView;          // Handle to the TreeView control
HIMAGELIST g_hImgList;        // Handle to the image list

  
TCHAR g_szTitle[80];          // Main window name
TCHAR g_szClassName[80];      // Main window class name
                  

#define IDC_TREEVIEW      101 // Identifier of the tree view control
#define CX_BITMAP         16  // Width of each image in the image list
#define CY_BITMAP         15  // Height of each image in the image list 
#define NUM_BITMAPS       3   // Number of images in the image list 
#define TVIS_GCNOCHECK    1    
#define TVIS_GCCHECK      2      
#define TVIS_GCSOMECHECK  3    

typedef enum tagIMAGES {IMAGE_OPEN, IMAGE_CLOSED, IMAGE_WINDOW,
                        IMAGE_CLASS, IMAGE_DOCUMENT} IMAGES;

BOOL InitTreeViewImageLists (HWND);
BOOL BuildDirectory (HWND, TV_ITEM, LPTSTR);
BOOL GetDirectoryContents (HWND, LPTSTR, HTREEITEM);
HTREEITEM AddItemToTree (HWND, LPTSTR, HTREEITEM, BOOL);
