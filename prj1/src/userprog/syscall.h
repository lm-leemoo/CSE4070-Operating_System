#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void check_addr(const void *);
int read(int fd, void *buffer, unsigned int size);
int write(int fd, const void* buffer, unsigned int size);
void exit(int status);
int fibonacci(int num);
int max_of_four_int(int a, int b, int c, int d);
#endif /* userprog/syscall.h */
