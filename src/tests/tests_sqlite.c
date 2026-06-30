/*
File:   tests_sqlite.c
Author: Taylor Robbins
Date:   07\30\2025
Description: 
	** Holds a bunch of helper functions for SQLite file system routing and a function
	** to test SQLite functionality at the bottom
*/

#if SQLITE_ENABLED

static sqlite3_io_methods SqliteVfsMethods = ZEROED;

typedef struct SqliteFileHandle SqliteFileHandle;
struct SqliteFileHandle
{
	sqlite3_file base; //NOTE: This has to be placed at the beginning of the struct
	int flags;
	OsFile file;
};

static int SqliteCallback(void *NotUsed, int argc, char** argv, char** azColName)
{
	UNUSED(NotUsed);
	for (int i=0; i < argc; i++)
	{
		PrintLine_E("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	return 0;
}

int Sqlite_FileOpen(sqlite3_vfs* fileSystem, sqlite3_filename filePathPntr, sqlite3_file* filePntr, int flags, int* outFlagsPntr)
{
	NotNull(filePathPntr);
	NotNull(filePntr);
	FilePath filePath = FilePathLit(filePathPntr);
	PrintLine_D("Sqlite_FileOpen(%p, \"%s\", %p, %d, %p)", fileSystem, filePathPntr, filePntr, flags, outFlagsPntr);
	AssertMsg(!IsFlagSet(flags, SQLITE_OPEN_READONLY) || !IsFlagSet(flags, SQLITE_OPEN_READWRITE), "Exactly one of the READWRITE and READONLY flags must be set");
	AssertMsg(IsFlagSet(flags, SQLITE_OPEN_READONLY) || IsFlagSet(flags, SQLITE_OPEN_READWRITE), "Exactly one of the READWRITE and READONLY flags must be set");
	AssertMsg(!IsFlagSet(flags, SQLITE_OPEN_CREATE) || IsFlagSet(flags, SQLITE_OPEN_READWRITE), "if CREATE is set, then READWRITE must also be set");
	AssertMsg(!IsFlagSet(flags, SQLITE_OPEN_EXCLUSIVE) || IsFlagSet(flags, SQLITE_OPEN_CREATE), "if EXCLUSIVE is set, then CREATE must also be set");
	AssertMsg(!IsFlagSet(flags, SQLITE_OPEN_DELETEONCLOSE) || IsFlagSet(flags, SQLITE_OPEN_CREATE), "if DELETEONCLOSE is set, then CREATE must also be set");
	OsOpenFileMode openMode = IsFlagSet(flags, SQLITE_OPEN_READONLY) ? OsOpenFileMode_Read : OsOpenFileMode_Append;
	if (IsFlagSet(flags, SQLITE_OPEN_CREATE)) { openMode = OsOpenFileMode_Write; }
	//TODO: Add support for SQLITE_OPEN_EXCLUSIVE. OsOpenFile needs to have this option
	SqliteFileHandle result = ZEROED;
	bool openResult = OsOpenFile(stdHeap, filePath, openMode, true, &result.file);
	if (openResult == false) { return SQLITE_CANTOPEN; }
	result.flags = flags;
	result.base.pMethods = &SqliteVfsMethods;
	MyMemCopy(filePntr, &result, sizeof(SqliteFileHandle));
	SetOptionalOutPntr(outFlagsPntr, flags);
	PrintLine_D("Sqlite_FileOpen(...) -> %p", filePntr);
	return SQLITE_OK;
}

int Sqlite_FileClose(sqlite3_file* filePntr)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	NotNull(fileHandle->file.arena);
	PrintLine_D("Sqlite_FileClose(%p)", filePntr);
	OsCloseFile(&fileHandle->file);
	//TODO: Should we delete the file if SQLITE_OPEN_DELETEONCLOSE is set?
	return SQLITE_OK;
}

int Sqlite_FileRead(sqlite3_file* filePntr, void* bufferPntr, int numBytes, sqlite3_int64 offset)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	Assert(numBytes >= 0);
	Assert(bufferPntr != nullptr || numBytes == 0);
	PrintLine_D("Sqlite_FileRead(%p, %p, %d, %lld)", filePntr, bufferPntr, numBytes, (i64)offset);
	
	if (offset != 0)
	{
		//TODO: Implement OsMoveFileCursor and handle offset
		AssertMsg(offset == 0, "We don't handle offsets in Sqlite_FileRead yet!");
	}
	
	uxx numBytesRead = 0;
	bool readResult = OsReadFromOpenFile(&fileHandle->file, (uxx)numBytes, false, bufferPntr, &numBytesRead);
	if (readResult == false) { return SQLITE_IOERR; }
	if (numBytesRead < (uxx)numBytes) { return SQLITE_ERROR; }
	
	return SQLITE_OK;
}

