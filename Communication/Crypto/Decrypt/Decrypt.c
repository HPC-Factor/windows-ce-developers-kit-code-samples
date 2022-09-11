/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Decrypt.c

ABSTRACT: 
  This code sample shows how to decrypt a file. The default file names
  and password are:

  Input (encrypted) file:   test.xxx
  Output (decrypted) file:  test2.txt
  Password:                 password

***********************************************************************/
#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>
   
#define BLOCK_SIZE            1000
#define BUFFER_SIZE           1008

BOOL DecryptFile (LPTSTR, LPTSTR, LPTSTR);

/***********************************************************************

  WinMain

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, int nCmdShow)                   
{
  LPTSTR lpszSource  = TEXT("test.xxx");
  LPTSTR lpszDestination = TEXT("test2.txt");
  LPTSTR lpszPassword  = TEXT("password");
 
  if (!DecryptFile (lpszSource, lpszDestination, lpszPassword)) 
  {
    wprintf (TEXT("Error encrypting file!\n"));
    return 1;
  }

  return 0;
}

/***********************************************************************

  DecryptFile

***********************************************************************/
BOOL DecryptFile (LPTSTR lpszSource, LPTSTR lpszDestination, 
                  LPTSTR lpszPassword)
{
  FILE *hSrcFile = NULL, 
       *hDestFile = NULL;

  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HCRYPTKEY hKey = 0;

  PBYTE pbBuffer = NULL, 
        pbKeyBlob = NULL;

  BOOL bEOF = 0, 
       bReturn = FALSE;

  DWORD dwCount, 
        dwKeyBlobLen;
  
  // Open the source file.
  if ((hSrcFile = _wfopen (lpszSource, TEXT("rb"))) == NULL) 
  {
    wprintf (TEXT("Error opening Ciphertext file!\n"));
    goto exit;
  }

  // Open the destination file.
  if ((hDestFile = _wfopen (lpszDestination, TEXT("wb"))) == NULL) 
  {
    wprintf (TEXT("Error opening Plaintext file!\n"));
    goto exit;
  }

  // Get the handle to the default provider.
  if (!CryptAcquireContext (&hProv, NULL, NULL, PROV_RSA_FULL, 0)) 
  {
    wprintf (TEXT("Error %x during CryptAcquireContext!\n"), 
             GetLastError ());
    goto exit;
  }

  if (lpszPassword == NULL) 
  {
    // Decrypt the file with the saved session key.

    // Read key blob length from the source file and allocate memory.
    fread (&dwKeyBlobLen, sizeof (DWORD), 1, hSrcFile);

    if (ferror (hSrcFile) || feof (hSrcFile)) 
    {
      wprintf (TEXT("Error reading file header!\n"));
      goto exit;
    }

    if ((pbKeyBlob = (PBYTE)malloc (dwKeyBlobLen)) == NULL) 
    {
      wprintf (TEXT("Out of memory or improperly formatted source ")
               TEXT("file!\n"));
      goto exit;
    }

    // Read the key blob from source file.
    fread (pbKeyBlob, 1, dwKeyBlobLen, hSrcFile);

    if (ferror (hSrcFile) || feof (hSrcFile)) 
    {
      wprintf (TEXT("Error reading file header!\n"));
      goto exit;
    }

    // Import the key blob into the CSP.
    if (!CryptImportKey (hProv, pbKeyBlob, dwKeyBlobLen, 0, 0, &hKey)) 
    {
      wprintf (TEXT("Error %x during CryptImportKey!\n"), 
               GetLastError ());
      goto exit;
    }
  } 
  else
  {
    // Decrypt the file with a session key derived from a password.

    // Create a hash object.
    if (!CryptCreateHash (hProv, CALG_MD5, 0, 0, &hHash)) 
    {
      wprintf (TEXT("Error %x during CryptCreateHash!\n"), 
               GetLastError ());
      goto exit;
    }

    // Hash in the password data.
    if (!CryptHashData (hHash, (PBYTE)lpszPassword, 
                        wcslen (lpszPassword), 0)) 
    {
      wprintf (TEXT("Error %x during CryptHashData!\n"), 
               GetLastError ());
      goto exit;
    }

    // Derive a session key from the hash object.
    if (!CryptDeriveKey (hProv, CALG_RC2, hHash, 0, &hKey)) 
    {
      wprintf (TEXT("Error %x during CryptDeriveKey!\n"), 
               GetLastError ());
      goto exit;
    }
  }

  // Allocate memory.
  if ((pbBuffer = (PBYTE)malloc (BUFFER_SIZE)) == NULL) 
  {
    wprintf (TEXT("Out of memory!\n"));
    goto exit;
  }

  // Decrypt the source file and write to the destination file.
  do 
  {
    // Read up to BLOCK_SIZE bytes from the source file.
    dwCount = fread (pbBuffer, 1, BLOCK_SIZE, hSrcFile);

    if (ferror (hSrcFile)) 
    {
      wprintf (TEXT("Error reading Ciphertext!\n"));
      goto exit;
    }

    bEOF = feof (hSrcFile);

    // Decrypt the data.
    if (!CryptDecrypt (hKey, 0, bEOF, 0, pbBuffer, &dwCount)) 
    {
      wprintf (TEXT("Error %x during CryptDecrypt!\n"), 
               GetLastError ());
      goto exit;
    }

    // Write the data to the destination file.
    fwrite (pbBuffer, 1, dwCount, hDestFile);

    if (ferror (hDestFile)) 
    {
      wprintf (TEXT("Error writing Plaintext!\n"));
      goto exit;
    }
  } while (!bEOF);

  bReturn = TRUE;

  wprintf (TEXT("OK\n"));

exit:

  // Close the source files.
  if (hSrcFile) 
    fclose (hSrcFile);

  // Close the destination files.
  if (hDestFile) 
    fclose (hDestFile);

  // Free memory.
  if (pbKeyBlob) 
    free (pbKeyBlob);

  // Free memory.
  if (pbBuffer) 
    free (pbBuffer);

  // Destroy the session key.
  if (hKey) 
    CryptDestroyKey (hKey);

  // Destroy the hash object.
  if (hHash) 
    CryptDestroyHash (hHash);

  // Release the provider handle.
  if (hProv) 
    CryptReleaseContext (hProv, 0);

  return bReturn;
}
