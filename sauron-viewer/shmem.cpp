
#include "stdafx.h"
#include "shmem.h"


bool create_memmap_file(HANDLE& hMMFile, const char* filename, const __int64 size)
{
	LARGE_INTEGER size64;
	size64.QuadPart = size;

	hMMFile = INVALID_HANDLE_VALUE;
	hMMFile = CreateFileMappingA(INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		size64.HighPart,
		size64.LowPart,
		filename);

	if (hMMFile == NULL || hMMFile == INVALID_HANDLE_VALUE)
	{
		hMMFile = NULL;
		return false;
	}

	return true;
}