int Sqlite_FileWrite(sqlite3_file* filePntr, const void* bytesPntr, int numBytes, sqlite3_int64 offset)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	Assert(numBytes >= 0);
	Assert(bytesPntr != nullptr || numBytes == 0);
	PrintLine_D("Sqlite_FileWrite(%p, %p, %d, %lld)", filePntr, bytesPntr, numBytes, (i64)offset);
	
	if (offset != 0)
	{
		//TODO: Implement OsMoveFileCursor and handle offset
		AssertMsg(offset == 0, "We don't handle offsets in Sqlite_FileWrite yet!");
	}
	
	if (numBytes == 0) { return SQLITE_OK; }
	
	bool writeResult = OsWriteToOpenFile(&fileHandle->file, NewStr8(numBytes, bytesPntr), false);
	if (writeResult == false) { return SQLITE_IOERR; }
	
	return SQLITE_OK;
}

int Sqlite_FileTruncate(sqlite3_file* filePntr, sqlite3_int64 size)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileTruncate(%p, %lld)", filePntr, (i64)size);
	AssertMsg((uxx)size == fileHandle->file.fileSize, "Sqlite_FileTruncate is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}

int Sqlite_FileSync(sqlite3_file* filePntr, int flags)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileSync(%p, %d)", filePntr, flags);
	AssertMsg(false, "Sqlite_FileSync is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}

int Sqlite_FileSize(sqlite3_file* filePntr, sqlite3_int64* sizeOut)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	Assert(fileHandle->file.isKnownSize);
	PrintLine_D("Sqlite_FileSize(%p, %p)", filePntr, sizeOut);
	SetOptionalOutPntr(sizeOut, (i64)fileHandle->file.fileSize);
	return SQLITE_OK;
}

int Sqlite_FileLock(sqlite3_file* filePntr, int lockType)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileLock(%p, %d)", filePntr, lockType);
	AssertMsg(false, "Sqlite_FileLock is not implemented yet!");
	//TODO: Implement me! lockType is SQLITE_LOCK_SHARED, SQLITE_LOCK_RESERVED, SQLITE_LOCK_PENDING, or SQLITE_LOCK_EXCLUSIVE (never SQLITE_LOCK_NONE)
	return SQLITE_OK;
}

int Sqlite_FileUnlock(sqlite3_file* filePntr, int unlockType)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileUnlock(%p, %d)", filePntr, unlockType);
	AssertMsg(false, "Sqlite_FileUnlock is not implemented yet!");
	//TODO: Implement me! unlockType is SQLITE_LOCK_SHARED, SQLITE_LOCK_RESERVED, SQLITE_LOCK_PENDING, SQLITE_LOCK_EXCLUSIVE, or SQLITE_LOCK_NONE
	return SQLITE_OK;
}

int Sqlite_FileCheckReservedLock(sqlite3_file* filePntr, int* resultOutPntr)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileCheckReservedLock(%p, %p)", filePntr, resultOutPntr);
	AssertMsg(false, "Sqlite_FileCheckReservedLock is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}

int Sqlite_FileControl(sqlite3_file* filePntr, int controlOp, void* valuePntr)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	PrintLine_D("Sqlite_FileControl(%p, %d, %p)", filePntr, controlOp, valuePntr);
	AssertMsg(false, "Sqlite_FileControl is not implemented yet!");
	//TODO: Implement me! controlOp is SQLITE_FCNTL_x
	return SQLITE_ERROR;
}

// int Sqlite_FileSectorSize(sqlite3_file* filePntr)
// {
// 	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
// 	NotNull(fileHandle);
// 	Assert(fileHandle->file.isOpen);
// 	AssertMsg(false, "Sqlite_FileSectorSize is not implemented yet!");
// 	//TODO: Implement me!
// 	return SQLITE_ERROR;
// }

