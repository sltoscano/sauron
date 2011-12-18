/*
 *  shmem.h
 *  updater
 *
 *  Created by Steven Toscano on 2/21/11.
 *  Copyright 2011 TriSource Software. All rights reserved.
 *
 */

#include <sys/types.h>

bool create_memory_map(const char* shared_name, int64_t sizeBytes, void** addr, int* pfd);

bool destroy_memory_map(const char* shared_name, int64_t sizeBytes, void* addr, int fd);

bool open_memory_map(const char* shared_name, int64_t& sizeBytes, void** addr, int* pfd);

bool close_memory_map(int64_t sizeBytes, void* addr, int fd);
