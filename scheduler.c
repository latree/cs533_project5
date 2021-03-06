#include "scheduler.h"

#define _GNU_SOURCE
#include <sched.h>



#define STACK_SIZE 1024*1024

struct thread * current_thread;
struct queue ready_list;


void thread_wrap() {
    mutex_lock(current_thread -> TCBMutex);
    current_thread->initial_function(current_thread->initial_argument);
    current_thread -> state = DONE;
    mutex_unlock(current_thread -> TCBMutex);

    condition_broadcast(current_thread -> TCBCond);
    //condition_signal(current_thread->TCBCond);
    yield();
}

void yield() {
    if(current_thread -> state != BLOCKED){
        if(current_thread -> state != DONE){
            current_thread -> state = READY;
            thread_enqueue(&ready_list, current_thread);    
        
        }

        struct thread * next_thread = thread_dequeue(&ready_list);
            
        if(next_thread != NULL){
            next_thread -> state = RUNNING;
            struct thread * temp = current_thread;
            current_thread = next_thread;
            thread_switch(temp, current_thread);        
        }
    
    }
    else{
         struct thread * next_thread = thread_dequeue(&ready_list);
            
        if(next_thread != NULL){
            next_thread -> state = RUNNING;
            struct thread * temp = current_thread;
            current_thread = next_thread;
            thread_switch(temp, current_thread);        
        }
    }
    
}


void scheduler_begin(){
    struct thread * main_thread = malloc(sizeof(struct thread));
    current_thread = main_thread;
    current_thread -> state = RUNNING;
 
    ready_list.head = NULL;
    ready_list.tail = NULL;

}

//void thread_fork(void(*target)(void*), void *arg){
struct thread * thread_fork(void(*target)(void*), void *arg){
    struct thread * new_thread = malloc(sizeof(struct thread));    
    
    new_thread -> stack_pointer = malloc(STACK_SIZE) + STACK_SIZE;
    new_thread -> initial_function = (void*)target;
    new_thread -> initial_argument = arg; 
    // modify from p3 because thread_join


    new_thread -> TCBMutex = malloc(sizeof(struct mutex));
    new_thread -> TCBCond = malloc(sizeof(struct condition));

    mutex_init(new_thread -> TCBMutex);
    condition_init(new_thread -> TCBCond);

    current_thread -> state = READY;
    thread_enqueue(&ready_list, current_thread); 
    new_thread -> state = RUNNING;
    struct thread * temp = current_thread;
    current_thread = new_thread;
    thread_start(temp, current_thread);

    // because of join
    return new_thread;
}

void thread_join(struct thread * thread){
    mutex_lock(thread -> TCBMutex);
    if(thread -> state != DONE){
        condition_wait(thread -> TCBCond, thread -> TCBMutex);
    }
    mutex_unlock(thread -> TCBMutex);

}

void scheduler_end(){
    while(!is_empty(&ready_list)){
        yield();
    }
}



void mutex_init(struct mutex * lock){
    lock -> held = 0;
    //lock -> waiting_threads = malloc(sizeof(struct queue));
    lock -> waiting_threads.head = NULL;
    lock -> waiting_threads.tail = NULL;
}


void mutex_lock(struct mutex * lock){
    if(lock -> held == 1){
        current_thread -> state = BLOCKED;
        thread_enqueue(&lock -> waiting_threads, current_thread);
        yield();   // this is where I forget
    }
    else
        lock -> held = 1;
}

void mutex_unlock(struct mutex * lock){

    if(!is_empty(&lock -> waiting_threads)){
        struct thread * ready_thread = thread_dequeue(&lock -> waiting_threads);
        if(ready_thread){
            ready_thread -> state = READY;
            thread_enqueue(&ready_list, ready_thread);
        }
    }
    else
        lock -> held = 0;
}



void condition_init(struct condition * cond){
    //cond -> waiting_threads =malloc(sizeof(struct queue));
    cond -> waiting_threads.head = NULL;
    cond -> waiting_threads.tail = NULL;
}

void condition_wait(struct condition * cond, struct mutex * lock){
    mutex_unlock(lock);
    thread_enqueue(&cond -> waiting_threads, current_thread);
    current_thread -> state = BLOCKED;
    yield();
    mutex_lock(lock);
}

void condition_signal(struct condition * cond){
    struct thread * ready_thread = thread_dequeue(&cond -> waiting_threads);
    if(ready_thread){
        ready_thread -> state = READY;
        thread_enqueue(&ready_list, ready_thread);
    }
}


void condition_broadcast(struct condition * cond){
    while(!is_empty(&cond -> waiting_threads)){
        struct thread * ready_thread = thread_dequeue(&cond -> waiting_threads);
        if(ready_thread){
            ready_thread -> state = READY;
            thread_enqueue(&ready_list, ready_thread);
        }
        else
            break;
    }
}