int Sqlite_FileDeviceCharacteristics(sqlite3_file* filePntr)
{
	UNUSED(filePntr);
	// SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	PrintLine_D("Sqlite_FileDeviceCharacteristics(%p)", filePntr);
	return SQLITE_IOCAP_SUBPAGE_READ; //TODO: Any other capability flags we should return?
}

#if 0
int Sqlite_FileShmMap(sqlite3_file* filePntr, int iPg, int pgsz, int, void volatile** outMappingPntr)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileShmMap is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
int Sqlite_FileShmLock(sqlite3_file* filePntr, int offset, int n, int flags)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileShmLock is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
void Sqlite_FileShmBarrier(sqlite3_file* filePntr)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileShmBarrier is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
int Sqlite_FileShmUnmap(sqlite3_file* filePntr, int deleteFlag)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileShmUnmap is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
int Sqlite_FileFetch(sqlite3_file* filePntr, sqlite3_int64 iOfst, int iAmt, void** pp)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileFetch is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
int Sqlite_FileUnfetch(sqlite3_file* filePntr, sqlite3_int64 iOfst, void* p)
{
	SqliteFileHandle* fileHandle = (SqliteFileHandle*)filePntr;
	NotNull(fileHandle);
	Assert(fileHandle->file.isOpen);
	AssertMsg(false, "Sqlite_FileUnfetch is not implemented yet!");
	//TODO: Implement me!
	return SQLITE_ERROR;
}
#endif

int Sqlite_FileDelete(sqlite3_vfs* fileSystem, const char* zName, int syncDir)
{
	PrintLine_D("Sqlite_FileDelete(%p, \"%s\", %d)", fileSystem, zName, syncDir);
	//TODO: Implement me!
	return 0;
}

int Sqlite_FileAccess(sqlite3_vfs* fileSystem, const char* zName, int flags, int* pResOut)
{
	PrintLine_D("Sqlite_FileAccess(%p, \"%s\", %d, %p)", fileSystem, zName, flags, pResOut);
	//TODO: Implement me!
	return 0;
}

int Sqlite_FileFullPathname(sqlite3_vfs* fileSystem, const char* zName, int bufferSize, char* bufferPntr)
{
	// PrintLine_D("Sqlite_FileFullPathname(%p, \"%s\", %d, %p)", fileSystem, zName, bufferSize, bufferPntr);
	//TODO: Should we maybe use a scratch arena for allocation and then copy into the bufferPntr afterwards?
	NotNull(zName);
	Assert(bufferSize > 0);
	NotNull(bufferPntr);
	Arena bufferArena = ZEROED;
	InitArenaBuffer(&bufferArena, bufferPntr, (uxx)bufferSize);
	FilePath fullPath = OsGetFullPath(&bufferArena, FilePathLit(zName));
	Assert(fullPath.length == 0 || fullPath.chars == bufferPntr);
	Assert(fullPath.length < (uxx)bufferSize);
	if (fullPath.chars != nullptr) { fullPath.chars[fullPath.length] = '\0'; }
	// PrintLine_D("Sqlite_FileFullPathname(...) -> \"%.*s\"", StrPrint(fullPath));
	return SQLITE_OK;
}

void* Sqlite_FileDlOpen(sqlite3_vfs* fileSystem, const char* zFilename)
{
	PrintLine_D("Sqlite_FileDlOpen(%p, \"%s\")", fileSystem, zFilename);
	//TODO: Implement me!
	return nullptr;
}

void Sqlite_FileDlError(sqlite3_vfs* fileSystem, int nByte, char* zErrMsg)
{
	PrintLine_D("Sqlite_FileDlError(%p, %d, \"%s\")", fileSystem, nByte, zErrMsg);
	//TODO: Implement me!
}

// void Sqlite_FileDlSym(sqlite3_vfs* fileSystem, void* soemthing, const char* zSymbol)
// {
// 	//TODO: Implement me!
// }

void Sqlite_FileDlClose(sqlite3_vfs* fileSystem, void* pntr)
{
	PrintLine_D("Sqlite_FileDlClose(%p, %p)", fileSystem, pntr);
	//TODO: Implement me!
}

int Sqlite_FileRandomness(sqlite3_vfs* fileSystem, int nByte, char* zOut)
{
	PrintLine_D("Sqlite_FileRandomness(%p, %d, %p)", fileSystem, nByte, zOut);
	//TODO: Implement me!
	return 0;
}

