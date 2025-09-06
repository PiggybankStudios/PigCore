/*
File:   os_error.h
Author: Taylor Robbins
Date:   01\08\2025
Description:
	** Contains some functions that help us resolve various error codes to debug output friendly strings
*/

#ifndef _OS_ERROR_H
#define _OS_ERROR_H

#include "base/base_compiler_check.h"
#include "base/base_defines_check.h"
#include "base/base_typedefs.h"
#include "base/base_macros.h"
#include "std/std_includes.h"

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	#if TARGET_IS_WINDOWS
	const char* Win32_GetErrorCodeStr(DWORD windowsErrorCode);
	#endif
	#if (TARGET_IS_LINUX || TARGET_IS_OSX)
	const char* GetErrnoStr(int errnoValue);
	#endif
#endif //!PIG_CORE_IMPLEMENTATION

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

#if TARGET_IS_WINDOWS
PEXP const char* Win32_GetErrorCodeStr(DWORD windowsErrorCode)
{
	switch (windowsErrorCode)
	{
		// https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
		case ERROR_SUCCESS:              return "ERROR_SUCCESS";              //0
		case ERROR_FILE_NOT_FOUND:       return "ERROR_FILE_NOT_FOUND";       //2
		case ERROR_PATH_NOT_FOUND:       return "ERROR_PATH_NOT_FOUND";       //3
		case ERROR_ACCESS_DENIED:        return "ERROR_ACCESS_DENIED";        //5
		case ERROR_SHARING_VIOLATION:    return "ERROR_SHARING_VIOLATION";    //32
		case ERROR_FILE_EXISTS:          return "ERROR_FILE_EXISTS";          //80
		case ERROR_INVALID_PARAMETER:    return "ERROR_INVALID_PARAMETER";    //87
		case ERROR_MOD_NOT_FOUND:        return "ERROR_MOD_NOT_FOUND";        //126
		case ERROR_ALREADY_EXISTS:       return "ERROR_ALREADY_EXISTS";       //183
		case ERROR_PIPE_BUSY:            return "ERROR_PIPE_BUSY";            //231
		case ERROR_DIRECTORY:            return "ERROR_DIRECTORY";            //267
		case ERROR_CLASS_ALREADY_EXISTS: return "ERROR_CLASS_ALREADY_EXISTS"; //1410
		// default: return (printUnknownValue ? TempPrint("(0x%08X)", windowsErrorCode) : UNKNOWN_STR); //TODO: Add this option back in once we have PrintInArena function!
		default: return UNKNOWN_STR;
	}
}
#endif //TARGET_IS_WINDOWS

