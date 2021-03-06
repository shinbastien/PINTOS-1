#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/filesys.h"
#include "filesys/directory.h"

typedef int32_t off_t;
int create_num=0; 


struct file 
  {
    struct inode *inode;        /* File's inode. */
    off_t pos;                  /* Current position. */
    bool deny_write;            /* Has file_deny_write() been called? */
  };

struct semaphore file_lock;

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  /* Initialize semaphore for file system synchronization */
  sema_init(&file_lock,1);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* Arguments are in stack of interrupt frame*/
  void *pointer = f->esp;
  // check_user_sp(pointer);
  // thread_current()->esp = pointer;
  switch(*(int*)(pointer)){
    case SYS_HALT:
      halt();
      break;                        /* Halt the operating system. */
    case SYS_EXIT:                   /* Terminate this process. */

      check_user_sp(pointer+4);
      exit(*(int*)(pointer+4));

      break;
    case SYS_EXEC:                   /* Start another process. */
  // sema_down(&file_lock);

      check_user_sp(pointer+4);
      f->eax=exec(*(int*)(pointer+4));
              // sema_up(&file_lock);

      break;
    case SYS_WAIT:                   /* Wait for a child process to die. */

      check_user_sp(pointer+4);
      f->eax=wait((tid_t)*(uint32_t *)(f->esp + 4));

      break;
    case SYS_CREATE:                 /* Create a file. */

      check_user_sp(pointer+8);
      // sema_down(&file_lock);
          // sema_down(&file_lock);

      f->eax=create(*(int*)(pointer+4),*(unsigned*)(pointer+8));
      // sema_up(&file_lock);
    // sema_up(&file_lock);

      break;
    case SYS_REMOVE:                 /* Delete a file. */

      check_user_sp(pointer+4);
      // sema_down(&file_lock);
      f->eax=remove(*(int*)(pointer+4));
      // sema_up(&file_lock);
      break;
    case SYS_OPEN:                   /* Open a file. */

      check_user_sp(pointer+4);
      f->eax=open (*(int*)(pointer+4));
      break;
    case SYS_FILESIZE:               /* Obtain a file's size. */
      check_user_sp(pointer+4);
      f->eax=filesize(*(int*)(pointer+4));
      break;
    case SYS_READ:                   /* Read from a file. */
      check_user_sp(pointer+12);
      f->eax=read(*(int*)(pointer+4),*(void**)(pointer+8),*(unsigned*)(pointer+12));
      break;
    case SYS_WRITE:                  /* Write to a file. */

      check_user_sp(pointer+12);
      f->eax=write(*(int*)(pointer+4),*(void**)(pointer+8),*(unsigned*)(pointer+12));

      break;
    case SYS_SEEK:                   /* Change position in a file. */

      check_user_sp(pointer+8);
      seek(*(int*)(pointer+4),*(unsigned*)(pointer+8));
      break;
    case SYS_TELL:                   /* Report current position in a file. */

      check_user_sp(pointer+4);
      f->eax=tell(*(int*)(pointer+4));
      break;
    case SYS_CLOSE:
      // sema_down(&file_lock);

      check_user_sp(pointer+4);
      close(*(int*)(pointer+4)); 
            // sema_up(&file_lock);

      break;                      /* Close a file. */
    case SYS_MMAP:
      // sema_down(&file_lock);

      check_user_sp(pointer+4);
      f->eax=mmap(*(int*)(pointer+4),*(void**)(pointer+8)); 
            // sema_up(&file_lock);

      break;                      /* Close a file. */
    case SYS_MUNMAP:
      // sema_down(&file_lock);

      check_user_sp(pointer+4);
      munmap(*(int*)(pointer+4)); 
            // sema_up(&file_lock);

      break;                      /* Close a file. */
    case SYS_CHDIR:
      check_user_sp(pointer+4);
      f->eax=chdir(*(int*)(pointer+4));
      break;
    case SYS_MKDIR:
      check_user_sp(pointer+4);
      f->eax=mkdir(*(int*)(pointer+4));
      break;
    case SYS_READDIR:
      check_user_sp(pointer+8);
      f->eax=readdir(*(int*)(pointer+4),*(int*)(pointer+8));
      break;
    case SYS_ISDIR:
      check_user_sp(pointer+4);
      f->eax=isdir(*(int*)(pointer+4));
      break;
    case SYS_INUMBER:
      check_user_sp(pointer+4);
      f->eax=inumber(*(int*)(pointer+4));
      break;
    default:
      exit(-1);
      // break;
  }
  // Original code call thread_exit() every time
  // thread_exit ();
}
void halt (void){
  shutdown_power_off ();
}

