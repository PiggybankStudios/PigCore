/*
File:   os_http.h
Author: Taylor Robbins
Date:   07\24\2025
Description:
	** Wraps the API for making HTTP requests (including SSL, aka HTTPS) for each OS (WinHTTP on Windows, ? on Linux, etc.)
*/

#ifndef _OS_HTTP_H
#define _OS_HTTP_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "std/std_memset.h"
#include "std/std_includes.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "os/os_threading.h"
#include "misc/misc_profiling_tracy_include.h"

#if BUILD_WITH_HTTP

#define HTTP_DEFAULT_CLIENT_WIDE_STR L"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0" //TODO: Make this a different client string?
#define HTTP_PORT  80
#define HTTPS_PORT 443

typedef enum HttpVerb HttpVerb;
enum HttpVerb
{
	HttpVerb_None = 0,
	HttpVerb_GET,
	HttpVerb_POST,
	HttpVerb_DELETE,
	HttpVerb_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetHttpVerbStr(HttpVerb enumValue);
#else
PEXP const char* GetHttpVerbStr(HttpVerb enumValue)
{
	switch (enumValue)
	{
		case HttpVerb_None:   return "None";
		case HttpVerb_GET:    return "GET";
		case HttpVerb_POST:   return "POST";
		case HttpVerb_DELETE: return "DELETE";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex HttpRequestArgs HttpRequestArgs;
plex HttpRequestArgs
{
	HttpVerb verb;
	Str8 urlStr;
	uxx numHeaders;
	Str8Pair* headers;
	uxx numContentItems;
	Str8Pair* contentItems;
};

typedef enum HttpRequestState HttpRequestState;
enum HttpRequestState
{
	HttpRequestState_None = 0,
	HttpRequestState_NotStarted,
	HttpRequestState_InProgress,
	HttpRequestState_Success,
	HttpRequestState_Failure,
	HttpRequestState_Cancelled,
	HttpRequestState_Count,
};
#if !PIG_CORE_IMPLEMENTATION
const char* GetHttpRequestStateStr(HttpRequestState enumValue);
#else
PEXP const char* GetHttpRequestStateStr(HttpRequestState enumValue)
{
	switch (enumValue)
	{
		case HttpRequestState_None:       return "None";
		case HttpRequestState_NotStarted: return "NotStarted";
		case HttpRequestState_InProgress: return "InProgress";
		case HttpRequestState_Success:    return "Success";
		case HttpRequestState_Failure:    return "Failure";
		case HttpRequestState_Cancelled:  return "Cancelled";
		default: return UNKNOWN_STR;
	}
}
#endif

typedef plex HttpRequest HttpRequest;
plex HttpRequest
{
	u8 magic[4];
	uxx id;
	HttpRequestState state;
	Result error;
	HttpRequestArgs args;
	
	bool receivingData;
	bool queriedData;
	
	Str8 protocolStr;
	Str8 hostnameStr;
	Str8 pathStr;
	uxx connectionIndex;
	
	Str8 encodedContent;
	Str8 encodedHeaders;
	#if TARGET_IS_WINDOWS
	HINTERNET requestHandle;
	uxx numStatusCallbacks;
	DWORD statusCallbacks[32];
	#endif
	
	VarArray responseBytes; //u8 (allocated from manager.readDataArena)
};

typedef plex HttpConnection HttpConnection;
plex HttpConnection
{
	u8 magic[4];
	bool usingSsl;
	Str8 hostname;
	u16 portNumber;
	u64 openTime;
	u64 lastUsedTime;
	uxx keepaliveTime;
	#if TARGET_IS_WINDOWS
	Str16 hostnameWide;
	HINTERNET handle;
	#endif
};

typedef plex HttpRequestManager HttpRequestManager;
plex HttpRequestManager
{
	u8 magic[4];
	Arena* arena;
	
	uxx nextRequestId;
	VarArray requests; //HttpRequest TODO: Should we make this a BktArray?
	uxx currentRequestIndex;
	VarArray connections; //HttpConnection
	Arena responseArena;
	
	#if TARGET_HAS_THREADING
	Mutex mutex;
	bool mainLockedMutex;
	#endif
	#if TARGET_IS_WINDOWS
	HINTERNET sessionHandle;
	#endif
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void OsCopyHttpRequestArgs(Arena* arena, HttpRequestArgs* dest, const HttpRequestArgs* source);
	PIG_CORE_INLINE void OsFreeHttpRequestManager(HttpRequestManager* manager);
	PIG_CORE_INLINE void OsInitHttpRequestManager(Arena* arena, HttpRequestManager* manager);
	PIG_CORE_INLINE HttpConnection* OsFindHttpConnection(HttpRequestManager* manager, Str8 hostname, u16 portNumber);
	void OsUpdateHttpRequestManager(HttpRequestManager* manager, u64 programTime);
	HttpRequest* OsMakeHttpRequest(HttpRequestManager* manager, const HttpRequestArgs* args, u64 programTime);
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void OsCopyHttpRequestArgs(Arena* arena, HttpRequestArgs* dest, const HttpRequestArgs* source)
{
	NotNull(arena);
	NotNull(dest);
	NotNull(source);
	MyMemCopy(dest, source, sizeof(HttpRequestArgs));
	dest->urlStr = AllocStr8(arena, dest->urlStr);
	if (dest->numHeaders > 0)
	{
		dest->headers = AllocArray(Str8Pair, arena, dest->numHeaders);
		NotNull(dest->headers);
		for (uxx hIndex = 0; hIndex < dest->numHeaders; hIndex++)
		{
			dest->headers[hIndex].key = AllocStr8(arena, source->headers[hIndex].key);
			dest->headers[hIndex].value = AllocStr8(arena, source->headers[hIndex].value);
		}
	}
	if (dest->numContentItems > 0)
	{
		dest->contentItems = AllocArray(Str8Pair, arena, dest->numContentItems);
		NotNull(dest->contentItems);
		for (uxx cIndex = 0; cIndex < dest->numContentItems; cIndex++)
		{
			dest->contentItems[cIndex].key = AllocStr8(arena, source->contentItems[cIndex].key);
			dest->contentItems[cIndex].value = AllocStr8(arena, source->contentItems[cIndex].value);
		}
	}
}

static void FreeHttpRequest(Arena* arena, HttpRequest* request)
{
	NotNull(request);
	NotNull(arena);
	FreeStr8(arena, &request->args.urlStr);
	for (uxx hIndex = 0; hIndex < request->args.numHeaders; hIndex++)
	{
		FreeStr8(arena, &request->args.headers[hIndex].key);
		FreeStr8(arena, &request->args.headers[hIndex].value);
	}
	if (request->args.headers != nullptr) { FreeArray(Str8Pair, arena, request->args.numHeaders, request->args.headers); }
	for (uxx cIndex = 0; cIndex < request->args.numContentItems; cIndex++)
	{
		FreeStr8(arena, &request->args.contentItems[cIndex].key);
		FreeStr8(arena, &request->args.contentItems[cIndex].value);
	}
	if (request->args.contentItems != nullptr) { FreeArray(Str8Pair, arena, request->args.numContentItems, request->args.contentItems); }
	ClearPointer(request);
}

PEXPI void OsFreeHttpRequestManager(HttpRequestManager* manager)
{
	NotNull(manager);
	if (manager->arena != nullptr)
	{
		Assert(manager->currentRequestIndex >= manager->requests.length);
		VarArrayLoop(&manager->requests, rIndex)
		{
			VarArrayLoopGet(HttpRequest, request, &manager->requests, rIndex);
			Assert(request->state != HttpRequestState_InProgress); //TODO: Maybe we can cancel the request rather than asserting?
			FreeHttpRequest(manager->arena, request);
		}
		FreeVarArray(&manager->requests);
		#if TARGET_HAS_THREADING
		DestroyMutex(&manager->mutex);
		#endif
		#if TARGET_IS_WINDOWS
		BOOL closeResult = WinHttpCloseHandle(manager->sessionHandle);
		Assert(closeResult == TRUE);
		UNUSED(closeResult);
		#endif
	}
	ClearPointer(manager);
}

#if TARGET_IS_WINDOWS

static const char* GetWinHttpStatusStr(DWORD status)
{
	switch (status)
	{
		case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:      return "CLOSING_CONNECTION";
		case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:     return "CONNECTED_TO_SERVER";
		case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:    return "CONNECTING_TO_SERVER";
		case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:       return "CONNECTION_CLOSED";
		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:          return "DATA_AVAILABLE";
		case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:          return "HANDLE_CREATED";
		case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:          return "HANDLE_CLOSING";
		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:       return "HEADERS_AVAILABLE";
		case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:   return "INTERMEDIATE_RESPONSE";
		case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:           return "NAME_RESOLVED";
		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:           return "READ_COMPLETE";
		case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:      return "RECEIVING_RESPONSE";
		case WINHTTP_CALLBACK_STATUS_REDIRECT:                return "REDIRECT";
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:           return "REQUEST_ERROR";
		case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:            return "REQUEST_SENT";
		case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:          return "RESOLVING_NAME";
		case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:       return "RESPONSE_RECEIVED";
		case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:          return "SECURE_FAILURE";
		case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:         return "SENDING_REQUEST";
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:    return "SENDREQUEST_COMPLETE";
		case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:          return "WRITE_COMPLETE";
		case WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE: return "GETPROXYFORURL_COMPLETE";
		case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:          return "CLOSE_COMPLETE";
		case WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE:       return "SHUTDOWN_COMPLETE";
		default: return UNKNOWN_STR;
	};
}

static const char* GetWinHttpErrorStr(DWORD errorCode)
{
	switch (errorCode)
	{
		case ERROR_WINHTTP_CANNOT_CONNECT: return "ERROR_WINHTTP_CANNOT_CONNECT";
		case ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW: return "ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW";
		case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: return "ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED";
		case ERROR_WINHTTP_CONNECTION_ERROR: return "ERROR_WINHTTP_CONNECTION_ERROR";
		case ERROR_WINHTTP_HEADER_COUNT_EXCEEDED: return "ERROR_WINHTTP_HEADER_COUNT_EXCEEDED";
		case ERROR_WINHTTP_HEADER_SIZE_OVERFLOW: return "ERROR_WINHTTP_HEADER_SIZE_OVERFLOW";
		case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: return "ERROR_WINHTTP_INCORRECT_HANDLE_STATE";
		case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: return "ERROR_WINHTTP_INCORRECT_HANDLE_TYPE";
		case ERROR_WINHTTP_INTERNAL_ERROR: return "ERROR_WINHTTP_INTERNAL_ERROR";
		case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: return "ERROR_WINHTTP_INVALID_SERVER_RESPONSE";
		case ERROR_WINHTTP_INVALID_URL: return "ERROR_WINHTTP_INVALID_URL";
		case ERROR_WINHTTP_LOGIN_FAILURE: return "ERROR_WINHTTP_LOGIN_FAILURE";
		case ERROR_WINHTTP_NAME_NOT_RESOLVED: return "ERROR_WINHTTP_NAME_NOT_RESOLVED";
		case ERROR_WINHTTP_OPERATION_CANCELLED: return "ERROR_WINHTTP_OPERATION_CANCELLED";
		case ERROR_WINHTTP_REDIRECT_FAILED: return "ERROR_WINHTTP_REDIRECT_FAILED";
		case ERROR_WINHTTP_RESEND_REQUEST: return "ERROR_WINHTTP_RESEND_REQUEST";
		case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: return "ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW";
		case ERROR_WINHTTP_SECURE_FAILURE: return "ERROR_WINHTTP_SECURE_FAILURE";
		case ERROR_WINHTTP_TIMEOUT: return "ERROR_WINHTTP_TIMEOUT";
		case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: return "ERROR_WINHTTP_UNRECOGNIZED_SCHEME";
		case ERROR_NOT_ENOUGH_MEMORY: return "ERROR_NOT_ENOUGH_MEMORY";
		default: return UNKNOWN_STR;
	}
}


static void WinHttpStatusCallback(HINTERNET handle, DWORD_PTR context, DWORD status, LPVOID infoPntr, DWORD infoLength)
{
	TracyCZoneN(Zone_Func, "WinHttpStatusCallback", true);
	char printBuffer[128];
	
	ThreadId threadId = OsGetCurrentThreadId();
	bool isMainThread = (threadId == MainThreadId);
	{
		if (isMainThread)
		{
			PrintLine_D("MAIN> HttpStatus %s(%04X): %p[%u] (%p,%p)", GetWinHttpStatusStr(status), status, infoPntr, infoLength, handle, (void*)context);
		}
		else
		{
			MyBufferPrintf(printBuffer, ArrayCount(printBuffer), "%llu> HttpStatus %s(%04X): %p[%u] (%p,%p)", (u64)threadId, GetWinHttpStatusStr(status), status, infoPntr, infoLength, handle, (void*)context);
			printBuffer[ArrayCount(printBuffer)-1] = '\0';
			WriteLine_D(printBuffer);
		}
	}
	if (status == WINHTTP_CALLBACK_STATUS_HANDLE_CREATED || status == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
	{
		//NOTE: HANDLE_CREATED callbacks happen before we have a chance to set the WINHTTP_OPTION_CONTEXT_VALUE option
		//      Since we don't have a contextPntr we can't do much right now
		//NOTE: HANDLE_CLOSING callbacks can happen on handles that don't have a contextPntr set
		// if (isMainThread) { PrintLine_D("%s handle %016X (on thread %llu)", status == WINHTTP_CALLBACK_STATUS_HANDLE_CREATED ? "Created" : "Closed", handle, (u64)threadId); }
		// else { WriteLine_D("Created/Closed handle on secondary thread"); }
		return;
	}
	
	#if 1
	Assert(context != 0);
	HttpRequestManager* manager = (HttpRequestManager*)context;
	#else
	UNUSED(context);
	void* contextPntr = nullptr;
	DWORD contextPntrSize = sizeof(void*);
	WinHttpQueryOption(handle, WINHTTP_OPTION_CONTEXT_VALUE, (void*)&contextPntr, &contextPntrSize);
	DebugAssert(contextPntrSize == sizeof(void*));
	NotNull(contextPntr);
	HttpRequestManager* manager = (HttpRequestManager*)contextPntr;
	#endif
	DebugAssert(manager->magic[0] == 'M' && manager->magic[1] == 'N' && manager->magic[2] == 'G' && manager->magic[3] == 'R');
	
	bool lockedMutex = false;
	if (!isMainThread || !manager->mainLockedMutex) { LockMutex(&manager->mutex, TIMEOUT_FOREVER); lockedMutex = true; }
	
	HttpRequest* currentRequest = nullptr;
	if (manager->currentRequestIndex < manager->requests.length)
	{
		currentRequest = VarArrayGet(HttpRequest, &manager->requests, manager->currentRequestIndex);
		if (currentRequest->requestHandle == handle)
		{
			if (currentRequest->numStatusCallbacks < ArrayCount(currentRequest->statusCallbacks))
			{
				currentRequest->statusCallbacks[currentRequest->numStatusCallbacks] = status;
				currentRequest->numStatusCallbacks++;
			}
			else { WriteLine_W("Exceeded max statusCallback buffer size!"); }
		}
	}
	
	switch (status)
	{
		// +==============================================+
		// | WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE |
		// +==============================================+
		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: //NOT main thread
		{
			NotNull(currentRequest);
			Assert(handle == currentRequest->requestHandle);
			BOOL receiveResult = WinHttpReceiveResponse(currentRequest->requestHandle, NULL);
			Assert(receiveResult != 0);
			UNUSED(receiveResult);
		} break;
		
		// +============================================+
		// | WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE |
		// +============================================+
		case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE: //NOT main thread
		{
			//NOTE: We can't call WinHttpQueryDataAvailable right now, we get a INCORRECT_HANDLE_STATE error if we try
			//      So instead we will wait till the main thread is free to query data, at that point the query handle should be available for requesting data
			NotNull(currentRequest);
			Assert(handle == currentRequest->requestHandle);
			currentRequest->receivingData = true;
		} break;
		
		// +========================================+
		// | WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE |
		// +========================================+
		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: //NOT main thread
		{
			NotNull(currentRequest);
			Assert(handle == currentRequest->requestHandle);
			
			DWORD numBytesToRead = infoLength;
			
			MyBufferPrintf(printBuffer, ArrayCount(printBuffer), "%llu byte%s available", (u64)numBytesToRead, Plural(numBytesToRead, "s"));
			printBuffer[ArrayCount(printBuffer)-1] = '\0';
			WriteLine_D(printBuffer);
			
			if (numBytesToRead > 0)
			{
				if (currentRequest->responseBytes.arena == nullptr)
				{
					InitVarArray(u8, &currentRequest->responseBytes, &manager->responseArena);
				}
				u8* newBytesSpace = VarArrayAddMulti(u8, &currentRequest->responseBytes, (uxx)numBytesToRead);
				NotNull(newBytesSpace);
				Assert(handle == currentRequest->requestHandle);
				DWORD numBytesRead = 0;
				BOOL readResult = WinHttpReadData(
					currentRequest->requestHandle, //hRequest
					newBytesSpace, //lpBuffer
					numBytesToRead, //dwNumberOfBytesToRead
					&numBytesRead //lpdwNumberOfBytesRead
				);
				Assert(readResult != 0);
				UNUSED(readResult);
				if (numBytesRead < numBytesToRead)
				{
					currentRequest->responseBytes.length -= (numBytesToRead - numBytesRead);
				}
				
				MyBufferPrintf(printBuffer, ArrayCount(printBuffer), "Read %llu byte%s, total %llu byte%s", (u64)numBytesRead, Plural(numBytesRead, "s"), currentRequest->responseBytes.length, Plural(currentRequest->responseBytes.length, "s"));
				printBuffer[ArrayCount(printBuffer)-1] = '\0';
				WriteLine_D(printBuffer);
			}
			currentRequest->queriedData = false;
		} break;
		
		// +========================================+
		// | WINHTTP_CALLBACK_STATUS_READ_COMPLETE  |
		// +========================================+
		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: //NOT main thread
		{
			NotNull(currentRequest);
			Assert(handle == currentRequest->requestHandle);
			Assert(currentRequest->receivingData);
			currentRequest->receivingData = false;
			currentRequest->state = HttpRequestState_Success;
			manager->currentRequestIndex = UINTXX_MAX;
		} break;
		
		// +==================================+
		// | WINHTTP_CALLBACK_STATUS_REDIRECT |
		// +==================================+
		case WINHTTP_CALLBACK_STATUS_REDIRECT: //NOT main thread
		{
			NotNull(currentRequest);
			Assert(handle == currentRequest->requestHandle);
			currentRequest->receivingData = false;
		} break;
		
		// case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: //NOT main thread
		// {
		// 	//TODO: Implement me!
		// } break;
		
		// case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE: //NOT main thread
		// {
		// 	//TODO: Implement me!
		// } break;
		
		// case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE: //main thread
		// {
		// 	//TODO: Implement me!
		// } break;
		
		// default:
		// {
		// 	if (status != WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING &&
		// 		status != WINHTTP_CALLBACK_STATUS_HANDLE_CREATED &&
		// 		status != WINHTTP_CALLBACK_STATUS_SENDING_REQUEST &&
		// 		status != WINHTTP_CALLBACK_STATUS_REQUEST_SENT &&
		// 		status != WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED &&
		// 		status != WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE &&
		// 		status != WINHTTP_CALLBACK_STATUS_RESOLVING_NAME &&
		// 		status != WINHTTP_CALLBACK_STATUS_NAME_RESOLVED &&
		// 		status != WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER &&
		// 		status != WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER &&
		// 		status != WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION)
		// 	{
		// 		MyBufferPrintf(printBuffer, ArrayCount(printBuffer), "HttpStatus %s(%04X): %p[%u] (%p,%p)", GetWinHttpStatusStr(status), status, infoPntr, infoLength, handle, (void*)context);
		// 		printBuffer[ArrayCount(printBuffer)-1] = '\0';
		// 		WriteLine_D(printBuffer);
		// 	}
		// } break;
	}
	
	if (lockedMutex) { UnlockMutex(&manager->mutex); }
	TracyCZoneEnd(Zone_Func);
}

#endif //TARGET_IS_WINDOWS

PEXPI void OsInitHttpRequestManager(Arena* arena, HttpRequestManager* manager)
{
	TracyCZoneN(Zone_Func, "OsInitHttpRequestManager", true);
	
	NotNull(arena);
	NotNull(manager);
	ClearPointer(manager);
	manager->magic[0] = 'M'; manager->magic[1] = 'N'; manager->magic[2] = 'G'; manager->magic[3] = 'R';
	manager->arena = arena;
	manager->nextRequestId = 1;
	manager->currentRequestIndex = UINTXX_MAX;
	InitVarArray(HttpRequest, &manager->requests, arena);
	InitVarArray(HttpConnection, &manager->connections, arena);
	#if TARGET_HAS_THREADING
	DebugAssert(MainThreadId != 0);
	PrintLine_D("Main Thread ID: %llu", (u64)MainThreadId);
	InitMutex(&manager->mutex);
	#endif
	
	InitArenaStackVirtual(&manager->responseArena, Megabytes(64));
	
	#if TARGET_IS_WINDOWS
	{
		manager->sessionHandle = WinHttpOpen(HTTP_DEFAULT_CLIENT_WIDE_STR,
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			WINHTTP_FLAG_ASYNC
		);
		Assert(manager->sessionHandle != NULL);
		PrintLine_D("Session handle: %016X (manager pntr: %p)", manager->sessionHandle, manager);
		
		DWORD callbackMask = WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS |
			WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS |
			WINHTTP_CALLBACK_FLAG_RESOLVE_NAME |
			WINHTTP_CALLBACK_FLAG_CONNECT_TO_SERVER |
			WINHTTP_CALLBACK_FLAG_DETECTING_PROXY |
			WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE |
			WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE |
			WINHTTP_CALLBACK_FLAG_READ_COMPLETE |
			WINHTTP_CALLBACK_FLAG_REQUEST_ERROR |
			WINHTTP_CALLBACK_FLAG_SEND_REQUEST |
			WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE |
			WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE |
			WINHTTP_CALLBACK_FLAG_RECEIVE_RESPONSE |
			WINHTTP_CALLBACK_FLAG_CLOSE_CONNECTION |
			WINHTTP_CALLBACK_FLAG_HANDLES |
			WINHTTP_CALLBACK_FLAG_REDIRECT |
			WINHTTP_CALLBACK_FLAG_INTERMEDIATE_RESPONSE |
			WINHTTP_CALLBACK_FLAG_SECURE_FAILURE;
		
		WINHTTP_STATUS_CALLBACK setCallbackResult = WinHttpSetStatusCallback(manager->sessionHandle, WinHttpStatusCallback, callbackMask, (DWORD_PTR)0);
		Assert(setCallbackResult != WINHTTP_INVALID_STATUS_CALLBACK); //TODO: Call GetLastError for more info
		
		unsigned long securitySetting = 0;
		WinHttpSetOption(manager->sessionHandle, WINHTTP_OPTION_SECURITY_FLAGS, &securitySetting, sizeof(securitySetting));
	}
	#endif //TARGET_IS_WINDOWS
	
	TracyCZoneEnd(Zone_Func);
}

PEXPI HttpConnection* OsFindHttpConnection(HttpRequestManager* manager, Str8 hostname, u16 portNumber)
{
	NotNull(manager);
	NotNull(manager->arena);
	VarArrayLoop(&manager->connections, cIndex)
	{
		VarArrayLoopGet(HttpConnection, connection, &manager->connections, cIndex);
		if (StrExactEquals(connection->hostname, hostname) && connection->portNumber == portNumber) { return connection; }
	}
	return nullptr;
}

static bool HttpRequestManagerStartRequest(HttpRequestManager* manager, uxx requestIndex)
{
	ScratchBegin1(scratch, manager->arena);
	HttpRequest* request = VarArrayGet(HttpRequest, &manager->requests, requestIndex);
	Assert(request->state == HttpRequestState_NotStarted);
	HttpConnection* connection = VarArrayGet(HttpConnection, &manager->connections, request->connectionIndex);
	bool result = false;
	
	PrintLine_D("Starting HTTP request %llu to \"%.*s\"...", request->id, StrPrint(request->args.urlStr));
	
	Str8 verbStr = StrLit(GetHttpVerbStr(request->args.verb));
	
	#if TARGET_IS_WINDOWS
	{
		Str16 verbStrWide = ConvertUtf8StrToUcs2(scratch, verbStr, true);
		Str16 pathStrWide = ConvertUtf8StrToUcs2(scratch, request->pathStr, true);
		PrintLine_D("Using Connection[%llu] handle: %016X", request->connectionIndex, connection->handle);
		request->requestHandle = WinHttpOpenRequest(connection->handle,
			verbStrWide.chars,
			pathStrWide.chars,
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			connection->usingSsl ? WINHTTP_FLAG_SECURE : 0
		);
		
		if (request->requestHandle == NULL)
		{
			DWORD error = GetLastError();
			PrintLine_E("Failed to open request! Error: %s (0x%04X)", GetWinHttpErrorStr(error), error);
			request->state = HttpRequestState_Failure;
			request->error = Result_FailedToOpenRequest;
		}
		else
		{
			PrintLine_D("Request handle: %016X", request->requestHandle);
			WinHttpSetOption(request->requestHandle, WINHTTP_OPTION_CONTEXT_VALUE, (void*)&manager, sizeof(void*));
			
			Str16 headersWideStr = Str16Lit(L""); // TODO: Fill this out!
			//TODO: Fill out request->encodedData
			
			//TODO: Do we need to lock the mutex here?
			
			BOOL requestResult = WinHttpSendRequest(
				request->requestHandle, //hRequest
				headersWideStr.chars, //lpszHeaders
				(DWORD)-1, //dwHeadersLength
				request->encodedContent.chars, //lpOptional
				(DWORD)request->encodedContent.length, //dwOptionalLength
				(DWORD)request->encodedContent.length, //dwTotalLength
				0 //dwContext
			);
			Assert(requestResult == TRUE); //TODO: Handle this error properly
			
			request->state = HttpRequestState_InProgress;
			manager->currentRequestIndex = requestIndex;
			result = true;
		}
	}
	#else
	AssertMsg(false, "HttpRequestManagerStartRequest does not support the current platform yet!");
	#endif
	
	ScratchEnd(scratch);
	return result;
}

PEXP void OsUpdateHttpRequestManager(HttpRequestManager* manager, u64 programTime)
{
	NotNull(manager);
	NotNull(manager->arena);
	TracyCZoneN(Zone_Func, "OsUpdateHttpRequestManager", true);
	
	uxx doCallbackIndex = UINTXX_MAX;
	LockMutex(&manager->mutex, TIMEOUT_FOREVER);
	manager->mainLockedMutex = true;
	{
		if (manager->currentRequestIndex < manager->requests.length)
		{
			HttpRequest* currentRequest = VarArrayGet(HttpRequest, &manager->requests, manager->currentRequestIndex);
			if (currentRequest->state == HttpRequestState_Success ||
				currentRequest->state == HttpRequestState_Failure ||
				currentRequest->state == HttpRequestState_Cancelled)
			{
				doCallbackIndex = manager->currentRequestIndex;
				manager->currentRequestIndex = UINTXX_MAX;
			}
			else if (currentRequest->state == HttpRequestState_InProgress)
			{
				if (currentRequest->receivingData && !currentRequest->queriedData)
				{
					WriteLine_D("Querying data...");
					currentRequest->queriedData = true;
					BOOL queryResult = WinHttpQueryDataAvailable(currentRequest->requestHandle, NULL);
					if (queryResult != TRUE)
					{
						currentRequest->queriedData = false;
						currentRequest->state = HttpRequestState_Failure;
						currentRequest->error = Result_Failure; //TODO: Choose a better error code
						DWORD errorCode = GetLastError();
						PrintLine_D("QueryData Failed: %s", GetWinHttpErrorStr(errorCode));
					}
					UNUSED(queryResult);
				}
			}
		}
	}
	manager->mainLockedMutex = false;
	UnlockMutex(&manager->mutex);
	
	if (doCallbackIndex < manager->requests.length)
	{
		//TODO: Implement me!
	}
	
	while (manager->currentRequestIndex >= manager->requests.length)
	{
		bool foundPendingRequest = false;
		VarArrayLoop(&manager->requests, rIndex)
		{
			VarArrayLoopGet(HttpRequest, request, &manager->requests, rIndex);
			if (request->state == HttpRequestState_NotStarted)
			{
				LockMutex(&manager->mutex, TIMEOUT_FOREVER);
				manager->mainLockedMutex = true;
				
				bool startedSuccessfully = HttpRequestManagerStartRequest(manager, rIndex);
				
				manager->mainLockedMutex = false;
				UnlockMutex(&manager->mutex);
				
				if (!startedSuccessfully)
				{
					//TODO: Implement me! (Do callback)
				}
				
				foundPendingRequest = true;
				break;
			}
		}
		if (!foundPendingRequest) { break; }
	}
	
	TracyCZoneEnd(Zone_Func);
}

PEXP HttpRequest* OsMakeHttpRequest(HttpRequestManager* manager, const HttpRequestArgs* args, u64 programTime)
{
	NotNull(manager);
	NotNull(manager->arena);
	Assert(!IsEmptyStr(args->urlStr));
	ScratchBegin1(scratch, manager->arena);
	
	HttpRequest* newRequest = nullptr;
	VarArrayLoop(&manager->requests, rIndex)
	{
		VarArrayLoopGet(HttpRequest, request, &manager->requests, rIndex);
		if (request->state == HttpRequestState_None) { newRequest = request; break; }
	}
	if (newRequest == nullptr)
	{
		LockMutex(&manager->mutex, TIMEOUT_FOREVER);
		manager->mainLockedMutex = true;
		
		newRequest = VarArrayAdd(HttpRequest, &manager->requests);
		NotNull(newRequest);
		
		manager->mainLockedMutex = false;
		UnlockMutex(&manager->mutex);
	}
	
	ClearPointer(newRequest);
	newRequest->magic[0] = 'R'; newRequest->magic[1] = 'E'; newRequest->magic[2] = 'Q'; newRequest->magic[3] = 'U';
	newRequest->id = manager->nextRequestId;
	manager->nextRequestId++;
	OsCopyHttpRequestArgs(manager->arena, &newRequest->args, args);
	newRequest->state = HttpRequestState_NotStarted;
	newRequest->protocolStr = GetUrlProtocolPart(newRequest->args.urlStr);
	if (IsEmptyStr(newRequest->protocolStr)) { newRequest->protocolStr = StrLit("http"); }
	Assert(StrExactEquals(newRequest->protocolStr, StrLit("http")) || StrExactEquals(newRequest->protocolStr, StrLit("https")));
	newRequest->hostnameStr = GetUrlHostnamePart(newRequest->args.urlStr);
	Assert(!IsEmptyStr(newRequest->hostnameStr));
	newRequest->pathStr = GetUrlPathPart(newRequest->args.urlStr);
	if (IsEmptyStr(newRequest->pathStr)) { newRequest->pathStr = StrLit("/"); }
	
	// TODO: Str8 encodedContent;
	// TODO: Str8 encodedHeaders;
	
	bool usingSsl = StrExactEquals(newRequest->protocolStr, StrLit("https"));
	u16 portNumber = usingSsl ? HTTPS_PORT : HTTP_PORT;
	
	HttpConnection* connection = OsFindHttpConnection(manager, newRequest->hostnameStr, portNumber);
	if (connection == nullptr)
	{
		HttpConnection* newConnection = VarArrayAdd(HttpConnection, &manager->connections);
		ClearPointer(newConnection);
		newConnection->magic[0] = 'C'; newConnection->magic[1] = 'O'; newConnection->magic[2] = 'N'; newConnection->magic[3] = 'N';
		newConnection->hostname = AllocStr8(manager->arena, newRequest->hostnameStr);
		newConnection->portNumber = portNumber;
		newConnection->usingSsl = usingSsl;
		newConnection->openTime = programTime;
		newConnection->lastUsedTime = programTime;
		newConnection->keepaliveTime = Thousand(10); //TODO: What is this value?
		PrintLine_D("[%llu] Connecting to %.*s%s...", manager->connections.length-1, StrPrint(newConnection->hostname), usingSsl ? " using SSL" : "");
		#if TARGET_IS_WINDOWS
		newConnection->hostnameWide = ConvertUtf8StrToUcs2(manager->arena, newConnection->hostname, true);
		NotNull(newConnection->hostnameWide.chars);
		newConnection->handle = WinHttpConnect(
			manager->sessionHandle, //hSession
			newConnection->hostnameWide.chars, //pswzServerName
			portNumber, //nServerPort
			0 //dwReserved
		);
		Assert(newConnection->handle != NULL); //TODO: Is this a valid error scenario we should handle?
		PrintLine_D("Connection Handle: %016X (pntr %p)", newConnection->handle, newConnection);
		#endif
		connection = newConnection;
	}
	VarArrayGetIndexOf(HttpConnection, &manager->connections, connection, &newRequest->connectionIndex);
	connection->lastUsedTime = programTime;
	
	PrintLine_D("Request to %.*s %.*s%s created (connection %llu, pntr %p)", StrPrint(newRequest->hostnameStr), StrPrint(newRequest->pathStr), usingSsl ? " using SLL" : "", newRequest->connectionIndex, newRequest);
	//TODO: can we do WinHttpOpenRequest when another request is potentially active?
	
	//TODO: Should we call OsUpdateHttpRequestManager directly?
	
	ScratchEnd(scratch);
	return newRequest;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_HTTP

#endif //  _OS_HTTP_H