int Sqlite_FileSleep(sqlite3_vfs* fileSystem, int microseconds)
{
	PrintLine_D("Sqlite_FileSleep(%p, %d)", fileSystem, microseconds);
	//TODO: Implement me!
	return 0;
}

int Sqlite_FileCurrentTime(sqlite3_vfs* fileSystem, double* timeOut)
{
	PrintLine_D("Sqlite_FileCurrentTime(%p, %p)", fileSystem, timeOut);
	//TODO: Implement me!
	return 0;
}

int Sqlite_FileGetLastError(sqlite3_vfs* fileSystem, int something, char* errorStrOut)
{
	PrintLine_D("Sqlite_FileGetLastError(%p, %d, %p)", fileSystem, something, errorStrOut);
	//TODO: Implement me!
	return 0;
}

// int Sqlite_FileCurrentTimeInt64(sqlite3_vfs* fileSystem, sqlite3_int64* timeOut)
// {
// 	//TODO: Implement me!
// 	return 0;
// }

// int Sqlite_FileSetSystemCall(sqlite3_vfs* fileSystem, const char* zName, sqlite3_syscall_ptr sysCallPntr)
// {
// 	//TODO: Implement me!
// 	return 0;
// }

// sqlite3_syscall_ptr Sqlite_FileGetSystemCall(sqlite3_vfs* fileSystem, const char* zName)
// {
// 	//TODO: Implement me!
// 	return nullptr;
// }

// const char* Sqlite_FileNextSystemCall(sqlite3_vfs* fileSystem, const char* zName)
// {
// 	//TODO: Implement me!
// 	return nullptr;
// }