void exit (int status){
  if(status>=0){
    printf ("%s: exit(%d)\n",thread_current()->name,status);
  }
  /* If status is less than 0, exit returns -1 */
  else{
    printf ("%s: exit(%d)\n",thread_current()->name,-1);
  }
  thread_current()->child_status=status;
  thread_exit ();
}

/* we use tid as pid */
tid_t exec (const char *cmd_line){
  // sema_down(&file_lock);
  // filesys_open(cmd_line);

  // sema_up(&file_lock);

  tid_t tid=process_execute(cmd_line);

  return tid;
}

int wait (tid_t pid){

      return process_wait(pid);
}

bool create (const char *file, unsigned initial_size){
  /* Check the validity of file */
    // printf("create\n");
  if(create_num<250)
    create_num++;
  else 
    return false;
  if(file==NULL||*file==NULL){
    exit(-1);
  }
  //filesys_create에 아래 조건문 넣기!
  else if(strlen(file)==0||strlen(file)>14){
    // printf("here\n");
    return 0;
    // sema_up(&file_lock);
  }
  /* If file is valid, create file with initial size */
  else{
    // printf("hi \n");
    // printf("create_success %d\n",thread_current()->tid);
    sema_down(&file_lock);
    bool success = filesys_create(file, initial_size);
    sema_up(&file_lock);
    return success;
  }
}

bool remove (const char *file){
  if(file==NULL||*file==NULL){
    exit(-1);
  }
  sema_down(&file_lock);
  bool success = filesys_remove(file);
  sema_up(&file_lock);
  return success;
}

int write (int fd, const void *buffer, unsigned size){
  // printf("write at : %d\n",fd);
  /* check the validity of buffer pointer */
  check_user_sp(buffer);
  // if(*(char*)buffer==NULL)
  //   exit(-1);

  struct thread *cur = thread_current();

  /* If buffer is valid, lock write for file synchronization */ 
  sema_down(&file_lock);
  /* Fd 1 means standard output(ex)printf) */

  if(fd ==1){

    putbuf(buffer,size);
    sema_up(&file_lock);
        // printf("write at %s\n",cur->name);

    return size;
  }


  else{
    struct file* current_file =cur->fd_table[fd];

    /* If file is running process, deny write */
    if(current_file->deny_write){

      sema_up(&file_lock);
      return 0;
    }
    // printf("write! \n");
    int result= file_write (cur->fd_table[fd], buffer, size);
    sema_up(&file_lock);
    return result;
   }
}

int read (int fd, void *buffer, unsigned size){
  // printf("read\n");
  /* check the validity of buffer pointer */
  check_user_sp(buffer);

  uint8_t readbytes;
  struct thread *cur = thread_current();
  sema_down(&file_lock);
  
  /* Fd 1 means standard input(keyboard)  */
  if(fd==0){
    readbytes=input_getc();
    sema_up(&file_lock);

    return readbytes;
  }
  else{
    int result = file_read(cur->fd_table[fd], buffer, size);
    sema_up(&file_lock);

    return result;
  }
}

