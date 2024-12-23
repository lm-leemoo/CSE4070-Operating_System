#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#include <stdbool.h>
#include "threads/thread.h"
void syscall_init (void);
void check_addr(const void *);
int read(int fd, void *buffer, unsigned int size);
int write(int fd, const void* buffer, unsigned int size);
void exit(int status);
int fibonacci(int num);
int max_of_four_int(int a, int b, int c, int d);
int open(const char *file);
bool create (const char *file, unsigned initial_size);
bool remove(const char *file);
void close(int fd);
int filesize(int fd);
void seek (int fd, unsigned int position);
unsigned int tell(int fd);
tid_t exec (const char *file);
int wait (tid_t pid);
void halt(void);
#endif /* userprog/syscall.h */