void TestSqlite()
{
	SqliteVfsMethods.iVersion = 1;
	SqliteVfsMethods.xClose = Sqlite_FileClose; // int (*xClose)(sqlite3_file*);
	SqliteVfsMethods.xRead = Sqlite_FileRead; // int (*xRead)(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
	SqliteVfsMethods.xWrite = Sqlite_FileWrite; // int (*xWrite)(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
	SqliteVfsMethods.xTruncate = Sqlite_FileTruncate; // int (*xTruncate)(sqlite3_file*, sqlite3_int64 size);
	SqliteVfsMethods.xSync = Sqlite_FileSync; // int (*xSync)(sqlite3_file*, int flags);
	SqliteVfsMethods.xFileSize = Sqlite_FileSize; // int (*xFileSize)(sqlite3_file*, sqlite3_int64 *pSize);
	SqliteVfsMethods.xLock = Sqlite_FileLock; // int (*xLock)(sqlite3_file*, int);
	SqliteVfsMethods.xUnlock = Sqlite_FileUnlock; // int (*xUnlock)(sqlite3_file*, int);
	SqliteVfsMethods.xCheckReservedLock = Sqlite_FileCheckReservedLock; // int (*xCheckReservedLock)(sqlite3_file*, int *pResOut);
	SqliteVfsMethods.xFileControl = Sqlite_FileControl; // int (*xFileControl)(sqlite3_file*, int op, void *pArg);
	// SqliteVfsMethods.xSectorSize = Sqlite_FileSectorSize; // int (*xSectorSize)(sqlite3_file*);
	SqliteVfsMethods.xDeviceCharacteristics = Sqlite_FileDeviceCharacteristics; // int (*xDeviceCharacteristics)(sqlite3_file*);
	// SqliteVfsMethods.xShmMap = Sqlite_FileShmMap; // int (*xShmMap)(sqlite3_file*, int iPg, int pgsz, int, void volatile**);
	// SqliteVfsMethods.xShmLock = Sqlite_FileShmLock; // int (*xShmLock)(sqlite3_file*, int offset, int n, int flags);
	// SqliteVfsMethods.xShmBarrier = Sqlite_FileShmBarrier; // void (*xShmBarrier)(sqlite3_file*);
	// SqliteVfsMethods.xShmUnmap = Sqlite_FileShmUnmap; // int (*xShmUnmap)(sqlite3_file*, int deleteFlag);
	// SqliteVfsMethods.xFetch = Sqlite_FileFetch; // int (*xFetch)(sqlite3_file*, sqlite3_int64 iOfst, int iAmt, void **pp);
	// SqliteVfsMethods.xUnfetch = Sqlite_FileUnfetch; // int (*xUnfetch)(sqlite3_file*, sqlite3_int64 iOfst, void *p);
	
	sqlite3_vfs virtualFileSystem = ZEROED;
	virtualFileSystem.iVersion = 1;
	virtualFileSystem.szOsFile = sizeof(SqliteFileHandle);
	virtualFileSystem.mxPathname = 1024;
	virtualFileSystem.zName = "PigCore";
	virtualFileSystem.xOpen = Sqlite_FileOpen; // int (*xOpen)(sqlite3_vfs*, sqlite3_filename zName, sqlite3_file*, int flags, int *pOutFlags);
	virtualFileSystem.xDelete = Sqlite_FileDelete; // int (*xDelete)(sqlite3_vfs*, const char *zName, int syncDir);
	virtualFileSystem.xAccess = Sqlite_FileAccess; // int (*xAccess)(sqlite3_vfs*, const char *zName, int flags, int *pResOut);
	virtualFileSystem.xFullPathname = Sqlite_FileFullPathname; // int (*xFullPathname)(sqlite3_vfs*, const char *zName, int nOut, char *zOut);
	virtualFileSystem.xDlOpen = Sqlite_FileDlOpen; // void *(*xDlOpen)(sqlite3_vfs*, const char *zFilename);
	virtualFileSystem.xDlError = Sqlite_FileDlError; // void (*xDlError)(sqlite3_vfs*, int nByte, char *zErrMsg);
	// virtualFileSystem.xDlSym = Sqlite_FileDlSym; // void (*(*xDlSym)(sqlite3_vfs*,void*, const char *zSymbol))(void);
	virtualFileSystem.xDlClose = Sqlite_FileDlClose; // void (*xDlClose)(sqlite3_vfs*, void*);
	virtualFileSystem.xRandomness = Sqlite_FileRandomness; // int (*xRandomness)(sqlite3_vfs*, int nByte, char *zOut);
	virtualFileSystem.xSleep = Sqlite_FileSleep; // int (*xSleep)(sqlite3_vfs*, int microseconds);
	virtualFileSystem.xCurrentTime = Sqlite_FileCurrentTime; // int (*xCurrentTime)(sqlite3_vfs*, double*);
	virtualFileSystem.xGetLastError = Sqlite_FileGetLastError; // int (*xGetLastError)(sqlite3_vfs*, int, char *);
	// virtualFileSystem.xCurrentTimeInt64 = Sqlite_FileCurrentTimeInt64; // int (*xCurrentTimeInt64)(sqlite3_vfs*, sqlite3_int64*);
	// virtualFileSystem.xSetSystemCall = Sqlite_FileSetSystemCall; // int (*xSetSystemCall)(sqlite3_vfs*, const char *zName, sqlite3_syscall_ptr);
	// virtualFileSystem.xGetSystemCall = Sqlite_FileGetSystemCall; // sqlite3_syscall_ptr (*xGetSystemCall)(sqlite3_vfs*, const char *zName);
	// virtualFileSystem.xNextSystemCall = Sqlite_FileNextSystemCall; // const char *(*xNextSystemCall)(sqlite3_vfs*, const char *zName);
	
	WriteLine_D("Registering SQLite vfs...");
	sqlite3_vfs_register(&virtualFileSystem, 1);
	
	WriteLine_D("Opening SQLite DB...");
	sqlite3* database = nullptr;
	int openResult = sqlite3_open_v2("test_database", &database, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE, 0);
	if (openResult != 0)
	{
		PrintLine_E("Failed to open SQLite database: %s", sqlite3_errmsg(database));
		sqlite3_close(database);
		return 1;
	}
	
	char* errorMsg = nullptr;
	WriteLine_D("Creating table...");
	int execResult1 = sqlite3_exec(database, "CREATE TABLE table1 (id INT, name VARCHAR(255))", SqliteCallback, 0, &errorMsg);
	if (execResult1 != SQLITE_OK) { PrintLine_E("sqlite3_exec[0] failed: %s", errorMsg); sqlite3_free(errorMsg); }
	WriteLine_D("Adding row...");
	int execResult2 = sqlite3_exec(database, "INSERT INTO table1 (id, name) VALUES (42, \"Taylor\")", SqliteCallback, 0, &errorMsg);
	if (execResult2 != SQLITE_OK) { PrintLine_E("sqlite3_exec[1] failed: %s", errorMsg); sqlite3_free(errorMsg); }
	
	sqlite3_close(database);
	WriteLine_D("DONE!");
}

#endif //SQLITE_ENABLED