int open (const char *file){
  // printf("open filename=%s\n",file);
  if(file==NULL){

    exit(-1);
  }
  /* Length of file name should be 1 to 13 */
  //이 코드를 filesys_open안으로 넣어야할듯!
  /* lock for file synchronization */
  // sema_down(&file_lock);
  // printf("filename=%s\n",file);
  // printf("thread_current tid %d\n",thread_current()->tid);
  struct file* opened_file;
  struct thread *cur =thread_current();
  int fd;
  sema_down(&file_lock);
  opened_file = filesys_open (file);
        // printf("open %s\n",opened_file);
  // sema_up(&file_lock);
  if(opened_file==NULL){
    sema_up(&file_lock);
    // printf("opened_file ==NULL\n");
    return -1;
  }

  /* if file is current process, deny write */
  if(!strcmp(file,cur->name))
    file_deny_write(opened_file);
  /* number of opened files should be less than 128 */
  /* check vacant room of fd_table */
  for(int i =2;i<130;i++){
    if(cur->fd_table[i]==NULL){
        cur->fd_table[i]=opened_file;
        fd=i;
        sema_up(&file_lock);
        // printf("fd=%d\n",fd);


        return fd;
    }
  }
  sema_up(&file_lock);
  // printf("here\n");
  return -1;
}

int filesize (int fd){
  struct thread *cur =thread_current();
  return file_length(cur->fd_table[fd]);
}

void seek (int fd, unsigned position){
  struct thread *cur =thread_current();
  sema_down(&file_lock);
  file_seek (cur->fd_table[fd], position);
  sema_up(&file_lock);
}

unsigned tell (int fd){
  struct thread *cur =thread_current();
  // printf('here\n');
  file_tell (cur->fd_table[fd]);
}

void close (int fd){
    sema_down(&file_lock);

  struct thread *cur =thread_current();
  struct file* file = cur->fd_table[fd];
  if(file!=NULL){
        // printf("close_success %d\n",thread_current()->tid);

    file_allow_write(file);
    file_close(file);
    cur->fd_table[fd]=NULL;
    sema_up(&file_lock);
    return;
  }
  sema_up(&file_lock);
}


/* check the validity of user stack pointer */
static inline void check_user_sp(const void* sp){
  if(!is_user_vaddr(sp)||sp<0x08048000){
    exit(-1);
  }
  // 	void *ptr = pagedir_get_page(thread_current()->pagedir,sp);
  // if(!ptr){
  //   printf("here\n");
  // }
}
static bool
install_page (void *upage, void *kpage, bool writable)
{
  struct thread *t = thread_current ();

  /* Verify that there's not already a page at that virtual
     address, then map our page there. */
  return (pagedir_get_page (t->pagedir, upage) == NULL
          && pagedir_set_page (t->pagedir, upage, kpage, writable));
}

bool load_from_exec (struct spte *spte)
{
    
    uint8_t *frame = frame_alloc(PAL_USER, spte);
    frame_table_lock_acquire();

    if (!frame) {
      frame_table_lock_release();

      return false;
    }
     
    if (spte->state==MM_FILE){
      if (spte->read_bytes > 0){
        sema_down(&file_lock);
        if ((int) spte->read_bytes != file_read_at(spte->mmap_file->file, frame, spte->read_bytes, spte->offset)){
          printf("1\n");

          printf("rb %d offset %d \n",spte->read_bytes,spte->offset);
          sema_up(&file_lock);
          frame_table_lock_release();

          free_frame(frame);
          return false;
        }
        sema_up(&file_lock);
        memset(frame + spte->read_bytes, 0, spte->zero_bytes);
      }
      else{
        memset (frame, 0, PGSIZE);
      }

    }
    else{
      if (spte->read_bytes > 0){
        sema_down(&file_lock);
        if ((int) spte->read_bytes != file_read_at(spte->file, frame, spte->read_bytes, spte->offset)){
          printf("1\n");
          printf("rb %d offset %d \n",spte->read_bytes,spte->offset);
          sema_up(&file_lock);
          frame_table_lock_release();
          free_frame(frame);
          return false;
        }
        sema_up(&file_lock);
        memset(frame + spte->read_bytes, 0, spte->zero_bytes);
      }
      else{
        memset (frame, 0, PGSIZE);
      }
    }
    if (!install_page(spte->upage, frame, spte->writable)) {
      frame_table_lock_release();

        free_frame(frame);
        return false;
    }
    spte->state = MEMORY;  
    frame_table_lock_release();

    return true;
}

