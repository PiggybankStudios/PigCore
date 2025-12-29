/*
File:   struct_work_subject.h
Author: Taylor Robbins
Date:   12\29\2025
Description:
	** This is a generic holder for parameters to a threaded piece of work
	** Like pntrs to memory it needs to work on, or enum values that determine the kind of work it should do
	** Since all threaded work ultimately has to start in something like ThreadPoolThread_Main it
	** needs some way to get back to the proper context where it can do a specific bit of work
*/

#ifndef _STRUCT_WORK_SUBJECT_H
#define _STRUCT_WORK_SUBJECT_H

#include "base/base_typedefs.h"
#include "base/base_assert.h"
#include "base/base_macros.h"
#include "mem/mem_arena.h"
#include "struct/struct_string.h"
#include "os/os_threading.h"

//NOTE: Make sure the WorkSubject structure is updated if these values change (add or remove bespoke aliases like "id2", "index1", etc.)
#define WORK_SUBJECT_NUM_IDS     3
#define WORK_SUBJECT_NUM_INDICES 3
#define WORK_SUBJECT_NUM_ARENAS  3
#define WORK_SUBJECT_NUM_PNTRS   3
#define WORK_SUBJECT_NUM_SLICES  3

typedef plex WorkSubject WorkSubject;
plex WorkSubject
{
	Mutex* mutex;
	
	car
	{
		uxx ids[WORK_SUBJECT_NUM_IDS];
		uxx id; //aka id0
		plex { uxx id0; uxx id1; uxx id2; };
	};
	car
	{
		uxx indices[WORK_SUBJECT_NUM_INDICES];
		uxx index; //aka index0
		plex { uxx index0; uxx index1; uxx index2; };
	};
	car
	{
		Arena* arenas[WORK_SUBJECT_NUM_ARENAS];
		Arena* arena; //aka arena0
		plex { Arena* arena0; Arena* arena1; Arena* arena2; };
	};
	car
	{
		void* pntrs[WORK_SUBJECT_NUM_PNTRS];
		void* pntr; //aka pntr0
		plex { void* pntr0; void* pntr1; void* pntr2; };
	};
	car
	{
		Str8 strings[WORK_SUBJECT_NUM_SLICES];
		Slice slices[WORK_SUBJECT_NUM_SLICES];
		Str8 string; //aka string0
		Slice slice; //aka slice0
		plex { Str8 string0; Str8 string1; Str8 string2; };
		plex { Slice slice0; Slice slice1; Slice slice2; };
	};
	
	// Setting sliceAllocArena will cause the ThreadPool to call FreeMem on that slice when FreeThreadPoolWorkItem is called
	Arena* sliceAllocArena[WORK_SUBJECT_NUM_SLICES];
	uxx sliceAllocAlignment[WORK_SUBJECT_NUM_SLICES];
	bool sliceHasNullTerm[WORK_SUBJECT_NUM_SLICES];
};

// +--------------------------------------------------------------+
// |                 Header Function Declarations                 |
// +--------------------------------------------------------------+
#if !PIG_CORE_IMPLEMENTATION
	PIG_CORE_INLINE void FreeWorkSubject(WorkSubject* subject);
	PIG_CORE_INLINE void* AllocStructInWorkSubject_(Arena* arena, WorkSubject* subject, uxx sliceIndex, uxx typeSize, uxx typeAlignment);
	PIG_CORE_INLINE void* GetStructInWorkSubject_(WorkSubject* subject, uxx sliceIndex, uxx typeSize, uxx typeAlignment);
	PIG_CORE_INLINE Str8 AllocStringInWorkSubject(Arena* arena, WorkSubject* subject, uxx stringIndex, Str8 stringValue, bool addNullTerm);
#endif

#if LANGUAGE_IS_C
#define AllocStructInWorkSubject(type, arenaPntr, subjectPntr, sliceIndex) (type*)AllocStructInWorkSubject_((arenaPntr), (subjectPntr), (sliceIndex), sizeof(type), _Alignof(type))
#define GetStructInWorkSubject(type, subjectPntr, sliceIndex) (type*)GetStructInWorkSubject_((subjectPntr), (sliceIndex), sizeof(type), _Alignof(type))
#else
#define AllocStructInWorkSubject(type, arenaPntr, subjectPntr, sliceIndex) (type*)AllocStructInWorkSubject_((arenaPntr), (subjectPntr), (sliceIndex), sizeof(type), std::alignment_of<type>())
#define GetStructInWorkSubject(type, subjectPntr, sliceIndex) (type*)GetStructInWorkSubject_((subjectPntr), (sliceIndex), sizeof(type), std::alignment_of<type>())
#endif

