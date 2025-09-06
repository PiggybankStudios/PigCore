/*
File:   parse_binary.h
Author: Taylor Robbins
Date:   09\06\2025
Description:
	** Holds some useful macros and functions that help us pull data out of a binary file easily
*/

#ifndef _PARSE_BINARY_H
#define _PARSE_BINARY_H

#include "base/base_defines_check.h"
#include "base/base_typedefs.h"

#define BinReadU8(slice, readIndexPntr, errorCode)  (((*(readIndexPntr) + sizeof(u8))  <= (slice).length) ?  *(u8*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) +  sizeof(u8)) > (slice).length) { errorCode; } else { *(readIndexPntr) +=  sizeof(u8); } do {} while(0)
#define BinReadU16(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(u16)) <= (slice).length) ? *(u16*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(u16)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(u16); } do {} while(0)
#define BinReadU32(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(u32)) <= (slice).length) ? *(u32*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(u32)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(u32); } do {} while(0)
#define BinReadU64(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(u64)) <= (slice).length) ? *(u64*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(u64)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(u64); } do {} while(0)
#define BinReadI8(slice, readIndexPntr, errorCode)  (((*(readIndexPntr) + sizeof(i8))  <= (slice).length) ?  *(i8*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) +  sizeof(i8)) > (slice).length) { errorCode; } else { *(readIndexPntr) +=  sizeof(i8); } do {} while(0)
#define BinReadI16(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(i16)) <= (slice).length) ? *(i16*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(i16)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(i16); } do {} while(0)
#define BinReadI32(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(i32)) <= (slice).length) ? *(i32*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(i32)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(i32); } do {} while(0)
#define BinReadI64(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(i64)) <= (slice).length) ? *(i64*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(i64)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(i64); } do {} while(0)
#define BinReadR32(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(r32)) <= (slice).length) ? *(r32*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(r32)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(r32); } do {} while(0)
#define BinReadR64(slice, readIndexPntr, errorCode) (((*(readIndexPntr) + sizeof(r64)) <= (slice).length) ? *(r64*)(&(slice).bytes[*(readIndexPntr)]) : 0); if ((*(readIndexPntr) + sizeof(r64)) > (slice).length) { errorCode; } else { *(readIndexPntr) += sizeof(r64); } do {} while(0)

#endif //  _PARSE_BINARY_H
