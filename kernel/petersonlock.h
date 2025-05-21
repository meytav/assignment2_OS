#ifndef PETERSONLOCK_H
#define PETERSONLOCK_H

#define MAX_PETERSON_LOCKS (NPROC - 1)

struct peterson_lock {
  int flag[2];    
  int turn;       
  int active;     
};



void petersonlocksinit(void);
int peterson_create(void);
int peterson_acquire(int lock_id, int role);
int peterson_release(int lock_id, int role);
int peterson_destroy(int lock_id);

#endif // PETERSONLOCK_H
