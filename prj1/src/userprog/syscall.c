#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <string.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "devices/input.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  switch(*(int32_t*)(f->esp)){
    case SYS_HALT:
      shutdown_power_off();
      break;
    case SYS_EXIT:
      check_addr(f->esp + 4);
      exit(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_EXEC:
      check_addr(f->esp + 4);
      f->eax = process_execute((char *)(*(uint32_t *)(f->esp + 4)));
      break;
    case SYS_WAIT:
      check_addr(f->esp + 4);
      f->eax = process_wait((*(uint32_t *)(f->esp + 4)));
      break;
    case SYS_WRITE:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      f->eax = write((int)*(uint32_t*)(f->esp + 4), (const void*)*(uint32_t*)(f->esp + 8), (unsigned)*(uint32_t*)(f->esp + 12));
      break;
    case SYS_READ:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      f->eax = read((int)*(uint32_t*)(f->esp + 4), (void*)*(uint32_t*)(f->esp + 8), (unsigned)*(uint32_t*)(f->esp + 12));
      break;
    case SYS_FIBONACCI:
      check_addr(f->esp + 4);
      f->eax = fibonacci((int)*(uint32_t*)(f->esp + 4));
      break;
    case SYS_MAX_OF_FOUR_INT:
      check_addr(f->esp + 4);
      check_addr(f->esp + 8);
      check_addr(f->esp + 12);
      check_addr(f->esp + 16);
      f->eax = max_of_four_int((int)*(uint32_t*)(f->esp + 4), (int)*(uint32_t*)(f->esp + 8), (int)*(uint32_t*)(f->esp + 12), (int)*(uint32_t*)(f->esp + 16));
      break;
  }
  //thread_exit ();
}

// The function which checks the validity of given address
// userprog/pagedir.c and threads/vaddr.h
void exit(int status){
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status=status;
  thread_exit();
}

void check_addr(const void *vaddr)
{
  if(vaddr == NULL) {
    exit(-1);
  }
  if(is_kernel_vaddr(vaddr)) {
    exit(-1);
  }
  if(!pagedir_get_page(thread_current()->pagedir, vaddr)) {
    exit(-1);
  }
}

int read (int fd, void *buffer, unsigned length)
{
  if(fd != 0) return -1;
  unsigned cnt = 1;
  while(cnt <= length) {
    *(uint8_t*)(buffer + cnt -1) = input_getc();
  }
  return cnt;
}

int write (int fd, const void *buffer, unsigned length)
{
  if (fd != 1) return -1;
  putbuf(buffer, length);
  return length;
}

int fibonacci(int num)
{
  if (num == 0) return 0;
  if (num == 1) return 1;
  return fibonacci(num-1) + fibonacci(num-2);
}

int max_of_four_int(int a, int b, int c, int d)
{
  int max1 = 0, max2 = 0;
  if (a > b) max1 = a;
  else max1 = b;
  if (c > d) max2 = c;
  else max2 = d;
  if (max1 > max2) return max1;
  else return max2; 
}