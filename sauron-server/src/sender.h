#pragma once

void sender(pid_t shmem_key);

void send_loop(pid_t shmem_key, int connected, sockaddr_in client_addr);
