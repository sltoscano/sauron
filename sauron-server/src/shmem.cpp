/*
 *  shmem.cpp
 *  updater
 *
 *  Created by Steven Toscano on 2/21/11.
 *  Copyright 2011 TriSource Software. All rights reserved.
 *
 */

#include "shmem.h"

#include <stdio.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>


bool create_memory_map(const char* shared_name, __int64_t sizeBytes, void** addr, int* pfd)
{
	shm_unlink(shared_name);
	
	*pfd = shm_open(shared_name, O_RDWR|O_CREAT, S_IRWXG|S_IRWXU|S_IRWXO);
	if (*pfd == -1 || ftruncate(*pfd, sizeBytes) == -1)
	{
		printf("\nError opening/trunc file: %s (%s)\n", shared_name, strerror(errno));
		return false;
	}
	
	*addr = mmap(NULL, sizeBytes, PROT_READ|PROT_WRITE, MAP_SHARED, *pfd, 0);
	if (*addr == MAP_FAILED)
	{
		close(*pfd);
		shm_unlink(shared_name);
		printf("\nError opening memmap: %s (%s)\n", shared_name, strerror(errno));
		return false;
	}
	return true;
}

bool destroy_memory_map(const char* shared_name, __int64_t sizeBytes, void* addr, int fd)
{
	if (munmap(addr, sizeBytes) == -1)
	{
		printf("\nError unmapping mem: %s (%s)\n", shared_name, strerror(errno));
		return false;
	}
	
	if (close(fd) == -1)
	{
		printf("\nError closing shared file: %s (%s)\n", shared_name, strerror(errno));
		return false;
	}
	
	if (shm_unlink(shared_name) == -1)
	{
		printf("\nError unlinking shared mem: %s (%s)\n", shared_name, strerror(errno));
		return false;
	}
	
	return true;
}

bool open_memory_map(const char* shared_name, __int64_t& sizeBytes, void** addr, int* pfd)
{
	*pfd = shm_open(shared_name, O_EXCL|O_CREAT, S_IRWXG|S_IRWXU|S_IRWXO);
	if (*pfd == -1 && errno == EEXIST)
	{
		*pfd = shm_open(shared_name, O_RDWR, S_IRWXG|S_IRWXU|S_IRWXO);
		if (*pfd == -1)
		{
			printf("\nError opening file: %s (%s)\n", shared_name, strerror(errno));
			return false;
		}
		
		// Force proper alignment otherwise mmap will fail
		int align = getpagesize() - 1;
		sizeBytes = (sizeBytes + align) & ~align;
		
		int infoSize = 0;
		sizeBytes += infoSize;
		
		struct stat st;
		fstat(*pfd, &st);
		if (st.st_size != sizeBytes)
		{
			printf("warning, %s file size doesn't match (got %d, expected %lld)\n", shared_name, (int)st.st_size, sizeBytes);
			sizeBytes = st.st_size;
		}
		
		off_t offset = infoSize;

		// Force proper alignment otherwise mmap will fail
		offset = (offset + align) & ~align;

		sizeBytes -= infoSize;
		
		*addr = mmap(NULL, sizeBytes, PROT_READ|PROT_WRITE, MAP_SHARED, *pfd, offset);
		if (*addr == MAP_FAILED)
		{
			close(*pfd);
			printf("\nError opening memmap: %s (%s)\n", shared_name, strerror(errno));
			return false;
		}
	}
	else {
		printf("\nCouldn't find existing shared memory object : %s\n", shared_name);
		close(*pfd);
		shm_unlink(shared_name);
		return false;
	}

	return true;
}

bool close_memory_map(__int64_t sizeBytes, void* addr, int fd)
{
	struct stat st;
	fstat(fd, &st);
	if (st.st_size != sizeBytes)
		sizeBytes = st.st_size;
	
	if (munmap(addr, sizeBytes) == -1)
	{
		printf("\nError unmapping mem (%s)\n", strerror(errno));
		return false;
	}
	
	if (close(fd) == -1)
	{
		printf("\nError closing shared file (%s)\n", strerror(errno));
		return false;
	}
	
	return true;
}
