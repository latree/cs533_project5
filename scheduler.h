#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

#define current_thread (get_current_thread())

typedef enum {
    RUNNING, 
    READY,
    BLOCKED,
    DONE
} state_t;

struct thread{
    unsigned char * stack_pointer;
    void(* initial_function)(void*);
    void * initial_argument;
    state_t state;
    struct mutex *TCBMutex;
    struct condition *TCBCond;
};


struct mutex {
     int held;
     struct queue waiting_threads;
};


struct condition {
     struct queue waiting_threads;
};





void scheduler_begin();
//void tread_fork(void(*target)(void*), void * arg);
struct thread * thread_fork(void(*target)(void*), void * arg);
void thread_join(struct thread * thread);

void yield();
void scheduler_end();


void mutex_init(struct mutex * lock);
void mutex_lock(struct mutex * lock);
void mutex_unlock(struct mutex * lock);

void condition_init(struct condition * cond);
void condition_wait(struct condition * cond, struct mutex * lock);
void condition_signal(struct condition * cond);
void condition_broadcast(struct condition * cond);





extern void thread_start(struct thread *old, struct thread *new);
extern void thread_switch(struct thread *old, struct thread *new);

extern struct thread * current_thread;