// +--------------------------------------------------------------+
// |                   Function Implementations                   |
// +--------------------------------------------------------------+
#if PIG_CORE_IMPLEMENTATION

PEXPI void FreeWorkSubject(WorkSubject* subject)
{
	NotNull(subject);
	for (uxx sIndex = 0; sIndex < WORK_SUBJECT_NUM_SLICES; sIndex++)
	{
		if (subject->sliceAllocArena[sIndex] != nullptr)
		{
			if (subject->sliceAllocAlignment[sIndex] == UINTXX_MAX)
			{
				FreeMem(subject->sliceAllocArena[sIndex], subject->slices[sIndex].pntr, subject->slices[sIndex].length);
			}
			else
			{
				FreeMemAligned(subject->sliceAllocArena[sIndex], subject->slices[sIndex].pntr, subject->slices[sIndex].length, subject->sliceAllocAlignment[sIndex]);
			}
		}
	}
	ClearPointer(subject);
}

// This is just shorthand for some convenience functionality.
// If you call this you can allocate enough space for a particular "type" from "arena" and store it in the "subject".
// This can be obtained in the workItemFunc with GetStructInWorkSubject(type, ...),
// AND it will get freed from "arena" automatically when FreeWorkSubject(...) is called
PEXPI void* AllocStructInWorkSubject_(Arena* arena, WorkSubject* subject, uxx sliceIndex, uxx typeSize, uxx typeAlignment)
{
	Assert(sliceIndex < WORK_SUBJECT_NUM_SLICES);
	void* result = AllocMemAligned(arena, typeSize, typeAlignment);
	if (result == nullptr) { return result; }
	subject->slices[sliceIndex].length = typeSize;
	subject->slices[sliceIndex].pntr = result;
	subject->sliceAllocArena[sliceIndex] = arena;
	subject->sliceAllocAlignment[sliceIndex] = typeAlignment;
	return result;
}
PEXPI void* GetStructInWorkSubject_(WorkSubject* subject, uxx sliceIndex, uxx typeSize, uxx typeAlignment)
{
	Assert(sliceIndex < WORK_SUBJECT_NUM_SLICES);
	Assert(subject->slices[sliceIndex].pntr == nullptr || subject->slices[sliceIndex].length == typeSize);
	Assert(subject->slices[sliceIndex].pntr == nullptr || subject->sliceAllocArena[sliceIndex] == nullptr || subject->sliceAllocAlignment[sliceIndex] == typeAlignment);
	return subject->slices[sliceIndex].pntr;
}

PEXPI Str8 AllocStringInWorkSubject(Arena* arena, WorkSubject* subject, uxx stringIndex, Str8 stringValue, bool addNullTerm)
{
	Assert(stringIndex < WORK_SUBJECT_NUM_SLICES);
	subject->strings[stringIndex] = Str8_Empty;
	subject->sliceAllocArena[stringIndex] = nullptr;
	if (stringValue.length > 0 || addNullTerm)
	{
		subject->strings[stringIndex].chars = (char*)AllocMem(arena, stringValue.length + (addNullTerm ? 1 : 0));
		if (subject->strings[stringIndex].chars != nullptr)
		{
			MyMemCopy(subject->strings[stringIndex].chars, stringValue.chars, stringValue.length);
			if (addNullTerm) { subject->strings[stringIndex].chars[stringValue.length] = '\0'; }
			subject->strings[stringIndex].length = stringValue.length;
			subject->sliceAllocArena[stringIndex] = arena;
			subject->sliceAllocAlignment[stringIndex] = UINTXX_MAX;
			subject->sliceHasNullTerm[stringIndex] = addNullTerm;
		}
	}
	return subject->strings[stringIndex];
}

#endif //PIG_CORE_IMPLEMENTATION

#endif //  _STRUCT_WORK_SUBJECT_H
