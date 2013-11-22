#include <windows.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <httpfilt.h>
#include <httpext.h>

#define _CRT_SECURE_NO_WARNINGS
/*

BOOL GetFilterVersion(HTTP_FILTER_VERSION *pVer)
{ 
  pVer->dwFlags = (SF_NOTIFY_PREPROC_HEADERS | SF_NOTIFY_AUTHENTICATION |
             SF_NOTIFY_URL_MAP  | SF_NOTIFY_SEND_RAW_DATA | SF_NOTIFY_LOG  | SF_NOTIFY_END_OF_NET_SESSION );
 
  pVer->dwFilterVersion = HTTP_FILTER_REVISION;
 
  strcpy(pVer->lpszFilterDesc, "UMFilter ISAPI");
 
  return TRUE;
}


DWORD WINAPI __stdcall HttpFilterProc(HTTP_FILTER_CONTEXT *pfc, DWORD NotificationType, VOID *pvData)
{
            CFile myFile("G:\\mylist.html", CFile::modeWrite);
            myFile.SeekToEnd();
 
            switch (NotificationType)  {
 
            case SF_NOTIFY_URL_MAP :
             myFile.Write("SF_NOTIFY_URL_MAP",strlen("SF_NOTIFY_URL_MAP>"));
                        break;
            case SF_NOTIFY_PREPROC_HEADERS :
            myFile.Write("SF_NOTIFY_PREPROC_HEADERS",strlen("SF_NOTIFY_PREPROC_HEADERS"));
            break;
 
default :
            break;        
            }         
            myFile.Close();
            return SF_STATUS_REQ_NEXT_NOTIFICATION;
}
*/


DWORD DoSendResponse(HTTP_FILTER_CONTEXT * pfc,HTTP_FILTER_SEND_RESPONSE * pResponse);
 
BOOL WINAPI __stdcall GetFilterVersion(HTTP_FILTER_VERSION *pVer)
{
	const char *name = "UMFilter ISAPI";
	size_t len = strlen(name) + 1;

   pVer->dwFlags = ( SF_NOTIFY_ORDER_LOW | SF_NOTIFY_SEND_RESPONSE );
   pVer->dwFilterVersion = HTTP_FILTER_REVISION;
   strncpy_s(pVer->lpszFilterDesc, len, "UMFilter ISAPI", len);

   return TRUE;
}


#pragma warning(suppress: 28251)
BOOL WINAPI GetExtensionVersion(HSE_VERSION_INFO *pVer)
{
   pVer->dwExtensionVersion = HSE_VERSION;
   strncpy_s(pVer->lpszExtensionDesc, HSE_MAX_EXT_DLL_NAME_LEN, 
      "UMFilter ISAPI Extension", _TRUNCATE);
   return TRUE;
}
 

DWORD WINAPI __stdcall HttpFilterProc(HTTP_FILTER_CONTEXT *pfc, DWORD NotificationType, VOID *pvData)
{
   switch (NotificationType)
   {
   case SF_NOTIFY_SEND_RESPONSE :
         return DoSendResponse(pfc, (HTTP_FILTER_SEND_RESPONSE *) pvData);
   default :
         break;
   }
   return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

 
DWORD DoSendResponse(HTTP_FILTER_CONTEXT * pfc,HTTP_FILTER_SEND_RESPONSE * pResponse)
{
   BOOL fServer = TRUE;
   DWORD dwServerError;
   fServer = pResponse->SetHeader(pfc, "UMFilter:", "Enabled");
   if ( !fServer )
   {
         dwServerError = GetLastError();
         pfc->pFilterContext = (LPVOID)(DWORD64)pResponse->HttpStatus;
   }
   return SF_STATUS_REQ_NEXT_NOTIFICATION;
}

void WriteContext(EXTENSION_CONTROL_BLOCK *pECB, char *pszFormat, ...)
{
   char szBuffer[1024];
   va_list arg_ptr;
   va_start(arg_ptr, pszFormat); 
   vsprintf_s(szBuffer, 1024, pszFormat, arg_ptr);
   va_end(arg_ptr);

   DWORD dwSize = strlen(szBuffer);
   pECB->WriteClient(pECB->ConnID, szBuffer, &dwSize, 0);
}

BOOL ReadContext(EXTENSION_CONTROL_BLOCK *pECB, LPVOID buffer, DWORD bufferSize, LPDWORD length)
{
	BOOL result;

	*length = bufferSize;

	result = pECB->ReadClient(pECB->ConnID, buffer, length);

	WCHAR num[12];

	// converts number to string

	//_itow_s(connectionId, num, sizeof(num), 10);

	_ultow_s(*length, num, 12, 10);

	//OutputDebugString(TEXT("ISAPI: "));

	//OutputDebugString(num);

	//OutputDebugString(TEXT(" bytes read\n"));

	return result;

}

void StartContext(EXTENSION_CONTROL_BLOCK *pECB)
{
   WriteContext(pECB, "<html>\r\n<body>\r\n");
}

void EndContext(EXTENSION_CONTROL_BLOCK *pECB)
{
   WriteContext(pECB, "</body>\r\n</html>");
}

#pragma warning(suppress: 28251)
DWORD WINAPI HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB) 
{
   StartContext(pECB);

   WriteContext(pECB, "<pre>");
   WriteContext(pECB, "%s", pECB->lpbData);

   if (pECB->cbAvailable > pECB->cbTotalBytes) {
	   // TODO REad buffer
	   //pECB->ReadClient
   }
   WriteContext(pECB, "</pre>");

   EndContext(pECB);

   return HSE_STATUS_SUCCESS;
}
