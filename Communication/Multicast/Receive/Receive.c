/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Receive.c

ABSTRACT: 
  This code sample shows how to create a datagram window socket. Join 
  the multicast group. Receive data from the multicasting group server.

***********************************************************************/

#include <windows.h> 
#include <winsock.h>

#define RECV_IP_ADDR      "234.5.6.7"     
#define DEST_PORT         4567            

int WINAPI WinMain (
              HINSTANCE hInstance,    // Handle to the current instance
              HINSTANCE hPrevInstance,// Handle to the previous instance
              LPTSTR lpCmdLine,       // Pointer to the command line
              int nCmdShow)           // Show state of the window
{
  int index = 0,                      // Integer index
      iRecvLen;                       // Length of recv_sin
  char szMessageA[100];               // ASCII string 
  TCHAR szMessageW[100];              // UNICODE string
  TCHAR szError[100];                 // Error message string

  SOCKET Sock = INVALID_SOCKET;       // Datagram window socket

  struct ip_mreq mreq;                // Used in adding or dropping 
                                      // multicasting addresses
  SOCKADDR_IN local_sin,              // Local socket's address
              recv_sin;               // Holds the source address upon 
                                      // function recvfrom returns
  WSADATA WSAData;                    // Contains details of the Windows
                                      // Sockets implementation
    
  // Initiate Windows Sockets. 
  if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
  {
    wsprintf (szError, TEXT("WSAStartup failed! Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Create a datagram socket, Sock.
  if ((Sock = socket (AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) 
  {
    wsprintf (szError, TEXT("Allocating socket failed! Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    return FALSE;
  }

  // Fill out the local socket's address information.
  local_sin.sin_family = AF_INET;
  local_sin.sin_port = htons (DEST_PORT);  
  local_sin.sin_addr.s_addr = htonl (INADDR_ANY);

  // Associate the local address with Sock.
  if (bind (Sock, 
            (struct sockaddr FAR *) &local_sin, 
            sizeof (local_sin)) == SOCKET_ERROR) 
  {
    wsprintf (szError, TEXT("Binding socket failed! Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (Sock);
    return FALSE;
  }

  // Join the multicast group from which to receive datagrams.
  mreq.imr_multiaddr.s_addr = inet_addr (RECV_IP_ADDR);
  mreq.imr_interface.s_addr = INADDR_ANY;

  if (setsockopt (Sock, 
                  IPPROTO_IP, 
                  IP_ADD_MEMBERSHIP, 
                  (char FAR *)&mreq, 
                  sizeof (mreq)) == SOCKET_ERROR)
  {
    wsprintf (szError, TEXT("setsockopt failed! Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (Sock);
    return FALSE;
  }

  iRecvLen = sizeof (recv_sin);
  
  // Receive data from the multicasting group server.
  if (recvfrom (Sock, 
                szMessageA,
                100,         
                0,
                (struct sockaddr FAR *) &recv_sin,
                &iRecvLen) == SOCKET_ERROR)
  {
    wsprintf (szError, TEXT("recvfrom failed! Error: %d"), 
              WSAGetLastError ());
    MessageBox (NULL, szError, TEXT("Error"), MB_OK);
    closesocket (Sock);
    return FALSE;
  }
  else
  {
    // Convert the ASCII string to the UNICODE string.
    for (index = 0; index <= sizeof (szMessageA); index++)
      szMessageW[index] = szMessageA[index];

    MessageBox (NULL, szMessageW, TEXT("Info"), MB_OK);
  }

  // Disable receiving on Sock before closing it.
  shutdown (Sock, 0x00);

  // Close Sock.
  closesocket (Sock);

  WSACleanup ();

  return TRUE;
}