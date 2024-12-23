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
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/off_t.h"
struct file 
  {
    struct inode *inode;
    off_t pos;
    bool deny_write;
  };
struct lock filelock;

static void syscall_handler (struct intr_frame *);
void
syscall_init (void) 
{
  lock_init(&filelock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  switch(*(int32_t*)(f->esp)){
    case SYS_HALT:
      halt();
      break;
    case SYS_EXIT:
      check_addr(f->esp + 4);
      exit(*(uint32_t*)(f->esp + 4));
      break;
    case SYS_EXEC:
      check_addr(f->esp + 4);
      f->eax= exec((const char *)(char *)(*(uint32_t *)(f->esp + 4)));
      break;
    case SYS_WAIT:
      check_addr(f->esp + 4);
      f->eax = wait((*(uint32_t *)(f->esp + 4)));
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
    //proj2
    case SYS_OPEN:
      check_addr(f->esp + 4);
      f->eax = open((char *)(*(uint32_t *)(f->esp + 4)));
      break;
    case SYS_CLOSE:
      check_addr(f->esp + 4);
      close((int)*(uint32_t*)(f->esp +4));
      break;
    case SYS_CREATE:
      check_addr(f->esp+4);
      check_addr(f->esp+8);
      f->eax = create((const char *)(*(uint32_t *)(f->esp+4)), (int)*(uint32_t*)(f->esp+8));
      break;
    case SYS_REMOVE:
      check_addr(f->esp+4);
      f->eax = remove((const char *)(*(uint32_t *)(f->esp+4)));
      break;
    case SYS_FILESIZE:
      check_addr(f->esp+4);
      f->eax = filesize((int)*(uint32_t*)(f->esp+4));
      break;
    case SYS_SEEK:
      check_addr(f->esp+4);
      check_addr(f->esp+8);
      seek((int)*(uint32_t*)(f->esp+4), (unsigned)*(uint32_t*)(f->esp+8));
      break;
    case SYS_TELL:
      check_addr(f->esp+4);
      f->eax = tell((int)*(uint32_t*)(f->esp+4));
      break;
  }
  //thread_exit ();
}

// The function which checks the validity of given address
// userprog/pagedir.c and threads/vaddr.h
void exit(int status){
  printf("%s: exit(%d)\n", thread_name(), status);
  thread_current()->exit_status=status;
  for(int i = 3; i < 126; i++) {
    if(thread_current()->filedescriptor[i] != NULL) {
      //printf("descripternum: %d\n", i);
      close(i);
    }
  }
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
  check_addr(buffer);
  if(pagedir_get_page(thread_current()->pagedir, buffer) == NULL) exit(-1);
  lock_acquire(&filelock);
  if(fd == 0) {//stdin
    unsigned cnt = 1;
    while(cnt <= length) {
      *(uint8_t*)(buffer + cnt -1) = input_getc();
    }
    lock_release(&filelock);
    return cnt;
  }
  if (2 < fd && fd < 126) { //file
    struct file* f = thread_current()->filedescriptor[fd];
    if (f == NULL) {
      lock_release(&filelock);
      exit(-1);
    }
    unsigned result = file_read(f, buffer, length);
    lock_release(&filelock);
    return result;
  }
  lock_release(&filelock);
  exit(-1);
}

int write (int fd, const void *buffer, unsigned length)
{
  check_addr(buffer);
  if(pagedir_get_page(thread_current()->pagedir, buffer) == NULL) exit(-1);
  lock_acquire(&filelock);
  if(fd == 1) {//stdout
    putbuf(buffer, length);
    lock_release(&filelock);
    return length;
  }
  if (2 < fd && fd < 126) { //file
    struct thread* t = thread_current();
    struct file* f = t->filedescriptor[fd];
    if (f == NULL) {
      lock_release(&filelock);
      exit(-1);
    }
    if(f->deny_write) file_deny_write(t->filedescriptor[fd]);
    unsigned result = file_write(f, buffer, length);
    lock_release(&filelock);
    return result;
  }
  lock_release(&filelock);
  exit(-1);
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

int open (const char *file)
{
  check_addr(file);
  int fd = -1;
  if(file == NULL) exit(-1);
  if(pagedir_get_page(thread_current()->pagedir, file) == NULL) exit(-1);
  lock_acquire(&filelock);
  struct file* f = filesys_open(file);
  if (f == NULL) {
    lock_release(&filelock);
    return -1;
  }
  for(int i = 3; i < 126; i++) {
    if(thread_current()->filedescriptor[i] == NULL) {
      if(!strcmp(thread_current()->name, file)) file_deny_write(f);
      fd = i;
      thread_current()->filedescriptor[i] = f;
      break;
    }
  }
  lock_release(&filelock);
  return fd;
}

bool create (const char *file, unsigned initial_size)
{
  if(file == NULL) exit(-1);
  return filesys_create(file, initial_size);
}

bool remove(const char *file){
  if(file==NULL) exit(-1);
  return filesys_remove(file);
}

void close(int fd){
  struct file* f = thread_current()->filedescriptor[fd];
  if(f == NULL) exit(-1);
  //printf("close: %d\n", fd);
  file_close(f);
  thread_current()->filedescriptor[fd]=NULL;
  //if (thread_current()->filedescriptor[fd]==NULL) printf("wow");
}

int filesize(int fd){
  struct file* f = thread_current()->filedescriptor[fd];
  if(f==NULL) exit(-1);
  return file_length(f);
}

void seek (int fd, unsigned int position)
{
  struct file* f = thread_current()->filedescriptor[fd];
  if(f==NULL) exit(-1);
  file_seek(f, position);
}

unsigned int tell(int fd){
  struct file* f = thread_current()->filedescriptor[fd];
  if(f==NULL){exit(-1);}
  return file_tell(f);
}

tid_t exec(const char* file) {
  check_addr(file);
  return process_execute(file);
}

int wait(tid_t pid) {
  return process_wait(pid);
}

void halt(){
  shutdown_power_off();
}
