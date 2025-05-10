#include "types.h"
#include "param.h"
#include "memlayout.h"   
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"        
#include "defs.h"
#include "petersonlock.h"


struct peterson_lock peterson_locks[MAX_PETERSON_LOCKS];

void
petersonlocksinit(void)
{
  printf("Initializing Peterson locks...\n");
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    peterson_locks[i].flag[0] = 0;
    peterson_locks[i].flag[1] = 0;
    peterson_locks[i].turn = 0;
    peterson_locks[i].active = 0;
  }
}


int
peterson_create(void)
{
  printf("im here");
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++) {
    printf("Checking lock %d: active=%d\n", i, peterson_locks[i].active);
    if (peterson_locks[i].active == 0) {  
      peterson_locks[i].flag[0] = 0;
      peterson_locks[i].flag[1] = 0;
      peterson_locks[i].turn = 0;
      __sync_synchronize();              
      peterson_locks[i].active = 1;
      return i; 
    }
  }
  return -1; 
}

int
peterson_acquire(int lock_id, int role)
{
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1)
    return -1;
  if (peterson_locks[lock_id].active == 0)
    return -1;

  __sync_lock_test_and_set(&peterson_locks[lock_id].flag[role], 1); 
  __sync_lock_test_and_set(&peterson_locks[lock_id].turn, 1 - role); 

  __sync_synchronize(); 

  while (peterson_locks[lock_id].flag[1 - role] && peterson_locks[lock_id].turn == (1 - role)) {
    yield(); 
    __sync_synchronize();
  }

  return 0;
}

int
peterson_release(int lock_id, int role)
{
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS || role < 0 || role > 1)
    return -1;
  if (peterson_locks[lock_id].active == 0)
    return -1;

  __sync_lock_release(&peterson_locks[lock_id].flag[role]);
  __sync_synchronize(); 

  return 0;
}

int
peterson_destroy(int lock_id)
{
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS)
    return -1;
  if (peterson_locks[lock_id].active == 0)
    return -1;

  peterson_locks[lock_id].active = 0; 
  __sync_synchronize();

  return 0;
}