#if (TARGET_IS_LINUX || TARGET_IS_OSX)
PEXP const char* GetErrnoStr(int errnoValue)
{
	switch (errnoValue)
	{
		//NOTE: These are all pulled from - https://man7.org/linux/man-pages/man3/errno.3.html
		case 0:               return "NONE";
		case E2BIG:           return "E2BIG";           //Argument list too long (POSIX.1-2001).
		case EACCES:          return "EACCES";          //Permission denied (POSIX.1-2001).
		case EADDRINUSE:      return "EADDRINUSE";      //Address already in use (POSIX.1-2001).
		case EADDRNOTAVAIL:   return "EADDRNOTAVAIL";   //Address not available (POSIX.1-2001).
		case EAFNOSUPPORT:    return "EAFNOSUPPORT";    //Address family not supported (POSIX.1-2001).
		case EAGAIN:          return "EAGAIN";          //Resource temporarily unavailable (may be the same value as EWOULDBLOCK) (POSIX.1-2001).
		case EALREADY:        return "EALREADY";        //Connection already in progress (POSIX.1-2001).
		case EBADF:           return "EBADF";           // Bad file descriptor (POSIX.1-2001).
		case EBADMSG:         return "EBADMSG";         //Bad message (POSIX.1-2001).
		case EBUSY:           return "EBUSY";           //Device or resource busy (POSIX.1-2001).
		case ECANCELED:       return "ECANCELED";       //Operation canceled (POSIX.1-2001).
		case ECHILD:          return "ECHILD";          //No child processes (POSIX.1-2001).
		case ECONNABORTED:    return "ECONNABORTED";    //Connection aborted (POSIX.1-2001).
		case ECONNREFUSED:    return "ECONNREFUSED";    //Connection refused (POSIX.1-2001).
		case ECONNRESET:      return "ECONNRESET";      //Connection reset (POSIX.1-2001).
		case EDEADLK:         return "EDEADLK";         //Resource deadlock avoided (POSIX.1-2001).
		#if (defined(EDEADLOCK) && (!defined(EDEADLK) || EDEADLK != EDEADLOCK))
		case EDEADLOCK:       return "EDEADLOCK";       //On most architectures, a synonym for EDEADLK.  On some architectures (e.g., Linux MIPS, PowerPC, SPARC), it is a separate error code "File locking deadlock error".
		#endif
		case EDESTADDRREQ:    return "EDESTADDRREQ";    //Destination address required (POSIX.1-2001).
		case EDOM:            return "EDOM";            //Mathematics argument out of domain of function (POSIX.1, C99).
		case EDQUOT:          return "EDQUOT";          //Disk quota exceeded (POSIX.1-2001).
		case EEXIST:          return "EEXIST";          //File exists (POSIX.1-2001).
		case EFAULT:          return "EFAULT";          //Bad address (POSIX.1-2001).
		case EFBIG:           return "EFBIG";           // File too large (POSIX.1-2001).
		case EHOSTDOWN:       return "EHOSTDOWN";       //Host is down.
		case EHOSTUNREACH:    return "EHOSTUNREACH";    //Host is unreachable (POSIX.1-2001).
		case EIDRM:           return "EIDRM";           //Identifier removed (POSIX.1-2001).
		case EILSEQ:          return "EILSEQ";          //Invalid or incomplete multibyte or wide character (POSIX.1, C99). The text shown here is the glibc error description; in POSIX.1, this error is described as "Illegal byte sequence".
		case EINPROGRESS:     return "EINPROGRESS";     //Operation in progress (POSIX.1-2001).
		case EINTR:           return "EINTR";           //Interrupted function call (POSIX.1-2001); see signal(7).
		case EINVAL:          return "EINVAL";          //Invalid argument (POSIX.1-2001).
		case EIO:             return "EIO";             //Input/output error (POSIX.1-2001).
		case EISCONN:         return "EISCONN";         //Socket is connected (POSIX.1-2001).
		case EISDIR:          return "EISDIR";          //Is a directory (POSIX.1-2001).
		case ELOOP:           return "ELOOP";           //Too many levels of symbolic links (POSIX.1-2001).
		case EMFILE:          return "EMFILE";          //Too many open files (POSIX.1-2001).  Commonly caused by exceeding the RLIMIT_NOFILE resource limit described in getrlimit(2).  Can also be caused by exceeding the limit specified in /proc/sys/fs/nr_open.
		case EMLINK:          return "EMLINK";          //Too many links (POSIX.1-2001).
		case EMSGSIZE:        return "EMSGSIZE";        //Message too long (POSIX.1-2001).
		case EMULTIHOP:       return "EMULTIHOP";       //Multihop attempted (POSIX.1-2001).
		case ENAMETOOLONG:    return "ENAMETOOLONG";    //Filename too long (POSIX.1-2001).
		case ENETDOWN:        return "ENETDOWN";        //Network is down (POSIX.1-2001).
		case ENETRESET:       return "ENETRESET";       //Connection aborted by network (POSIX.1-2001).
		case ENETUNREACH:     return "ENETUNREACH";     //Network unreachable (POSIX.1-2001).
		case ENFILE:          return "ENFILE";          //Too many open files in system (POSIX.1-2001).  On Linux, this is probably a result of encountering the /proc/sys/fs/file-max limit (see proc(5)).
		case ENOBUFS:         return "ENOBUFS";         //No buffer space available (POSIX.1 (XSI STREAMS option)).
		case ENODATA:         return "ENODATA";         //The named attribute does not exist, or the process has no access to this attribute; see xattr(7). In POSIX.1-2001 (XSI STREAMS option), this error was described as "No message is available on the STREAM head read queue".
		case ENODEV:          return "ENODEV";          //No such device (POSIX.1-2001).
		case ENOENT:          return "ENOENT";          //No such file or directory (POSIX.1-2001). Typically, this error results when a specified pathname does not exist, or one of the components in the directory prefix of a pathname does not exist, or the specified pathname is a dangling symbolic link.
		case ENOEXEC:         return "ENOEXEC";         //Exec format error (POSIX.1-2001).
		case ENOLCK:          return "ENOLCK";          //No locks available (POSIX.1-2001).
		case ENOLINK:         return "ENOLINK";         //Link has been severed (POSIX.1-2001).
		case ENOMEM:          return "ENOMEM";          //Not enough space/cannot allocate memory (POSIX.1-2001).
		case ENOMSG:          return "ENOMSG";          //No message of the desired type (POSIX.1-2001).
		case ENOPROTOOPT:     return "ENOPROTOOPT";     //Protocol not available (POSIX.1-2001).
		case ENOSPC:          return "ENOSPC";          //No space left on device (POSIX.1-2001).
		case ENOSR:           return "ENOSR";           //No STREAM resources (POSIX.1 (XSI STREAMS option)).
		case ENOSTR:          return "ENOSTR";          //Not a STREAM (POSIX.1 (XSI STREAMS option)).
		case ENOSYS:          return "ENOSYS";          //Function not implemented (POSIX.1-2001).
		case ENOTBLK:         return "ENOTBLK";         //Block device required.
		case ENOTCONN:        return "ENOTCONN";        //The socket is not connected (POSIX.1-2001).
		case ENOTDIR:         return "ENOTDIR";         //Not a directory (POSIX.1-2001).
		case ENOTEMPTY:       return "ENOTEMPTY";       //Directory not empty (POSIX.1-2001).
		case ENOTRECOVERABLE: return "ENOTRECOVERABLE"; //State not recoverable (POSIX.1-2008).
		case ENOTSOCK:        return "ENOTSOCK";        //Not a socket (POSIX.1-2001).
		case ENOTSUP:         return "ENOTSUP";         //Operation not supported (POSIX.1-2001).
		case ENOTTY:          return "ENOTTY";          //Inappropriate I/O control operation (POSIX.1-2001).
		case ENXIO:           return "ENXIO";           //No such device or address (POSIX.1-2001).
		#if (defined(EOPNOTSUPP) && (!defined(ENOTSUP) || ENOTSUP != EOPNOTSUPP))
		case EOPNOTSUPP:      return "EOPNOTSUPP";      //Operation not supported on socket (POSIX.1-2001). (ENOTSUP and EOPNOTSUPP have the same value on Linux, but according to POSIX.1 these error values should be distinct.)
		#endif
		case EOVERFLOW:       return "EOVERFLOW";       //Value too large to be stored in data type (POSIX.1-2001).
		case EOWNERDEAD:      return "EOWNERDEAD";      //Owner died (POSIX.1-2008).
		case EPERM:           return "EPERM";           //Operation not permitted (POSIX.1-2001).
		case EPFNOSUPPORT:    return "EPFNOSUPPORT";    //Protocol family not supported.
		case EPIPE:           return "EPIPE";           //Broken pipe (POSIX.1-2001).
		case EPROTO:          return "EPROTO";          //Protocol error (POSIX.1-2001).
		case EPROTONOSUPPORT: return "EPROTONOSUPPORT"; //Protocol not supported (POSIX.1-2001).
		case EPROTOTYPE:      return "EPROTOTYPE";      //Protocol wrong type for socket (POSIX.1-2001).
		case ERANGE:          return "ERANGE";          //Result too large (POSIX.1, C99).
		case EREMOTE:         return "EREMOTE";         //Object is remote.
		case EROFS:           return "EROFS";           //Read-only filesystem (POSIX.1-2001).
		case ESHUTDOWN:       return "ESHUTDOWN";       //Cannot send after transport endpoint shutdown.
		case ESPIPE:          return "ESPIPE";          //Invalid seek (POSIX.1-2001).
		case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT"; //Socket type not supported.
		case ESRCH:           return "ESRCH";           //No such process (POSIX.1-2001).
		case ESTALE:          return "ESTALE";          //Stale file handle (POSIX.1-2001). This error can occur for NFS and for other filesystems.
		case ETIME:           return "ETIME";           //Timer expired (POSIX.1 (XSI STREAMS option)). (POSIX.1 says "STREAM ioctl(2) timeout".)
		case ETIMEDOUT:       return "ETIMEDOUT";       //Connection timed out (POSIX.1-2001).
		case ETOOMANYREFS:    return "ETOOMANYREFS";    //Too many references: cannot splice.
		case ETXTBSY:         return "ETXTBSY";         //Text file busy (POSIX.1-2001).
		case EUSERS:          return "EUSERS";          //Too many users.
		#if (defined(EWOULDBLOCK) && (!defined(EAGAIN) || EAGAIN != EWOULDBLOCK))
		case EWOULDBLOCK:     return "EWOULDBLOCK";     //Operation would block (may be same value as EAGAIN) (POSIX.1-2001).
		#endif
		case EXDEV:           return "EXDEV";           //Invalid cross-device link (POSIX.1-2001).
		#if TARGET_IS_LINUX
		case EBADE:           return "EBADE";           // Invalid exchange.
		case EBADFD:          return "EBADFD";          //File descriptor in bad state.
		case EBADR:           return "EBADR";           // Invalid request descriptor.
		case EBADRQC:         return "EBADRQC";         //Invalid request code.
		case EBADSLT:         return "EBADSLT";         //Invalid slot.
		case ECHRNG:          return "ECHRNG";          //Channel number out of range.
		case ECOMM:           return "ECOMM";           //Communication error on send.
		case EHWPOISON:       return "EHWPOISON";       //Memory page has hardware error.
		case EISNAM:          return "EISNAM";          //Is a named type file.
		case EKEYEXPIRED:     return "EKEYEXPIRED";     //Key has expired.
		case EKEYREJECTED:    return "EKEYREJECTED";    //Key was rejected by service.
		case EKEYREVOKED:     return "EKEYREVOKED";     //Key has been revoked.
		case EL2HLT:          return "EL2HLT";          //Level 2 halted.
		case EL2NSYNC:        return "EL2NSYNC";        //Level 2 not synchronized.
		case EL3HLT:          return "EL3HLT";          //Level 3 halted.
		case EL3RST:          return "EL3RST";          //Level 3 reset.
		case ELIBACC:         return "ELIBACC";         //Cannot access a needed shared library.
		case ELIBBAD:         return "ELIBBAD";         //Accessing a corrupted shared library.
		case ELIBMAX:         return "ELIBMAX";         //Attempting to link in too many shared libraries.
		case ELIBSCN:         return "ELIBSCN";         //.lib section in a.out corrupted
		case ELIBEXEC:        return "ELIBEXEC";        //Cannot exec a shared library directly.
		case ELNRNG:          return "ELNRNG";          //Link number out of range.
		case EMEDIUMTYPE:     return "EMEDIUMTYPE";     //Wrong medium type.
		case ENOANO:          return "ENOANO";          //No anode.
		case ENOKEY:          return "ENOKEY";          //Required key not available.
		case ENOMEDIUM:       return "ENOMEDIUM";       //No medium found.
		case ENONET:          return "ENONET";          //Machine is not on the network.
		case ENOPKG:          return "ENOPKG";          //Package not installed.
		case ENOTUNIQ:        return "ENOTUNIQ";        //Name not unique on network.
		case EREMCHG:         return "EREMCHG";         //Remote address changed.
		case EREMOTEIO:       return "EREMOTEIO";       //Remote I/O error.
		case ERESTART:        return "ERESTART";        //Interrupted system call should be restarted.
		case ERFKILL:         return "ERFKILL";         //Operation not possible due to RF-kill.
		case ESTRPIPE:        return "ESTRPIPE";        //Streams pipe error.
		case EUCLEAN:         return "EUCLEAN";         //Structure needs cleaning.
		case EUNATCH:         return "EUNATCH";         //Protocol driver not attached.
		case EXFULL:          return "EXFULL";          //Exchange full.
		#endif //TARGET_IS_LINUX
		default: return UNKNOWN_STR;
	}
}
#endif

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _OS_ERROR_H
