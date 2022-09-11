/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Sspi.c

ABSTRACT: 
  This code sample shows how use the Security Support Provider Interface
  to access common authentication. 

  Users need to insert code for assigning the server name, making the
  second call to the function InitializeSecurityContext if the client 
  must send the output token (returned by the first call to the
  function) to the server.

***********************************************************************/
#include <windows.h>
#include <sspi.h>
#include <issperr.h>
#include <winsock.h>

#define BUFFERLEN     16384 

BOOL InitSspi (HINSTANCE);
BOOL AuthConn (PSecurityFunctionTable, PSecPkgInfo, DWORD);

/***********************************************************************

FUNCTION: 
  WinMain

PURPOSE: 
  Called by the system as the initial entry point for this Windows 
  CE-based application.

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, int nCmdShow)
{
  HINSTANCE DllHandle;
  TCHAR szError[100];

  // Load the security provider DLL.
  DllHandle = LoadLibrary (TEXT("secur32.dll"));

  if (!DllHandle)
  {
    wsprintf (szError, 
              TEXT("Failed in loading secur32.dll, Error: %x"),
              GetLastError ());
    return 0;
  }

  if (!InitSspi (DllHandle))
  {
    MessageBox (NULL, 
                TEXT("Failed in initializing the SSPI."),
                TEXT("Error"),
                MB_OK);
    return 0;
  }

  return 1;
}


/***********************************************************************

FUNCTION: 
  InitSspi

PURPOSE: 
  Initializes Security Support Provider Interface.

***********************************************************************/
BOOL InitSspi (HINSTANCE DllHandle)
{
  DWORD dwIndex,
        dwNumOfPkgs,
        dwPkgToUse;
  TCHAR szError[100];

  INIT_SECURITY_INTERFACE InitSecurityInterface;
  PSecurityFunctionTable pSecurityInterface = NULL;
  PSecPkgInfo pSecurityPackages = NULL;
  SECURITY_STATUS status;
  ULONG ulCapabilities;

  // Get the address of the function InitSecurityInterface.
  InitSecurityInterface = (INIT_SECURITY_INTERFACE) GetProcAddress (
                                        DllHandle, 
                                        TEXT("InitSecurityInterfaceW"));

  if (!InitSecurityInterface)
  {
    wsprintf (szError, 
              TEXT("Failed in getting the function address, Error: %x"),
              GetLastError ());
    return FALSE;
  }

  // Use InitSecurityInterface to get the function table.
  pSecurityInterface = (*InitSecurityInterface)();

  if (!pSecurityInterface)
  {
    wsprintf (szError, 
              TEXT("Failed in getting the function table, Error: %x"),
              GetLastError ());
    return FALSE;
  }

  if (!(pSecurityInterface->EnumerateSecurityPackages))
  {
    wsprintf (szError, 
              TEXT("Failed in getting the function table, Error: %x"),
              GetLastError ());
    return FALSE;
  }

  // Retrieve the security packages supported by the provider.
  status = (*pSecurityInterface->EnumerateSecurityPackages)(
                                                    &dwNumOfPkgs, 
                                                    &pSecurityPackages);
  if (status != SEC_E_OK)
  {
    wsprintf (szError, 
              TEXT("Failed in retrieving security packages, Error: %x"),
              GetLastError ());
    return FALSE;
  }

  // Initialize dwPkgToUse. 
  dwPkgToUse = -1;

  // Assume the application needs integrity, privacy, and impersonation 
  // on messages.
  ulCapabilities = SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY | 
                   SECPKG_FLAG_IMPERSONATION;

  // Determine which package should be used.
  for (dwIndex = 0; dwIndex < dwNumOfPkgs; dwIndex++)
  {
    if ((pSecurityPackages[dwIndex].fCapabilities & ulCapabilities) ==
        ulCapabilities)
    {
      dwPkgToUse = dwIndex;
      break;
    }
  }

  if (!AuthConn (pSecurityInterface, pSecurityPackages, dwPkgToUse))
  {
    MessageBox (NULL, 
                TEXT("Failed in authenticating a connection."),
                TEXT("Error"),
                MB_OK);
    return FALSE;
  }
  
  return TRUE;
}

