#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>

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
    context_t* context_;
    struct Thread* next_;
} Thread_t;
 
// extern void threads_init();

extern void create_thread(Thread_t* thread, void (*func)(), uint64_t stack_ptr);
 
extern void schedule(); 

extern void switch_threads(context_t **old_c, context_t *new_c); 
 
#endif // __TASK_H__