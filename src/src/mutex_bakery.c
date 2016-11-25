#include <mutex_backery.h>

// int select[MAX_THREAD_NUM];
// int ticket[MAX_THREAD_NUM];

int max() {
	int rc = 0;
	for (int i = 0; i < MAX_THREAD_NUM; i++) {
		if (ticket[i] > rc) {
			rc = ticket[i];
		}
	}
	return rc;
}

void lock(int thread_id, mutex_t& mtx) {
	mtx.select[thread_id] = true;
	mtx.ticket[thread_id] = max() + 1;
	mtx.select[thread_id] = false;

	for (int thread = 0; thread < MAX_THREAD_NUM; thread++) {
		if (thread == thread_id) {
			continue;
		}
		while (mtx.select[thread]);
		while (mtx.ticket[thread] && ((mtx.ticket[thread] < mtx.ticket[thread_id]) || 
			(mtx.ticket[thread] == mtx.ticket[thread_id] && thread < thread_id)));
	}
}

void unlock(int thread_id, mutex_t& mtx) {
	mtx.ticket[thread_id] = 0;
}