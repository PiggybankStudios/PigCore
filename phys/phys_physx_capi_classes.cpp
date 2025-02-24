/*
File:   phys_physx_capi_classes.cpp
Author: Taylor Robbins
Date:   02\23\2025
Description: 
	** Holds various class implementations that we need to implement the C++ side of the PhysX CAPI
*/

// +--------------------------------------------------------------+
// |          PigCorePhysXErrorCallback Implementations           |
// +--------------------------------------------------------------+
PigCorePhysXErrorCallback::PigCorePhysXErrorCallback()
{
}

PigCorePhysXErrorCallback::~PigCorePhysXErrorCallback()
{
}

void PigCorePhysXErrorCallback::reportError(physx::PxErrorCode::Enum e, const char* message, const char* file, int line)
{
	AssertMsg(false, "PhysX Error Reported!");
}


// +--------------------------------------------------------------+
// |            PigCorePhysXAllocator Implementations             |
// +--------------------------------------------------------------+
PigCorePhysXAllocator::PigCorePhysXAllocator(Arena* arenaPntr)
{
	NotNull(arenaPntr);
	Assert(IsFlagSet(arenaPntr->flags, ArenaFlag_AllowFreeWithoutSize));
	this->arena = arenaPntr;
}

void* PigCorePhysXAllocator::allocate(size_t numBytes, const char* filePath, const char* funcName, int lineNumber)
{
	NotNull(arena);
	Assert(numBytes <= UINTXX_MAX);
	UNUSED(filePath);
	UNUSED(funcName);
	UNUSED(lineNumber);
	void* result = AllocMemAligned(this->arena, (uxx)numBytes, PHYSX_ALLOCATOR_ALIGNMENT);
	return result;
}

void PigCorePhysXAllocator::deallocate(void* allocPntr)
{
	NotNull(arena);
	if (allocPntr != nullptr && CanArenaFree(this->arena))
	{
		FreeMemAligned(this->arena, allocPntr, 0, PHYSX_ALLOCATOR_ALIGNMENT);
	}
}

