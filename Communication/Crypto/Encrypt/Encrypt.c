/***********************************************************************

THIS CODE AND INFORMATION IS PROVIDED AS IS WITHOUT WARRANTY OF ANY
KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
PURPOSE.

Copyright(c)  1999  Microsoft Corporation.  All Rights Reserved.

MODULE: 
  Encrypt.c

ABSTRACT: 
  This code sample shows how to encrypt a file. The default file names
  and password are:

  Input file:               test1.txt
  Output (encrypted) file:  test.xxx
  Password:                 password

***********************************************************************/
#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>

#define BLOCK_SIZE            1000
#define BUFFER_SIZE           1008

BOOL EncryptFile (LPTSTR, LPTSTR, LPTSTR);

/***********************************************************************

  WinMain

***********************************************************************/
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, int nCmdShow)            
{
  LPTSTR lpszSource  = TEXT("test1.txt");
  LPTSTR lpszDestination = TEXT("test.xxx");
  LPTSTR lpszPassword  = TEXT("password");
  
  if (!EncryptFile (lpszSource, lpszDestination, lpszPassword)) 
  {
    wprintf (TEXT("Error encrypting file!\n"));
    return 1;
  }

  return 0;
}

/***********************************************************************

  EncryptFile

***********************************************************************/
BOOL EncryptFile (LPTSTR lpszSource, LPTSTR lpszDestination, 
                  LPTSTR lpszPassword)
{
  FILE *hSrcFile = NULL, 
       *hDestFile = NULL;

  HCRYPTPROV hProv = 0;
  HCRYPTHASH hHash = 0;
  HCRYPTKEY hKey = 0, 
            hXchgKey = 0;

  PBYTE pbBuffer = NULL, 
        pbKeyBlob = NULL;

  BOOL bEOF = 0, 
       bReturn = FALSE;

  DWORD dwCount, 
        dwKeyBlobLen;

  // Open the source file.
  if ((hSrcFile = _wfopen (lpszSource, TEXT("rb"))) == NULL) 
  {
    wprintf (TEXT("Error opening Plaintext file!\n"));
    goto exit;
  }

  // Open the destination file.
  if ((hDestFile = _wfopen (lpszDestination, TEXT("wb"))) == NULL) 
  {
    wprintf (TEXT("Error opening Ciphertext file!\n"));
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
    // Encrypt the file with a random session key.

    // Create a random session key.
    if (!CryptGenKey (hProv, CALG_RC2, CRYPT_EXPORTABLE, &hKey)) 
    {
      wprintf (TEXT("Error %x during CryptGenKey!\n"), 
               GetLastError ());
      goto exit;
    }

    // Get the handle to the key exchange public key.
    if (!CryptGetUserKey (hProv, AT_KEYEXCHANGE, &hXchgKey)) 
    {
      wprintf (TEXT("Error %x during CryptGetUserKey!\n"), 
               GetLastError ());
      goto exit;
    }

    // Determine the size of the key blob and allocate memory.
    if (!CryptExportKey (hKey, hXchgKey, SIMPLEBLOB, 0, NULL, 
                         &dwKeyBlobLen)) 
    {
      wprintf (TEXT("Error %x computing blob length!\n"), 
               GetLastError ());
      goto exit;
    }

    if ((pbKeyBlob = malloc (dwKeyBlobLen)) == NULL) 
    {
      wprintf (TEXT("Out of memory!\n"));
      goto exit;
    }

    // Export the session key into a simple key blob.
    if (!CryptExportKey (hKey, hXchgKey, SIMPLEBLOB, 0, pbKeyBlob, 
                         &dwKeyBlobLen)) 
    {
      wprintf (TEXT("Error %x during CryptExportKey!\n"), 
               GetLastError ());
      goto exit;
    }

    // Write the size of key blob to the destination file.
    fwrite (&dwKeyBlobLen, sizeof (DWORD), 1, hDestFile);

    if (ferror (hDestFile)) 
    {
      wprintf (TEXT("Error writing header!\n"));
      goto exit;
    }

    // Write the key blob to the destination file.
    fwrite (pbKeyBlob, 1, dwKeyBlobLen, hDestFile);

    if (ferror (hDestFile)) 
    {
      wprintf (TEXT("Error writing header!\n"));
      goto exit;
    }
  } 
  else 
  {
    // Encrypt the file with a session key derived from a password.

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
  if ((pbBuffer = malloc (BUFFER_SIZE)) == NULL) 
  {
    wprintf (TEXT("Out of memory!\n"));
    goto exit;
  }

  // Encrypt the source file and write to the destination file.
  do 
  {
    // Read up to BLOCK_SIZE bytes from the source file.
    dwCount = fread (pbBuffer, 1, BLOCK_SIZE, hSrcFile);

    if (ferror (hSrcFile)) 
    {
      wprintf (TEXT("Error reading Plaintext!\n"));
      goto exit;
    }

    bEOF = feof (hSrcFile);

    // Encrypt the data.
    if (!CryptEncrypt (hKey, 0, bEOF, 0, pbBuffer, &dwCount, 
                       BUFFER_SIZE)) 
    {
      wprintf (TEXT("bytes required:%d\n"), dwCount);
      wprintf (TEXT("Error %x during CryptEncrypt!\n"), 
               GetLastError ());
      goto exit;
    }

    // Write the data to the destination file.
    fwrite (pbBuffer, 1, dwCount, hDestFile);

    if (ferror (hDestFile)) 
    {
      wprintf (TEXT("Error writing Ciphertext!\n"));
      goto exit;
    }
  } while (!bEOF);

  bReturn = TRUE;

  wprintf (TEXT("OK\n"));

exit:

   // Close the files.
  if (hSrcFile) 
    fclose (hSrcFile);

  if (hDestFile) 
    fclose (hDestFile);

  // Free memory.
  if (pbKeyBlob) 
    free (pbKeyBlob);

  if (pbBuffer) 
    free (pbBuffer);

  // Destroy the session key.
  if (hKey) 
    CryptDestroyKey (hKey);

  // Release the key exchange key handle.
  if (hXchgKey) 
    CryptDestroyKey (hXchgKey);

  // Destroy the hash object.
  if (hHash) 
    CryptDestroyHash (hHash);

  // Release the provider handle.
  if (hProv) 
    CryptReleaseContext (hProv, 0);

  return bReturn;
}



