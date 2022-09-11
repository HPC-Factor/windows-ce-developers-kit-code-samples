/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Server.c

ABSTRACT: 
  This code sample demonstrates how to implement an IR socket server.
  It checks the incoming message sent by the client and sends a
  message to the client. 

***********************************************************************/

#include <windows.h>
#include <af_irda.h>

int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
{
  SOCKET ServerSock,              // IR socket bound to the server 
         ClientSock;              // IR socket bound to the client 

  SOCKADDR_IRDA address = {AF_IRDA, 0, 0, 0, 0, "IRServer"};
                                  // Specifies the server socket address
  int index = 0,                  // Integer index
      iReturn;                    // Return value of "recv" function
  char szServerA[100];            // ASCII string 
  TCHAR szServerW[100];           // UNICODE string
  TCHAR szError[100];             // Error message string
  

  // Create a socket bound to the server.
  if ((ServerSock = socket (AF_IRDA, SOCK_STREAM, 0)) == INVALID_SOCKET) 
  {
    wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Associate the server socket address with the server socket.
  if (bind (ServerSock, (struct sockaddr *)&address, sizeof (address))
           == SOCKET_ERROR) 
  {
    wsprintf (szError, TEXT("Binding socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // Establish a socket to listen for incoming connections.
  if (listen (ServerSock, 5) == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Listening to the client failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // Accept a connection on the socket.
  if ((ClientSock = accept (ServerSock, 0, 0)) == INVALID_SOCKET)
  {
    wsprintf (szError, TEXT("Accepting connection with client failed.")
              TEXT(" Error: %d"), WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (ServerSock);
    return FALSE;
  }

  // Stop listening for connections from clients.
  closesocket (ServerSock);

  // Send a string from the server socket to the client socket.
  if (send (ClientSock, "To Client!", strlen ("To Client!") + 1, 0)
           == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Sending data to the client failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
  }

  // Receive data from the client.
  iReturn = recv (ClientSock, szServerA, sizeof (szServerA), 0);

  // Check if there is any data received. If there is, display it.
  if (iReturn == SOCKET_ERROR)
  {
    wsprintf (szError, TEXT("No data is received, recv failed.")
              TEXT(" Error: %d"), WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Server"), MB_OK);
  }
  else if (iReturn == 0)
  {
    MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Server"),
                MB_OK);
  }
  else
  {
    // Convert the ASCII string to the UNICODE string.
    for (index = 0; index <= sizeof (szServerA); index++)
      szServerW[index] = szServerA[index];

    // Display the string received from the client.
    MessageBox (NULL, szServerW, TEXT("Received From Client"), MB_OK);
  }
 
  // Close the client and server sockets.
  closesocket (ClientSock);

  return 0;
}

