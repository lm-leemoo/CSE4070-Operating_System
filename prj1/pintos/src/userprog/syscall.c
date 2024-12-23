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

static void syscall_handler (struct intr_frame *);
void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf("wow%d\n", *(int32_t*)(f->esp));
  // switch(*(int32_t*)(f->esp)){
  //   case SYS_HALT:
  //     shutdown_power_off();
  //     break;
  //   case SYS_EXIT:
  //     check_addr((const void *)(*(uint32_t *)(f->esp + 4)));
  //     exit(*(uint32_t*)(f->esp + 4));
  //     thread_exit();
  //     break;
  //   case SYS_EXEC:
  //     check_addr((const void *)(*(uint32_t *)(f->esp + 4)));
  //     f->eax = process_execute((char *)(*(uint32_t *)(f->esp + 4)));
  //     break;
  //   case SYS_WRITE:
  //     printf("write system call!\n");
  //     check_addr(f->esp+4);
  //     check_addr(f->esp+8);
  //     check_addr(f->esp+12);
  //     f->eax = write((int)*(uint32_t*)(f->esp+4), (const void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
  //   break;
  //   case SYS_READ:
  //     printf("read system call!\n");
  //     check_addr(f->esp+4);
  //     check_addr(f->esp+8);
  //     check_addr(f->esp+12);
  //     f->eax = read((int)*(uint32_t*)(f->esp+4), (void*)*(uint32_t*)(f->esp+8), (unsigned)*(uint32_t*)(f->esp+12));
  //     break;
  // }
  //printf ("system call!\n");
  thread_exit ();
}

// The function which checks the validity of given address
// userprog/pagedir.c and threads/vaddr.h
void exit(int status)
{
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status = status;
  thread_exit();
}

void check_addr(const void *vaddr)
{
  //if(vaddr == NULL) exit(-1);
  if(is_kernel_vaddr(vaddr)) exit(-1);
  //if(!pagedir_get_page(thread_current()->pagedir, vaddr) == NULL) exit(-1);
}


int read (int fd, void *buffer, unsigned length)
{
  unsigned Count = 0;
  if(!fd)
  {
    while(Count < length)
    {
      if((((char *)buffer)[Count] = input_getc()) == '\0')
        break;
      Count++;
    }
  }
  return Count;
}
int write (int fd, const void *buffer, unsigned length)
{
  if(fd == 1)
  {
    putbuf(buffer, length);
    return length;
  }
  else return -1;
}