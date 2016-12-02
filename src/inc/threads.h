#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <list.h>

typedef enum
{
    WORKING,
    TERMINATED
} thread_state;

typedef struct context
{
    uint64_t rflags;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rip;
    uint64_t rdi;
} context_t;

typedef struct Thread {
    list_head_t node_;
    context_t* context_;
    uint64_t stack_start_ptr_;
    uint64_t stack_end_ptr_;
    thread_state state_;
} Thread_t;
 

void main_thread_setup();

extern void init_thread(Thread_t* thread, 
                 void (*func)(void *), 
                 void *args);

extern void start_thread(Thread_t* thread);

extern void terminate_thread(Thread_t * thread);

extern void switch_current_thread(); 

extern void switch_threads(context_t **old_c, context_t *new_c); 
 
#endif // __TASK_H__