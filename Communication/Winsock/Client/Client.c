/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Client.c

ABSTRACT: 
  This code sample demonstrates how to implement a Winsock client.
  It sends a message to the server and checks the incoming message 
  sent by the server. This sample can be run on the same device with
  Server.exe. In that case, HOSTNAME should be defined as "localhost".
  Otherwise, HOSTNAME should be defined as the full server name.

***********************************************************************/

#include <windows.h> 
#include <winsock.h>

#define PORTNUM         5000          // Port number
#define HOSTNAME        "localhost"   // Server name string
                                      // This should be changed
                                      // according to the server
int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
{
  int index = 0,                      // Integer index
      iReturn;                        // Return value of recv function
  char szClientA[100];                // ASCII string 
  TCHAR szClientW[100];               // UNICODE string
  TCHAR szError[100];                 // Error message string

  SOCKET ServerSock = INVALID_SOCKET; // Socket bound to the server
  SOCKADDR_IN destination_sin;        // Server socket address
  PHOSTENT phostent = NULL;           // Points to the HOSTENT structure
                                      // of the server
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

  // Create a TCP/IP socket that is bound to the server.
  if ((ServerSock = socket (AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Fill out the server socket's address information.
  destination_sin.sin_family = AF_INET;

  // Retrieve the host information corresponding to the host name.
  if ((phostent = gethostbyname (HOSTNAME)) == NULL) 
  {
    wsprintf (szError, TEXT("Unable to get the host name. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // Assign the socket IP address.
  memcpy ((char FAR *)&(destination_sin.sin_addr), 
          phostent->h_addr, 
          phostent->h_length);

  // Convert to network ordering.
  destination_sin.sin_port = htons (PORTNUM);      

  // Establish a connection to the server socket.
  if (connect (ServerSock, 
               (PSOCKADDR) &destination_sin, 
               sizeof (destination_sin)) == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Connecting to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // Send a string to the server.
  if (send (ServerSock, "To Server.", strlen ("To Server.") + 1, 0)
           == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Sending data to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
  }

  // Disable sending on ServerSock.
  shutdown (ServerSock, 0x01);

  for (;;)
  {
    // Receive data from the server socket.
    iReturn = recv (ServerSock, szClientA, sizeof (szClientA), 0);

    // Check if there is any data received. If there is, display it.
    if (iReturn == SOCKET_ERROR)
    {
      wsprintf (szError, TEXT("No data is received, recv failed.")
                TEXT(" Error: %d"), WSAGetLastError ());
      MessageBox (NULL, szError, TEXT("Client"), MB_OK);
      break;
    }
    else if (iReturn == 0)
    {
      MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Client"),
                  MB_OK);
      break;
    }
    else
    {
      // Convert the ASCII string to the UNICODE string.
      for (index = 0; index <= sizeof (szClientA); index++)
        szClientW[index] = szClientA[index];

      // Display the string received from the server.
      MessageBox (NULL, szClientW, TEXT("Received From Server"), MB_OK);
    }
  }

  // Disable receiving on ServerSock.
  shutdown (ServerSock, 0x00);

  // Close the socket.
  closesocket (ServerSock);

  WSACleanup ();

  return TRUE;
}