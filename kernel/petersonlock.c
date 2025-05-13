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
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++)
  {
    peterson_locks[i].flag[0] = 0;
    peterson_locks[i].flag[1] = 0;
    peterson_locks[i].turn = 0;
    peterson_locks[i].active = 0;
  }

}

int 
peterson_create(void)
{
  for (int i = 0; i < MAX_PETERSON_LOCKS; i++)
  {
    if (__sync_lock_test_and_set(&peterson_locks[i].active, 1) == 0)
    {
      peterson_locks[i].flag[0] = 0;
      peterson_locks[i].flag[1] = 0;
      peterson_locks[i].turn = 0;
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

  // Set turn to the other process - this is the "polite" gesture
  // We don't need test_and_set here, just assign with a barrier
  peterson_locks[lock_id].turn = 1 - role;

  __sync_synchronize();

  while (peterson_locks[lock_id].flag[1 - role] && peterson_locks[lock_id].turn == (1 - role))
  {
    yield();
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

  __sync_synchronize();

  __sync_lock_release(&peterson_locks[lock_id].flag[role]);

  return 0;
}

int 
peterson_destroy(int lock_id)
{
  if (lock_id < 0 || lock_id >= MAX_PETERSON_LOCKS)
    return -1;
  if (peterson_locks[lock_id].active == 0)
    return -1;
  // Ensure no one is using the lock before deactivating it
  if (peterson_locks[lock_id].flag[0] || peterson_locks[lock_id].flag[1])
    return -1; // Lock is still in use

  __sync_synchronize();

  __sync_lock_release(&peterson_locks[lock_id].active);

  return 0;
}
