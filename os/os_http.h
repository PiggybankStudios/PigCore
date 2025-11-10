/*
File:   os_http.h
Author: Taylor Robbins
Date:   07\24\2025
Description:
	** Wraps the API for making HTTP requests (including TLS\SSL, aka HTTPS) for each OS (WinHTTP on Windows, ? on Linux, etc.)
References:
	RFC 7230: "Hypertext Transfer Protocol (HTTP/1.1): Message Syntax and Routing" (https://datatracker.ietf.org/doc/html/rfc7230)
	RFC 3986: "Uniform Resource Identifier (URI): Generic Syntax" (https://datatracker.ietf.org/doc/html/rfc3986)
		** 2.1: For consistency, URI producers and normalizers should use uppercase hexadecimal digits for all percent-encodings.
		** 2.2: Reserved characters = ':' '/' '?' '#' '[' ']' '@' '!' '$' '&' '\'' '(' ')' '*' '+' ',' ';' '='
		** 2.3: Unreserved characters = ALPHA DIGIT '-' '.' '_' '~'
		** 3.0: The generic URI syntax consists of a hierarchical sequence of components referred to as the scheme, authority, path, query, and fragment.
		**       foo://example.com:8042/over/there?name=ferret#nose
		**       \_/   \______________/\_________/ \_________/ \__/
		**        |           |            |            |        |
		**     scheme     authority       path        query   fragment
	RFC 1866: Hypertext Markup Language - 2.0 (Obseleted by 2854) (https://datatracker.ietf.org/doc/html/rfc1866)
	RFC 2854: The 'text/html' Media Type (https://datatracker.ietf.org/doc/html/rfc2854)
	RFC 2045:  Multipurpose Internet Mail Extensions (MIME) Part One: Format of Internet Message Bodies (https://datatracker.ietf.org/doc/html/rfc2045)
	Accepted Media Types: https://www.iana.org/assignments/media-types/media-types.xhtml
		** application/x-www-form-urlencoded => WHATWG: Anne_van_Kesteren https://www.iana.org/assignments/media-types/application/x-www-form-urlencoded
	List of HTTP Status Codes: https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
	List of HTTP Header Fields: https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
*/

#ifndef _OS_HTTP_H
#define _OS_HTTP_H

#include "base/base_defines_check.h"
#include "base/base_compiler_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "base/base_assert.h"
#include "base/base_notifications.h"
#include "std/std_memset.h"
#include "std/std_includes.h"
#include "mem/mem_arena.h"
#include "mem/mem_scratch.h"
#include "struct/struct_string.h"
#include "struct/struct_var_array.h"
#include "os/os_threading.h"
#include "lib/lib_tracy.h"
#include "misc/misc_two_pass.h"
#include "misc/misc_web.h"

#if BUILD_WITH_HTTP

// We used to use "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:47.0) Gecko/20100101 Firefox/47.0"
#define HTTP_DEFAULT_USER_AGENT_STR       "PigCore/1.0"
#define HTTP_DEFAULT_USER_AGENT_WIDE_STR L"PigCore/1.0"

#define HTTP_MAX_RESPONSE_SIZE   Megabytes(64) // This determines the virtual memory allocated for responseArena, so we only pay the memory cost of the largest response, but once we get a large response we never uncommit that memory

#define HTTP_CALLBACK_DEF(functionName) void functionName(plex HttpRequest* request)
typedef HTTP_CALLBACK_DEF(HttpCallback_f);

typedef plex HttpRequestArgs HttpRequestArgs;
plex HttpRequestArgs
{
	HttpVerb verb;
	Str8 urlStr;
	uxx numHeaders;
	Str8Pair* headers;
	MimeType contentEncoding;
	uxx numContentItems;
	Str8Pair* contentItems;
	HttpCallback_f* callback;
	void* contextPntr;
	uxx contextId;
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
	uxx id;
	HttpRequestState state;
	Result error;
	HttpRequestArgs args;
	
	bool receivingData;
	bool queriedData;
	
	Str8 protocolStr;
	Str8 hostnameStr;
	Str8 pathStr;
	Str8 parametersStr;
	uxx connectionIndex;
	Str8 encodedContent;
	
	#if TARGET_IS_WINDOWS
	HINTERNET requestHandle;
	#endif
	
	VarArray responseBytes; //u8 (allocated from manager.readDataArena)
	u16 statusCode;
	Str8 statusCodeStr;
	bool responseHeadersAvailable;
	Str8 responseHeadersStr;
	uxx numResponseHeaders;
	Str8Pair* responseHeaders;
};

