/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  TreeView.c

ABSTRACT: 
  This is a C file of the FileView Windows CE sample application. It 
  contains functions for adding items in the tree view control, setting 
  the image list for the tree view control, retrieving the files inside
  a directory, and adding these files to the tree view control.

***********************************************************************/

#include "FileView.h"

/***********************************************************************

FUNCTION: 
  InitTreeViewImageLists 

PURPOSE: 
  Creates an image list and adds images to the image list. Sets the  
  image list for the tree view control and redraws the control by 
  using the new images.

***********************************************************************/
BOOL InitTreeViewImageLists (HWND hwndTreeView)
{
  HBITMAP hBmp;          // Handle of the bitmaps to be added

  // Create the image list for the item pictures.
  if ((g_hImgList = ImageList_Create (CX_BITMAP, CY_BITMAP, ILC_MASK, 
                                      NUM_BITMAPS, 0)) == NULL)
    return FALSE;

  // Load the bitmap resource.
  hBmp = LoadBitmap (g_hInst, MAKEINTRESOURCE (IDB_IMAGES));

  // Add the images to the image list, generating a mask from the 
  // bitmap.
  if (ImageList_AddMasked (g_hImgList, hBmp, RGB (0, 255, 0)) == -1)
  {
    return FALSE;
  }

  // Delete the bitmap object and free system resources.
  DeleteObject (hBmp);

  // If not all the images were added, then return.
  if (ImageList_GetImageCount (g_hImgList) < NUM_BITMAPS)
    return FALSE;

  // Set the image list for the tree view control and redraw the  
  // control by using the new images.
  TreeView_SetImageList (hwndTreeView, g_hImgList, TVSIL_NORMAL);
  
  return TRUE;
}

/***********************************************************************

FUNCTION: 
  AddItemToTree 

PURPOSE: 
  Adds an item to the tree view control.

***********************************************************************/
HTREEITEM AddItemToTree (HWND hwndTreeView, LPTSTR lpszItem, 
                         HTREEITEM htiParent, BOOL bDirectory)
{
  TV_ITEM tvi;
  HTREEITEM hti;
  TV_INSERTSTRUCT tvins;
  
  // Filter out the "." and ".." directories.
  if (!lstrcmpi (lpszItem, TEXT (".")) || 
      !lstrcmpi (lpszItem, TEXT ("..")))
  {
    return (HTREEITEM)TRUE;
  }

  // Initialize the structures.
  memset (&tvi, 0, sizeof (TV_ITEM));
  memset (&tvins, 0, sizeof (TV_INSERTSTRUCT));
  tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;

  // If it's a directory, add a child count so the expand button shows.
  if (bDirectory)
  {
    tvi.mask |= TVIF_CHILDREN;
    tvi.cChildren = 1;
  }

  // Set the text of the item.
  tvi.pszText = lpszItem;
  tvi.cchTextMax = lstrlen (lpszItem);

  // Give the item the appropriate image.
  tvi.iImage = bDirectory ? IMAGE_CLOSED : IMAGE_DOCUMENT;
  tvi.iSelectedImage = bDirectory ? IMAGE_CLOSED : IMAGE_DOCUMENT;
  tvins.item = tvi;
  tvins.hInsertAfter = TVI_SORT;

  // Set the parent item based on the specified level.
  if (!htiParent)
    tvins.hParent = TVI_ROOT;
  else
    tvins.hParent = htiParent;

  // Add the item to the tree view control.
  hti = (HTREEITEM) SendMessage (hwndTreeView, TVM_INSERTITEM, 0, 
                                 (LPARAM) (LPTV_INSERTSTRUCT) &tvins);
  // Return the handle to the item.
  return hti;
}

