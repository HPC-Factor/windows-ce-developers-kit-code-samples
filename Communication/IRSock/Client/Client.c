/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Client.c

ABSTRACT:
  This code sample demonstrates how to implement an IR Socket client.
  It sends a message to the server and checks the incoming message 
  sent by the server.

***********************************************************************/

#include <windows.h>
#include <af_irda.h>

#define NUMRETYR  5                   // Maximum number of retries

int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
{
  SOCKET sock;                    // Socket bound to the server
  DEVICELIST devList;             // Device list
  SOCKADDR_IRDA address = {AF_IRDA, 0, 0, 0, 0, "IRServer"};
                                  // Specifies the server socket address
  int iCount = 0,                 // Number of retries
      index = 0,                  // Integer index
      iReturn,                    // Return value of recv function
      iDevListLen = sizeof (devList);
                                  // Size of the device list
  char szClientA[100];            // ASCII string 
  TCHAR szClientW[100];           // UNICODE string
  TCHAR szError[100];             // Error message string

  // Create a socket that is bound to the server.
  if ((sock = socket (AF_IRDA, SOCK_STREAM, 0)) == INVALID_SOCKET)
  {
    wsprintf (szError, TEXT("Allocating socket failed. Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }
  
  // Initialize the number of devices to zero.
  devList.numDevice = 0;      

  while ( (devList.numDevice == 0) && (iCount <= NUMRETYR))
  {
    // Retrieve the socket option.
    if (getsockopt (sock, SOL_IRLMP, IRLMP_ENUMDEVICES, 
                    (char *)&devList, &iDevListLen) == SOCKET_ERROR)
    {
      wsprintf (szError, TEXT("Server could not be located, getsockopt")
                TEXT(" failed. Error: %d"), WSAGetLastError ());
      MessageBox (NULL, szError, TEXT("Error"), MB_OK);
      closesocket (sock);
      return FALSE;
    }
    
    iCount++;

    // Wait one second before retrying.
    Sleep (1000);          
  }

  if (iCount > NUMRETYR)
  {
    MessageBox (NULL, TEXT ("Server could not be located!"), 
                TEXT ("Error"), MB_OK);
    closesocket (sock);
    return FALSE;
  }

  // Get the server socket address.
  for (index = 0; index <= 3; index++)
  {
    address.irdaDeviceID[index] = devList.Device[0].irdaDeviceID[index];
  }

  // Establish a connection to the socket.
  if (connect (sock, (struct sockaddr *)&address, 
               sizeof (SOCKADDR_IRDA)) == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Connecting to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (sock);
    return FALSE;
  }

  // Send a string from the client socket to the server socket.
  if (send (sock, "To Server.", strlen ("To Server.") + 1, 0)
           == SOCKET_ERROR) 
  {
    wsprintf (szError, 
              TEXT("Sending data to the server failed. Error: %d"),
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
  }

  // Receive data from the server socket.
  iReturn = recv (sock, szClientA, sizeof (szClientA), 0);

  // Check if there is any data received. If there is, display it.
  if (iReturn == SOCKET_ERROR)
  {
    wsprintf (szError, TEXT("No data is received, recv failed.")
              TEXT(" Error: %d"), WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Client"), MB_OK);
  }
  else if (iReturn == 0)
  {
    MessageBox (NULL, TEXT("Finished receiving data"), TEXT("Client"),
                MB_OK);
  }
  else
  {
    // Convert the ASCII string to the UNICODE string.
    for (index = 0; index <= sizeof (szClientA); index++)
      szClientW[index] = szClientA[index];

    // Display the string received from the server.
    MessageBox (NULL, szClientW, TEXT("Received From Server"), MB_OK);
  }

  // Close the socket.
  closesocket (sock);

  return 0;
}