/***********************************************************************

FUNCTION: 
  AuthConn

PURPOSE: 
  Authenticates a connection.

***********************************************************************/
BOOL AuthConn (PSecurityFunctionTable pSecurityInterface,
               PSecPkgInfo pSecurityPackages,
               DWORD dwPkgToUse)
{
  BOOL bReturn = FALSE;           // Return value of the function
  TCHAR szError[100],             // String for the error message
        szTargetName[100];        // Target name 
  LPSTR pszOutBuffer = NULL;      // Used in security data xfr
  ULONG ulContextReq,             // Required context attributes
        ulContextAttributes;      // Receives attributes of the context
  TimeStamp tsExpiry;             // Returned credentials' life time
  SECURITY_STATUS status;         // Return codes
  CredHandle hCredential;         // Handle to the credential 
  CtxtHandle hNewContext;         // Handle to the security context
  SecBuffer OutSecBuffer;         // Output buffer
  SecBufferDesc OutBufferDesc;    // Output buffer descriptor

  // Check if the pointer to SecurityFunctionTable is valid.
  if (!pSecurityInterface)
    goto exit;
 
  // Allocate buffer memory for pszOutBuffer.
  if (!(pszOutBuffer = new char[BUFFERLEN]))
    goto exit;

  // Acquire an outbound credential handle.
  status = (*pSecurityInterface->AcquireCredentialsHandle)(
                                    NULL,
                                    pSecurityPackages[dwPkgToUse].Name,
                                    SECPKG_CRED_OUTBOUND,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &hCredential,
                                    &tsExpiry);
  if (status != SEC_E_OK)
  {
    wsprintf (szError, 
              TEXT("Failed in acquiring the credential handle: %x"), 
              status);
    goto exit;
  }

  // Initialize the OutSecBuffer structure.
  OutSecBuffer.cbBuffer = BUFFERLEN;
  OutSecBuffer.BufferType = SECBUFFER_TOKEN;
  OutSecBuffer.pvBuffer = pszOutBuffer;

  // Initialize the OutBufferDesc structure.
  OutBufferDesc.ulVersion = 0;
  OutBufferDesc.cBuffers = 1;
  OutBufferDesc.pBuffers = &OutSecBuffer;

  ulContextReq = ISC_REQ_MUTUAL_AUTH | ISC_REQ_CONNECTION |
                 ISC_REQ_SEQUENCE_DETECT | ISC_REQ_REPLAY_DETECT | 
                 ISC_REQ_CONFIDENTIALITY | ISC_REQ_ALLOCATE_MEMORY;

  // Assign the target (server) name.
  // wcscpy (szTargetName, TEXT("..."));

  // Get the authentication token from the security package to send to
  // the server to request an authenticated token.
  status = (*pSecurityInterface->InitializeSecurityContext)(
                                            &hCredential,
                                            NULL,
                                            szTargetName,
                                            ulContextReq,
                                            0,
                                            SECURITY_NATIVE_DREP,
                                            NULL,
                                            0,
                                            &hNewContext,
                                            &OutBufferDesc,
                                            &ulContextAttributes,
                                            &tsExpiry);

  if (status == SEC_I_CONTINUE_NEEDED)
  {
    SOCKET Socket = INVALID_SOCKET;         // Server socket 
  
    // Add code here to connect to server. Get the server socket.
    // ...

    // Send hCredential to server
    if (send (Socket, (const char *)OutSecBuffer.pvBuffer, 
              OutSecBuffer.cbBuffer, 0) == SOCKET_ERROR)
    {
      wsprintf (szError, 
                TEXT("Failed in sending hCredential to the server: %d"),
                WSAGetLastError ());
      goto exit;
    }

    // Add code here to make the second call to the function 
    // InitializeSecurityContext.
    // ...
  }
  else 
  {
    if (status != SEC_E_OK)
    {
      wsprintf (szError, 
                TEXT("Failed in initiating the outbound security ")
                TEXT("context: %x"),
                status);
      goto exit;
    }
  }

  bReturn = TRUE;

exit:

  if (pszOutBuffer) 
    delete[] pszOutBuffer;

  if (pSecurityInterface)
  {
    (*pSecurityInterface->FreeCredentialHandle)(&hCredential);
    (*pSecurityInterface->DeleteSecurityContext)(&hNewContext);
    (*pSecurityInterface->FreeContextBuffer)(&OutBufferDesc);
  }

  return bReturn;
}