typedef plex HttpConnection HttpConnection;
plex HttpConnection
{
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
	else { dest->headers = nullptr; } //ensure the pointer is cleared so FreeHttpRequest doesn't get mad
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
	else { dest->contentItems = nullptr; } //ensure the pointer is cleared so FreeHttpRequest doesn't get mad
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
	if (request->args.headers != nullptr) { Assert(request->args.numHeaders > 0); FreeArray(Str8Pair, arena, request->args.numHeaders, request->args.headers); }
	for (uxx cIndex = 0; cIndex < request->args.numContentItems; cIndex++)
	{
		FreeStr8(arena, &request->args.contentItems[cIndex].key);
		FreeStr8(arena, &request->args.contentItems[cIndex].value);
	}
	if (request->args.contentItems != nullptr) { Assert(request->args.numContentItems > 0); FreeArray(Str8Pair, arena, request->args.numContentItems, request->args.contentItems); }
	//NOTE: protocolStr, hostnameStr, and pathStr are all slices into args.urlStr so we don't need to free them
	FreeStr8(arena, &request->encodedContent);
	if (request->responseBytes.arena != nullptr) { FreeVarArray(&request->responseBytes); }
	FreeStr8(arena, &request->responseHeadersStr);
	if (request->responseHeaders != nullptr) { FreeArray(Str8Pair, arena, request->numResponseHeaders, request->responseHeaders); }
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
	char printBuffer[512];
	
	ThreadId threadId = OsGetCurrentThreadId();
	bool isMainThread = (threadId == MainThreadId);
	BufferPrintLine_D(printBuffer, "%llu> HttpStatus %s(%04X): %p[%u] (%p,%p)", isMainThread ? (u64)0 : (u64)threadId, GetWinHttpStatusStr(status), status, infoPntr, infoLength, handle, (void*)context);
	if (status == WINHTTP_CALLBACK_STATUS_HANDLE_CREATED || status == WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING)
	{
		//NOTE: HANDLE_CREATED callbacks happen before we have a chance to set the WINHTTP_OPTION_CONTEXT_VALUE option
		//      Since we don't have a contextPntr we can't do much right now
		//NOTE: HANDLE_CLOSING callbacks can happen on handles that don't have a contextPntr set
		// if (isMainThread) { PrintLine_D("%s handle %016X (on thread %llu)", status == WINHTTP_CALLBACK_STATUS_HANDLE_CREATED ? "Created" : "Closed", handle, (u64)threadId); }
		// else { WriteLine_D("Created/Closed handle on secondary thread"); }
		TracyCZoneEnd(Zone_Func);
		return;
	}
	
	Assert(context != 0);
	HttpRequestManager* manager = (HttpRequestManager*)context;
	Assert(manager->arena != nullptr);
	Assert(manager->arena->type > ArenaType_None && manager->arena->type < ArenaType_Count); //Sanity check that the context pntr is being passed correctly by making sure the arena pointer looks valid
	
	bool lockedMutex = false;
	if (!isMainThread || !manager->mainLockedMutex)
	{
		TracyCZoneN(Zone_LockMutex, "LockMutex", true);
		LockMutex(&manager->mutex, TIMEOUT_FOREVER);
		TracyCZoneEnd(Zone_LockMutex);
		lockedMutex = true;
	}
	
	{
		HttpRequest* request = nullptr;
		if (manager->currentRequestIndex < manager->requests.length)
		{
			request = VarArrayGet(HttpRequest, &manager->requests, manager->currentRequestIndex);
			if (request->requestHandle != handle) { request = nullptr; }
		}
		
		switch (status)
		{
			// +==============================================+
			// | WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE |
			// +==============================================+
			case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: //NOT main thread
			{
				NotNull(request);
				BOOL receiveResult = WinHttpReceiveResponse(request->requestHandle, NULL);
				Assert(receiveResult != 0);
			} break;
			
			// +============================================+
			// | WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE |
			// +============================================+
			case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE: //NOT main thread
			{
				//NOTE: We can't call WinHttpQueryDataAvailable right now, we get a INCORRECT_HANDLE_STATE error if we try
				//      So instead we will wait till the main thread is free to query data, at that point the query handle should be available for requesting data
				NotNull(request);
				request->receivingData = true;
			} break;
			
			// +========================================+
			// | WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE |
			// +========================================+
			case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: //NOT main thread
			{
				NotNull(request);
				Assert(infoLength == sizeof(DWORD));
				DWORD numBytesToRead = *((DWORD*)infoPntr);
				BufferPrintLine_D(printBuffer, "%llu byte%s available", (u64)numBytesToRead, Plural(numBytesToRead, "s"));
				
				if (numBytesToRead == 0)
				{
					request->receivingData = false;
					request->state = HttpRequestState_Success;
				}
				else
				{
					if (request->responseBytes.arena == nullptr)
					{
						InitVarArray(u8, &request->responseBytes, &manager->responseArena);
					}
					void* beforePntr = request->responseBytes.items;
					u8* newBytesSpace = VarArrayAddMulti(u8, &request->responseBytes, (uxx)numBytesToRead);
					NotNull(newBytesSpace);
					Assert(beforePntr == request->responseBytes.items || beforePntr == nullptr);
					Assert(request->responseBytes.items == manager->responseArena.mainPntr);
					Assert(handle == request->requestHandle);
					DWORD numBytesRead = 0;
					BOOL readResult = WinHttpReadData(
						request->requestHandle, //hRequest
						newBytesSpace, //lpBuffer
						numBytesToRead, //dwNumberOfBytesToRead
						&numBytesRead //lpdwNumberOfBytesRead
					);
					Assert(readResult != 0);
					if (numBytesRead < numBytesToRead)
					{
						request->responseBytes.length -= (numBytesToRead - numBytesRead);
					}
					
					BufferPrintLine_D(printBuffer, "Read %llu byte%s (total %llu byte%s)", (u64)numBytesRead, Plural(numBytesRead, "s"), request->responseBytes.length, Plural(request->responseBytes.length, "s"));
				}
				request->queriedData = false;
			} break;
			
			// +========================================+
			// | WINHTTP_CALLBACK_STATUS_READ_COMPLETE  |
			// +========================================+
			case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: //NOT main thread
			{
				NotNull(request);
				Assert(request->receivingData);
			} break;
			
			// +==================================+
			// | WINHTTP_CALLBACK_STATUS_REDIRECT |
			// +==================================+
			case WINHTTP_CALLBACK_STATUS_REDIRECT: //NOT main thread
			{
				NotNull(request);
				request->receivingData = false;
			} break;
			
			// +========================================+
			// | WINHTTP_CALLBACK_STATUS_REQUEST_ERROR  |
			// +========================================+
			case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: //NOT main thread
			{
				NotNull(request);
				request->state = HttpRequestState_Failure;
				request->error = Result_Failure; //TODO: Fill with a better errorCode!
			} break;
			
			// +========================================+
			// | WINHTTP_CALLBACK_STATUS_SECURE_FAILURE |
			// +========================================+
			case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE: //main thread
			{
				NotNull(request);
				request->state = HttpRequestState_Failure;
				request->error = Result_SslProblem;
			} break;
			
			// +============================================+
			// | WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE  |
			// +============================================+
			case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
			{
				NotNull(request);
				request->responseHeadersAvailable = true;
			} break;
			
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
			// 		BufferPrintLine_W(printBuffer, "HttpStatus %s(%04X): %p[%u] (%p,%p)", GetWinHttpStatusStr(status), status, infoPntr, infoLength, handle, (void*)context);
			// 	}
			// } break;
		}
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
	
	InitArenaStackVirtual(&manager->responseArena, HTTP_MAX_RESPONSE_SIZE);
	
	#if TARGET_IS_WINDOWS
	{
		manager->sessionHandle = WinHttpOpen(
			HTTP_DEFAULT_USER_AGENT_WIDE_STR,
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			WINHTTP_FLAG_ASYNC
		);
		Assert(manager->sessionHandle != NULL);
		PrintLine_D("Session handle: %016X (manager pntr: %p)", manager->sessionHandle, manager);
		
		//TODO: Refine this list once we know which callbacks we actually want
		DWORD callbackMask = (0
			| WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS
			| WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS
			| WINHTTP_CALLBACK_FLAG_RESOLVE_NAME
			| WINHTTP_CALLBACK_FLAG_CONNECT_TO_SERVER
			| WINHTTP_CALLBACK_FLAG_DETECTING_PROXY
			| WINHTTP_CALLBACK_FLAG_DATA_AVAILABLE
			| WINHTTP_CALLBACK_FLAG_HEADERS_AVAILABLE
			| WINHTTP_CALLBACK_FLAG_READ_COMPLETE
			| WINHTTP_CALLBACK_FLAG_REQUEST_ERROR
			| WINHTTP_CALLBACK_FLAG_SEND_REQUEST
			| WINHTTP_CALLBACK_FLAG_SENDREQUEST_COMPLETE
			| WINHTTP_CALLBACK_FLAG_WRITE_COMPLETE
			| WINHTTP_CALLBACK_FLAG_RECEIVE_RESPONSE
			| WINHTTP_CALLBACK_FLAG_CLOSE_CONNECTION
			| WINHTTP_CALLBACK_FLAG_HANDLES
			| WINHTTP_CALLBACK_FLAG_REDIRECT
			| WINHTTP_CALLBACK_FLAG_INTERMEDIATE_RESPONSE
			| WINHTTP_CALLBACK_FLAG_SECURE_FAILURE
		);
		
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
	TracyCZoneN(Zone_Func, "HttpRequestManagerStartRequest", true);
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
		RangeUXX pathRange = SliceToRangeUXX(request->args.urlStr, request->pathStr);
		RangeUXX parametersRange = SliceToRangeUXX(request->args.urlStr, request->parametersStr);
		RangeUXX pathWithParamsRange = BothRangeUXX(pathRange, parametersRange);
		Str8 pathWithParamsStr = StrSlice(request->args.urlStr, pathWithParamsRange.min, pathWithParamsRange.max);
		Str16 pathWithParamsStrWide = ConvertUtf8StrToUcs2(scratch, pathWithParamsStr, true);
		
		PrintLine_D("Using Connection[%llu] handle: %016X", request->connectionIndex, connection->handle);
		request->requestHandle = WinHttpOpenRequest(connection->handle,
			verbStrWide.chars,
			pathWithParamsStrWide.chars,
			NULL,
			WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			connection->usingSsl ? WINHTTP_FLAG_SECURE : 0
		);
		
		if (request->requestHandle == NULL)
		{
			DWORD error = GetLastError();
			NotifyPrint_E("Failed to open request! Error: %s (0x%04X)", GetWinHttpErrorStr(error), error);
			request->state = HttpRequestState_Failure;
			request->error = Result_FailedToOpenRequest;
		}
		else
		{
			// PrintLine_D("Request handle: %016X", request->requestHandle);
			WinHttpSetOption(request->requestHandle, WINHTTP_OPTION_CONTEXT_VALUE, (void*)&manager, sizeof(void*));
			
			//TODO: We should ensure that the requested headers don't contain an entry for "Content-Type"
			const uxx numExtraHeaders = 1; //One extra header for Content-Type
			uxx numAllHeaders = request->args.numHeaders + numExtraHeaders;
			Str8Pair* allHeaders = AllocArray(Str8Pair, scratch, numAllHeaders);
			NotNull(allHeaders);
			if (request->args.numHeaders > 0) { MyMemCopy(allHeaders, request->args.headers, sizeof(Str8Pair) * request->args.numHeaders); }
			allHeaders[request->args.numHeaders + 0].key = StrLit("Content-Type");
			allHeaders[request->args.numHeaders + 0].value = StrLit(GetMimeTypeOfficialName(request->args.contentEncoding));
			Str8 encodedHeaders = EncodeHttpHeaders(scratch, numAllHeaders, allHeaders, false);
			Str16 encodedHeaders16 = ConvertUtf8StrToUcs2(scratch, encodedHeaders, true);
			request->encodedContent = EncodeHttpKeyValuePairContent(manager->arena,
				request->args.numContentItems, request->args.contentItems,
				request->args.contentEncoding,
				true //addNullTerm
			);
			
			//TODO: Do we need to lock the mutex here?
			
			TracyCZoneN(Zone_WinHttpSendRequest, "WinHttpSendRequest", true);
			//TODO: This function is taking ~8ms to return! We should probably be doing this on another thread!
			BOOL requestResult = WinHttpSendRequest(
				request->requestHandle, //hRequest
				encodedHeaders16.chars, //lpszHeaders
				(DWORD)encodedHeaders16.length, //dwHeadersLength
				request->encodedContent.chars, //lpOptional
				(DWORD)request->encodedContent.length, //dwOptionalLength
				(DWORD)request->encodedContent.length, //dwTotalLength
				0 //dwContext
			);
			TracyCZoneEnd(Zone_WinHttpSendRequest);
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
	TracyCZoneEnd(Zone_Func);
	return result;
}

PEXP void OsUpdateHttpRequestManager(HttpRequestManager* manager, u64 programTime)
{
	NotNull(manager);
	NotNull(manager->arena);
	UNUSED(programTime);
	TracyCZoneN(Zone_Func, "OsUpdateHttpRequestManager", true);
	
	// +==============================+
	// |     Check currentRequest     |
	// +==============================+
	uxx doCallbackIndex = UINTXX_MAX;
	LockMutexBlockWithTracyZone(&manager->mutex, TIMEOUT_FOREVER, Zone_LockMutex, "LockMutex")
	{
		manager->mainLockedMutex = true;
		TracyCZoneN(Zone_CheckCurrentRequest, "CheckCurrentRequest", true);
		if (manager->currentRequestIndex < manager->requests.length)
		{
			HttpRequest* currentRequest = VarArrayGet(HttpRequest, &manager->requests, manager->currentRequestIndex);
			
			if (currentRequest->responseHeadersAvailable)
			{
				currentRequest->responseHeadersAvailable = false;
				
				// +==============================+
				// |     Get HTTP Status Code     |
				// +==============================+
				if (currentRequest->statusCode == 0)
				{
					DWORD statusCode = 0;
					DWORD statusCodeSize = sizeof(statusCode);
					BOOL queryResult = WinHttpQueryHeaders(
						currentRequest->requestHandle, //hRequest
						WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, //dwInfoLevel
						WINHTTP_HEADER_NAME_BY_INDEX, //pwszName
						&statusCode, //lpBuffer
						&statusCodeSize, //lpdwBufferLength
						WINHTTP_NO_HEADER_INDEX //lpdwIndex
					);
					Assert(queryResult == TRUE);
					Assert(statusCodeSize == sizeof(DWORD));
					Assert(statusCode < 600); //TODO: Do codes above 600 exist?
					currentRequest->statusCode = (u16)statusCode;
					// PrintLine_D("Status Code: %u", currentRequest->statusCode);
					
					//TODO: Can we get the statusCode readable string?
				}
				
				// +==============================+
				// |     Get Response Headers     |
				// +==============================+
				if (currentRequest->responseHeadersStr.length == 0)
				{
					ScratchBegin1(scratch, manager->arena);
					
					DWORD headersByteLength = 0;
					BOOL queryResult1 = WinHttpQueryHeaders(
						currentRequest->requestHandle, //hRequest
						WINHTTP_QUERY_RAW_HEADERS_CRLF, //dwInfoLevel
						WINHTTP_HEADER_NAME_BY_INDEX, //pwszName
						nullptr, //lpBuffer
						&headersByteLength, //lpdwBufferLength
						WINHTTP_NO_HEADER_INDEX //lpdwIndex
					);
					Assert(queryResult1 == FALSE);
					Assert(GetLastError() == ERROR_INSUFFICIENT_BUFFER);
					if (headersByteLength > 0)
					{
						DWORD wideHeadersByteLength = headersByteLength;
						char16_t* wideBuffer = AllocArray(char16_t, scratch, (uxx)(wideHeadersByteLength / sizeof(char16_t)));
						NotNull(wideBuffer);
						BOOL queryResult2 = WinHttpQueryHeaders(
							currentRequest->requestHandle, //hRequest
							WINHTTP_QUERY_RAW_HEADERS_CRLF, //dwInfoLevel
							WINHTTP_HEADER_NAME_BY_INDEX, //pwszName
							wideBuffer, //lpBuffer
							&wideHeadersByteLength, //lpdwBufferLength
							WINHTTP_NO_HEADER_INDEX //lpdwIndex
						);
						Assert(queryResult2 == TRUE);
						Str16 headersWideStr = NewStr16(wideHeadersByteLength/sizeof(char16_t), wideBuffer);
						currentRequest->responseHeadersStr = ConvertUcs2StrToUtf8(manager->arena, headersWideStr, false);
						currentRequest->numResponseHeaders = DecodeHttpHeaders(manager->arena, currentRequest->responseHeadersStr, false, &currentRequest->responseHeaders);
						// PrintLine_D("Headers [%llu]:\n%.*s\n\n", currentRequest->responseHeadersStr.length, StrPrint(currentRequest->responseHeadersStr));
					}
					
					ScratchEnd(scratch);
				}
			}
			
			if (currentRequest->state >= HttpRequestState_Success)
			{
				// Remember this index so we can do the callback
				doCallbackIndex = manager->currentRequestIndex;
				manager->currentRequestIndex = UINTXX_MAX;
				
				// +==========================================+
				// | Move Response Bytes out of responseArena |
				// +==========================================+
				if (currentRequest->responseBytes.arena == &manager->responseArena)
				{
					if (currentRequest->responseBytes.length > 0)
					{
						uxx numBytes = currentRequest->responseBytes.length;
						u8* srcPntr = (u8*)currentRequest->responseBytes.items;
						InitVarArrayWithInitial(u8, &currentRequest->responseBytes, manager->arena, numBytes);
						u8* destPntr = VarArrayAddMulti(u8, &currentRequest->responseBytes, numBytes);
						NotNull(destPntr);
						MyMemCopy(destPntr, srcPntr, numBytes);
					}
					else
					{
						ClearStruct(currentRequest->responseBytes);
					}
					ArenaResetToMark(&manager->responseArena, 0);
				}
			}
			else if (currentRequest->state == HttpRequestState_InProgress)
			{
				// +================================+
				// | Call WinHttpQueryDataAvailable |
				// +================================+
				if (currentRequest->receivingData && !currentRequest->queriedData)
				{
					currentRequest->queriedData = true;
					BOOL queryResult = WinHttpQueryDataAvailable(currentRequest->requestHandle, NULL);
					if (queryResult != TRUE)
					{
						currentRequest->queriedData = false;
						currentRequest->state = HttpRequestState_Failure;
						currentRequest->error = Result_WinHTTPError;
						DWORD errorCode = GetLastError();
						PrintLine_D("WinHTTP QueryData Error: %s", GetWinHttpErrorStr(errorCode));
					}
				}
			}
		}
		TracyCZoneEnd(Zone_CheckCurrentRequest);
		manager->mainLockedMutex = false;
	}
	
	// +==========================================+
	// | Do Callbacks and Start Pending Requests  |
	// +==========================================+
	while (manager->currentRequestIndex >= manager->requests.length)
	{
		TracyCZoneN(Zone_HttpCallback, "HttpCallback", true);
		if (doCallbackIndex < manager->requests.length)
		{
			HttpRequest* request = VarArrayGet(HttpRequest, &manager->requests, doCallbackIndex);
			PrintLine_D("Callback on request %llu to \"%.*s\" result=%s, got %llu byte%s",
				request->id,
				StrPrint(request->args.urlStr),
				GetHttpRequestStateStr(request->state),
				request->responseBytes.length, Plural(request->responseBytes.length, "s")
			);
			// Write_D("["); VarArrayLoop(&request->responseBytes, bIndex) { VarArrayLoopGetValue(u8, responseByte, &request->responseBytes, bIndex); Print_D(" %02X", responseByte); } WriteLine_D(" ]");
			if (request->args.callback != nullptr)
			{
				request->args.callback(request);
			}
			FreeHttpRequest(manager->arena, request);
			doCallbackIndex = UINTXX_MAX;
		}
		TracyCZoneEnd(Zone_HttpCallback);
		
		TracyCZoneN(Zone_TryStartPendingRequest, "TryStartPendingRequest", true);
		bool foundPendingRequest = false;
		VarArrayLoop(&manager->requests, rIndex)
		{
			VarArrayLoopGet(HttpRequest, request, &manager->requests, rIndex);
			if (request->state == HttpRequestState_NotStarted)
			{
				bool startedSuccessfully = false;
				LockMutexBlockWithTracyZone(&manager->mutex, TIMEOUT_FOREVER, Zone_LockMutex2, "LockMutex")
				{
					manager->mainLockedMutex = true;
					startedSuccessfully = HttpRequestManagerStartRequest(manager, rIndex);
					manager->mainLockedMutex = false;
				}
				if (!startedSuccessfully) { doCallbackIndex = rIndex; }
				foundPendingRequest = true;
				break;
			}
		}
		TracyCZoneEnd(Zone_TryStartPendingRequest);
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
	Assert(args->contentEncoding != MimeType_None && args->contentEncoding < MimeType_Count);
	
	HttpRequest* newRequest = nullptr;
	VarArrayLoop(&manager->requests, rIndex)
	{
		VarArrayLoopGet(HttpRequest, request, &manager->requests, rIndex);
		if (request->state == HttpRequestState_None) { newRequest = request; break; }
	}
	if (newRequest == nullptr)
	{
		LockMutexBlockWithTracyZone(&manager->mutex, TIMEOUT_FOREVER, Zone_LockMutex, "LockMutex")
		{
			manager->mainLockedMutex = true;
			newRequest = VarArrayAdd(HttpRequest, &manager->requests);
			NotNull(newRequest);
			manager->mainLockedMutex = false;
		}
	}
	
	ClearPointer(newRequest);
	newRequest->id = manager->nextRequestId;
	manager->nextRequestId++;
	OsCopyHttpRequestArgs(manager->arena, &newRequest->args, args);
	newRequest->state = HttpRequestState_NotStarted;
	UriParts uriParts = GetUriParts(newRequest->args.urlStr);
	newRequest->protocolStr = uriParts.protocol;
	if (IsEmptyStr(newRequest->protocolStr)) { newRequest->protocolStr = StrLit("https"); }
	Assert(StrExactEquals(newRequest->protocolStr, StrLit("http")) || StrExactEquals(newRequest->protocolStr, StrLit("https"))); //TODO: Report this as an error!
	newRequest->hostnameStr = uriParts.hostname;
	Assert(!IsEmptyStr(newRequest->hostnameStr));
	newRequest->pathStr = uriParts.path;
	if (IsEmptyStr(newRequest->pathStr)) { newRequest->pathStr = StrLit("/"); }
	newRequest->parametersStr = uriParts.parameters;
	//TODO: Should we save the uriParts.anchor? How do we send this to WinHTTP? It doesn't seem to work when we include it in the path+query_params that we pass to WinHttpOpenRequest
	
	bool usingSsl = StrExactEquals(newRequest->protocolStr, StrLit("https"));
	u16 portNumber = usingSsl ? HTTPS_PORT : HTTP_PORT;
	
	HttpConnection* connection = OsFindHttpConnection(manager, newRequest->hostnameStr, portNumber);
	if (connection == nullptr)
	{
		HttpConnection* newConnection = VarArrayAdd(HttpConnection, &manager->connections);
		ClearPointer(newConnection);
		newConnection->hostname = AllocStr8(manager->arena, newRequest->hostnameStr);
		newConnection->portNumber = portNumber;
		newConnection->usingSsl = usingSsl;
		newConnection->openTime = programTime;
		newConnection->lastUsedTime = programTime;
		newConnection->keepaliveTime = Thousand(10); //TODO: What is this value?
		PrintLine_D("[%llu] Connecting to %.*s%s...", manager->connections.length-1, StrPrint(newConnection->hostname), usingSsl ? " using SSL" : "");
		#if TARGET_IS_WINDOWS
		Str16 hostnameWide = ConvertUtf8StrToUcs2(manager->arena, newConnection->hostname, true);
		NotNull(hostnameWide.chars);
		newConnection->handle = WinHttpConnect(
			manager->sessionHandle, //hSession
			hostnameWide.chars, //pswzServerName
			portNumber, //nServerPort
			0 //dwReserved
		);
		Assert(newConnection->handle != NULL); //TODO: Is this a valid error scenario we should handle?
		// PrintLine_D("Connection Handle: %016X (pntr %p)", newConnection->handle, newConnection);
		#endif
		connection = newConnection;
	}
	VarArrayGetIndexOf(HttpConnection, &manager->connections, connection, &newRequest->connectionIndex);
	connection->lastUsedTime = programTime;
	
	PrintLine_D("Request to %.*s %.*s%s created", StrPrint(newRequest->hostnameStr), StrPrint(newRequest->pathStr), usingSsl ? " using SLL" : "");
	//TODO: can we do WinHttpOpenRequest when another request is potentially active?
	
	//TODO: Should we call OsUpdateHttpRequestManager directly?
	
	ScratchEnd(scratch);
	return newRequest;
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //BUILD_WITH_HTTP

#endif //  _OS_HTTP_H