int mmap (int fd, void* upage) {
  if(get_spte(&thread_current()->spt,upage))
    return -1;
  if(pg_round_down(upage)!=upage)
    return -1;
  if(!is_user_vaddr(upage)||upage<0x08048000)
    return -1;
  ASSERT(fd!=0&&fd!=1);
  if (fd>130)
    exit(-1);
  struct thread *cur =thread_current();
  struct file* file = cur->fd_table[fd];
  int filesize=file_length(file);
  int read_bytes=filesize;
  if(filesize<1)
    return -1;
  int ofs=0;
  struct mmap_file* mmap_file = (struct mmap_file*) malloc(sizeof(struct mmap_file));
  list_init(&mmap_file->spte_list);
  list_push_back(&cur->mmap_list,&mmap_file->elem);
  mmap_file->map_id=fd;
  while (ofs<filesize) 
    {
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      if(!get_spte(&cur->spt,upage)){
        bool result=  create_spte_from_mmf(file, ofs,upage, page_read_bytes, page_zero_bytes,true,mmap_file);
        if(!result){
          printf("spte error at load_segment");
        }
        ofs+= PGSIZE;
        /* Advance. */
        read_bytes -= page_read_bytes;
        upage += PGSIZE;
      }
      else
        return -1;
    }
  sema_down(&file_lock);
  struct file* new_file = file_reopen(file);
  mmap_file->file = new_file;
  sema_up(&file_lock);
  if(new_file==NULL){
    return -1;
  }
  return fd;
}
void
munmap(int mid){
  struct thread* cur = thread_current();
  struct mmap_file* mmap_file;
  struct list_elem* e;
  for(e=list_begin(&thread_current()->mmap_list);e!=list_end(&thread_current()->mmap_list);e=list_next(e)){
    if((mmap_file=list_entry(e,struct mmap_file,elem))->map_id==mid)
      break;
  }
  struct list_elem* h;

  struct spte* spte;
  struct spte find;

  for(h=list_begin(&mmap_file->spte_list);h!=list_end(&mmap_file->spte_list);h=list_next(h)){
    spte=list_entry(h,struct spte,mmf_elem);
    find.upage=spte->upage;
    hash_delete (&cur->spt,&find.elem);
    if(spte->state==MEMORY&&pagedir_is_dirty(cur->pagedir,spte->upage)){
      sema_down(&file_lock);
      file_write_at(mmap_file->file,spte->upage,spte->read_bytes,spte->offset);
      sema_up(&file_lock);
    }
  }
  sema_down(&file_lock);
  file_close (mmap_file->file);
  sema_up (&file_lock);
  list_remove(&mmap_file->elem);
  free(mmap_file);
}

bool chdir(const char *dir) {
  // printf("chdir : %s\n",dir);
  struct dir *dir_now = find_dir(dir);
  if(dir_now==NULL)
    return false;
  thread_current()->dir_now = dir_now;
  return true;
}
bool mkdir (const char *dir) {
    // printf("mkdir : %s\n",dir);

  // sema_down(&file_lock);
  bool success;
  success=filesys_create_dir(dir);
  return success;
  // sema_up (&file_lock);
}

bool isdir (int fd) {
  struct file* file = thread_current()->fd_table[fd];
  return is_dir(file->inode);
}
bool readdir (int fd, const char *name) {
      // printf("readdir : %s\n");

  struct file* file = thread_current()->fd_table[fd];   
 if (is_dir(file->inode)) {
  struct inode* inode = file->inode;
  // if (check_open(inode)) {
  //   inode_close(inode);
  // }
  struct dir* dir = dir_open(inode);
  bool result =dir_readdir(dir, name);
  // dir_close(dir);
  return result;
 }
 else
  return false;
}
// 
// 
// 
int inumber(int fd) {
  struct file* file = thread_current()->fd_table[fd];
  return inode_get_inumber(file->inode);
}