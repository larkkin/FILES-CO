#include <threads.h>
#include <mutex_cas.h>
#include <time.h>

#define STACK_SIZE  = 1024

static spinlock_t threads_mutex;

static Thread_t* current_thread; 
static Thread_t* threads_list_top;




void init_thread(Thread_t* thread, 
                   void (*func)(), 
                   uint64_t stack_ptr) {
    stack_ptr -= sizeof (*thread->context_);
    thread->context_ = (context_t*) stack_ptr;
    thread->context_->rflags = 0;
    thread->context_->r15 = 0;
    thread->context_->r13 = 0;
    thread->context_->r12 = 0;
    thread->context_->rbp = 0;
    thread->context_->rbx = 0;
    thread->context_->rdi = (uint64_t) thread;
    thread->context_->rip = (uint64_t) func;
}

void start_thread(Thread_t* thread) {
    lock(&threads_mutex);
    threads_list_top->next_ = thread;
    threads_list_top = thread;
    unlock(&threads_mutex);    
}

void terminate_current_thread() { 
    lock(&threads_mutex); //в принципе вполне-таки раунд робин
    if (current_thread != threads_list_top) {
        Thread_t* previous_thread = current_thread;
        current_thread = current_thread->next_; 
        switch_threads(&previous_thread->context_, current_thread->context_);
    }
    unlock(&threads_mutex);
}

void force_start_thread(Thread_t* thread) {
    lock(&threads_mutex);
    thread->next_ = current_thread;
    context_t* previous_context = current_thread->context_;
    current_thread = thread;
    switch_threads(&previous_context, current_thread->context_);
    unlock(&threads_mutex);   
}






