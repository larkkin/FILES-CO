const int MAX_THREAD_NUM = 2;

void lock(int thread_id, mutex& mtx);
void unlock(int thread_id, mutex& mtx);

typedef struct mutex_type {
	int select[MAX_THREAD_NUM];
	int ticket[MAX_THREAD_NUM];
} mutex_t

int get_thread_num() {
	return ???;
}