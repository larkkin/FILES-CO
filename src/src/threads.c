#include <alloc.h>
#include <memory.h>
#include <ints.h>
#include <mutex_cas.h>
#include <time.h>
#include <threads.h>
#include <i8259a.h>
#include <time.h>

#define STACK_SIZE   2 * PAGE_SIZE

static spinlock_t threads_mutex;

static Thread_t* current_thread; 
static Thread_t* threads_list_top;
int all_started = 0;  
int all_proc_ended = 0; 



void main_thread_setup()
{
    Thread_t * main_thread = mem_alloc(sizeof(Thread_t));
    main_thread->stack_end_ptr_ = 0;
    main_thread->stack_start_ptr_ = 0;
    main_thread->state_ = WORKING;
    start_thread(main_thread);
}


void init_thread(Thread_t* thread, 
                 void (*func)(void *), 
                 void *args) {
    void * new_stack_end = mem_alloc(STACK_SIZE);
    thread->stack_start_ptr_ = (uint64_t) new_stack_end + STACK_SIZE;
    thread->stack_end_ptr_ = (uint64_t) new_stack_end;

    thread->state_ = WORKING;

    thread->stack_start_ptr_ -= sizeof(context_t);

    thread->context_ = (context_t*) thread->stack_start_ptr_;
    thread->context_->rflags = 0;
    thread->context_->r15 = 0;
    thread->context_->r14 = (uint64_t) args;
    thread->context_->r13 = 0;
    thread->context_->r12 = 0;
    thread->context_->rbp = 0;
    thread->context_->rbx = 0;
    thread->context_->rdi = (uint64_t) thread;
    thread->context_->rip = (uint64_t) func;
}

void start_thread(Thread_t* thread) {
    lock(&threads_mutex);
    if (threads_list_top == 0)
    {
        threads_list_top = thread;
        list_init(&thread->node_);
    }
    else
    {
        list_add_tail(&(thread->node_), &(threads_list_top->node_));
    }
    unlock(&threads_mutex);    
}

void wait_for_thread(Thread_t * thread)
{
    while(thread->state_ != TERMINATED);
    if (thread->stack_end_ptr_) {
        mem_free((void *)(thread->stack_end_ptr_));
    }
    mem_free(thread);
}

void terminate_thread(Thread_t * thread)
{
    disable_ints();
    lock(&threads_mutex);

    thread->state_ = TERMINATED;
    if (threads_list_top == thread)
    {
        switch_current_thread();
    }
    else
    {
        list_del(&thread->node_);
    }
    unlock(&threads_mutex);
    enable_ints();
}


void switch_current_thread() { 
    lock(&threads_mutex);
    if (current_thread != threads_list_top) {
        Thread_t* previous_thread = current_thread;
        current_thread = (Thread_t*) current_thread->node_.next; 
        switch_threads(&previous_thread->context_, current_thread->context_);
    }
    unlock(&threads_mutex);
}


void force_start_thread(Thread_t* thread) {
    lock(&threads_mutex); 
    thread->node_.next = &current_thread->node_; //чтобы он вызвал после себя текущий
    context_t* previous_context = current_thread->context_;
    current_thread = thread;

    pic_ack(PIT_IRQ);
    
    switch_threads(&previous_context, current_thread->context_);
    unlock(&threads_mutex);   
}






