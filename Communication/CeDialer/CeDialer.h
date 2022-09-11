/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  CeDialer.h

ABSTRACT: 
  This is a header file of the CeDialer Windows CE sample application.

***********************************************************************/

#ifndef _CEDIALER_H_
#define _CEDIALER_H_

#define ERR_NONE              0
#define TAPI_VERSION_1_0      0x00010003
#define TAPI_VERSION_1_4      0x00010004
#define TAPI_VERSION_2_0      0x00020000
#define TAPI_CURRENT_VERSION  TAPI_VERSION_2_0

typedef struct tagLINEINFO
{
  HLINE hLine;              // Line handle returned by lineOpen
  BOOL  bVoiceLine;         // Indicates if the line is a voice line
  DWORD dwAPIVersion;       // API version that the line supports
  DWORD dwNumOfAddress;     // Number of available addresses on the line
  DWORD dwPermanentLineID;  // Permanent line identifier
  TCHAR szLineName[256];    // Name of the line
} LINEINFO, *LPLINEINFO;

int WINAPI WinMain (HINSTANCE, HINSTANCE, LPTSTR, int);

BOOL CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialingProc (HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK ConnectUsingProc (HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK tapiCallback (DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);

DWORD InitializeTAPI ();
DWORD GetLineInfo (DWORD, LPLINEINFO);
VOID MakePhoneCall (LPCTSTR);
VOID CurrentLineClose ();
VOID HangupCall ();

BOOL MakeCanonicalNum (LPTSTR);
BOOL InitLineCB (HWND, DWORD, DWORD);
BOOL InitAddrCB (HWND, HWND, DWORD, DWORD);
  
#endif 
