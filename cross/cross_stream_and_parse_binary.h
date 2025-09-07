/*
File:   cross_stream_and_parse_binary.h
Author: Taylor Robbins
Date:   09\06\2025
*/

#ifndef _CROSS_STREAM_AND_PARSE_BINARY_H
#define _CROSS_STREAM_AND_PARSE_BINARY_H

//NOTE: Intentionally no includes here

#define BinStreamReadU8(streamPntr, arenaPntr, errorCode)   *(u8*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(u8),  (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadU16(streamPntr, arenaPntr, errorCode) *(u16*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(u16), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadU32(streamPntr, arenaPntr, errorCode) *(u32*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(u32), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadU64(streamPntr, arenaPntr, errorCode) *(u64*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(u64), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadI8(streamPntr, arenaPntr, errorCode)   *(i8*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(i8),  (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadI16(streamPntr, arenaPntr, errorCode) *(i16*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(i16), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadI32(streamPntr, arenaPntr, errorCode) *(i32*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(i32), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadI64(streamPntr, arenaPntr, errorCode) *(i64*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(i64), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadR32(streamPntr, arenaPntr, errorCode) *(r32*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(r32), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }
#define BinStreamReadR64(streamPntr, arenaPntr, errorCode) *(r64*)TryReadFromDataStreamOrZeros((streamPntr), sizeof(r64), (arenaPntr)); if ((streamPntr)->error != Result_None) { errorCode; }

#endif //  _CROSS_STREAM_AND_PARSE_BINARY_H
