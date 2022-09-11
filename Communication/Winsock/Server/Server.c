/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Server.c

ABSTRACT: 
  This code sample demonstrates how to implement a Winsock server.
  It checks the incoming message sent by the client and sends a
  message to the client. This sample can be run on different devices
  or the same device with Client.exe. 

***********************************************************************/

#include <windows.h> 
#include <winsock.h>

#define PORTNUM               5000    // Port number  
#define MAX_PENDING_CONNECTS  4       // Maximum length of the queue 
                                      // of pending connections
int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
{        
  int index = 0,                      // Integer index
      iReturn;                        // Return value of recv function
  char szServerA[100];                // ASCII string 
  TCHAR szServerW[100];               // UNICODE string
  TCHAR szError[100];                 // Error message string

  SOCKET WinSocket = INVALID_SOCKET,  // Window socket
         ClientSock = INVALID_SOCKET; // Socket for communicating 
                                      // between the server and client
  SOCKADDR_IN local_sin,              // Local socket address
              accept_sin;             // Receives the address of the 
                                      // connecting entity
  int accept_sin_len;                 // Length of accept_sin

  WSADATA WSAData;                    // Contains details of the Windows
                                      // Sockets implementation

  // Initiate Windows Sockets.
  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
  {
    wsprintf (szError, TEXT("WSAStartup failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Create a TCP/IP socket, WinSocket.
  if ((WinSocket = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
  {
    wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Fill out the local socket's address information.
  local_sin.sin_family = AF_INET;
  local_sin.sin_port = htons (PORTNUM);  
  local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

  // Associate the local address with WinSocket.
  if (bind (WinSocket, 
            (struct sockaddr *) &local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) 
  {
    wsprintf (szError, TEXT("Binding socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (WinSocket);
    return FALSE;
  }

  // Establish a socket to listen for incoming connections.
  if (listen (WinSocket, MAX_PENDING_CONNECTS) == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Listening to the client failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (WinSocket);
    return FALSE;
  }

  accept_sin_len = sizeof (accept_sin);

  // Accept an incoming connection attempt on WinSocket.
  ClientSock = accept (WinSocket, 
                       (struct sockaddr *) &accept_sin, 
                       (int *) &accept_sin_len);

  // Stop listening for connections from clients.
  closesocket (WinSocket);

  if (ClientSock == INVALID_SOCKET) 
  {
    wsprintf (szError, TEXT("Accepting connection with client failed.")
              TEXT(" Error: %d"), WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  for (;;)
  {
    // Receive data from the client.
    iReturn = recv (ClientSock, szServerA, sizeof (szServerA), 0);

    // Check if there is any data received. If there is, display it.
    if (iReturn == SOCKET_ERROR)
    {
      wsprintf (szError, TEXT("No data is received, recv failed.")
                TEXT(" Error: %d"), WSAGetLastError ());
      MessageBox (NULL, szError, TEXT("Server"), MB_OK);
      break;
    }
    else if (iReturn == 0)
    {
      MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Server"),
                  MB_OK);
      break;
    }
    else
    {
      // Convert the ASCII string to the UNICODE string.
      for (index = 0; index <= sizeof (szServerA); index++)
        szServerW[index] = szServerA[index];

      // Display the string received from the client.
      MessageBox (NULL, szServerW, TEXT("Received From Client"), MB_OK);
    }
  } 

  // Send a string from the server to the client.
  if (send (ClientSock, "To Client.", strlen ("To Client.") + 1, 0)
           == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Sending data to the client failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
  }

  // Disable both sending and receiving on ClientSock.
  shutdown (ClientSock, 0x02);

  // Close ClientSock.
  closesocket (ClientSock);

  WSACleanup ();

  return TRUE;
}