/***********************************************************************

FUNCTION: 
  BuildDirectory

PURPOSE: 
  Gets the path and name of a directory.

***********************************************************************/
BOOL BuildDirectory (HWND hwndTreeView, TV_ITEM tvi, LPTSTR lpszDir)
{
  HTREEITEM hti;            
  LPTSTR lpszDirTmp,            // Temperary string
         lpszTextTmp;           // Temperary string

  // Allocate memory for the temperary strings.
  lpszTextTmp = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * MAX_PATH);
  lpszDirTmp = (LPTSTR) LocalAlloc (LPTR, sizeof (TCHAR) * MAX_PATH);

  // Get the text for the first item.
  tvi.mask |= TVIF_TEXT;
  tvi.pszText = lpszTextTmp;
  tvi.cchTextMax = MAX_PATH;

  if (!TreeView_GetItem (hwndTreeView, &tvi))
    return FALSE;

  // Creates the initial string.
  wsprintf (lpszDirTmp, TEXT ("%s"), tvi.pszText);
  lstrcpy (lpszDir, lpszDirTmp);

  hti = tvi.hItem;

  // Add the parent directories if there are any.
  while (hti = TreeView_GetParent (hwndTreeView, hti))
  {
    tvi.mask = TVIF_TEXT;
    tvi.hItem = hti;
    if (!TreeView_GetItem (hwndTreeView, &tvi))
      return FALSE;

    lstrcpy (lpszDirTmp, lpszDir);

    // If at the root, don't put "\" between the two strings. 
    // Otherwise, put "\" between them.
    if (wcscmp (tvi.pszText, TEXT ("\\")) == 0)
      wsprintf (lpszDir, TEXT ("%s%s"), tvi.pszText, lpszDirTmp);
    else
      wsprintf (lpszDir, TEXT ("%s\\%s"), tvi.pszText, lpszDirTmp);
  }

  // Add the wildcard needed for FindFirstFile ().
  lstrcpy (lpszDirTmp, lpszDir);

  // If at the root, don't put "\" between the two strings. 
  // Otherwise, put "\" between them.
  if (wcscmp (lpszDirTmp, TEXT ("\\")) == 0)          
    wsprintf (lpszDir, TEXT ("%s*.*"), lpszDirTmp);
  else
    wsprintf (lpszDir, TEXT ("%s\\*.*"), lpszDirTmp);

  // Free the strings.
  LocalFree (lpszTextTmp);
  LocalFree (lpszDirTmp);

  return TRUE;
}


/***********************************************************************

FUNCTION: 
  GetDirectoryContents 

PURPOSE: 
  Retrieves the files inside a directory and adds these files to the 
  tree view control.

***********************************************************************/
BOOL GetDirectoryContents (HWND hwndTreeView, 
                           LPTSTR pszDirectory, 
                           HTREEITEM htiParent)
{
  BOOL bInserted = FALSE;   // Indicates if all items gets inserted
  HANDLE hSearch;           // Search handle returned by FindFirstFile
  WIN32_FIND_DATA wfd;      // Contains information of a file
  
  __try
  {
    // Get the first file in the directory.
    hSearch = FindFirstFile (pszDirectory, &wfd);

    if (hSearch != INVALID_HANDLE_VALUE)
    {
      if (!AddItemToTree (
                      hwndTreeView, 
                      wfd.cFileName, 
                      htiParent, 
                      wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      __leave;

      // Loop on all remaining entries in the directory.
      while (FindNextFile (hSearch, &wfd))
      {
        if (!AddItemToTree (
                      hwndTreeView, 
                      wfd.cFileName, 
                      htiParent, 
                      wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        __leave;
      }
    }
    else
    {
      if (GetLastError () != ERROR_NO_MORE_FILES)
        OutputDebugString (
              TEXT ("FindFirstFile returned invalid handle.\r\n"));
      else
      {
        TV_ITEM tvi;

        tvi.hItem = htiParent;
        tvi.mask = TVIF_CHILDREN;
        tvi.cChildren = 0;

        if (!TreeView_SetItem (hwndTreeView, &tvi))
          OutputDebugString (
                TEXT ("ERROR: setting item for no children.\r\n"));
      }
    }

    // All items are successfully inserted. Assign bInserted as TRUE.
    bInserted = TRUE;
  }

  __finally
  {
    if (hSearch != INVALID_HANDLE_VALUE)
      FindClose (hSearch);
  }

  return bInserted;
}
// END OF TREEVIEW.C
