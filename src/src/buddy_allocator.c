#include <buddy_allocator.h>
#include <print.h>


typedef struct buddy_allocator_descriptor {
	uint64_t page_num;
	bool is_free;
	uint32_t block_order;
	page_t* page_ptr;
	// struct buddy_allocator_descriptor* prev;
	// struct buddy_allocator_descriptor* next;
} buddy_descriptor_t;



uint64_t deg_2_round_up(uint64_t num)
{

	uint64_t res = 0;
	
	uint64_t num_copy = 1;
	while (num_copy < num) {
		num_copy <<= 1;
		res++;
	}

	return res;
}

static buddy_descriptor_t* descriptors_start;
static uint64_t pages_number;
static uint64_t MEMORY_START;
static uint64_t MEMORY_END;


typedef struct longed_page_type {
	uint64_t arr[1<<9];
} longed_page_t;

uint64_t get_order(uint64_t page_num) {
	uint64_t res = 0;
	uint64_t current_dist = 1;
	buddy_descriptor_t* descriptor_ptr = descriptors_start+page_num;
	while ((page_num & 1) == 0 && (uint64_t) (descriptor_ptr + (current_dist)) < MEMORY_END) {
		page_num >>= 1;
		current_dist <<= 1;
		res++;
	}
	return res;
}


void clear_page(longed_page_t* page) {
	uint64_t length = 1 << 9;
	for (uint64_t i = 0; i < length; ++i) {
		page->arr[i] = 0;
	}
}
void clear_block(buddy_descriptor_t* descriptor_ptr) {
	buddy_descriptor_t* temp_descriptor_ptr = descriptor_ptr;
	while (temp_descriptor_ptr != descriptor_ptr + (1<<descriptor_ptr->block_order)) {
		temp_descriptor_ptr->is_free = true;
		clear_page((longed_page_t*) temp_descriptor_ptr->page_ptr);
		temp_descriptor_ptr++;
	}
}

page_t* b_allocate(uint32_t size) {
	// printf("starting allocation\n");
	uint64_t order_to_allocate =  deg_2_round_up(size);
	uint64_t pages_to_allocate = 1 << order_to_allocate;
	uint32_t page_num = 0; 
	buddy_descriptor_t* descriptor_ptr = descriptors_start;
	while (page_num < pages_number && 
		   (descriptor_ptr->block_order < order_to_allocate || !descriptor_ptr->is_free)) {
		page_num += pages_to_allocate;
		descriptor_ptr+=pages_to_allocate;
	}
	// printf("page found\n");
	if (page_num >= pages_number) {
		printf("unable to allocate memory\n");
		return 0;
	}
	buddy_descriptor_t* temp_descriptor_ptr = descriptor_ptr;
	while (temp_descriptor_ptr != descriptor_ptr + (1<<order_to_allocate)) {
		temp_descriptor_ptr->is_free = false;
		temp_descriptor_ptr++;
	}
	descriptor_ptr->block_order = order_to_allocate;
	printf("allocated %d Kbytes to page %d, order: %d\n", pages_to_allocate<<2, page_num, order_to_allocate);
	return descriptor_ptr->page_ptr;
}

bool b_free(page_t* page) {
	// printf("%lx\n", (uint64_t) page);
	uint64_t dist_to_start = ((uint64_t) page) - MEMORY_START;
	uint64_t page_num = dist_to_start/PAGE_SIZE; 
	if ((uint64_t) page < MEMORY_START || 
		(uint64_t) page > MEMORY_END || 
		dist_to_start/PAGE_SIZE*PAGE_SIZE != dist_to_start)
	{
		return false;
	}

	buddy_descriptor_t* descriptor_ptr = descriptors_start+page_num;
	clear_block(descriptor_ptr);
	while ((uint64_t) (page + (1<<descriptor_ptr->block_order)) < MEMORY_END) { 	
		uint64_t current_order = descriptor_ptr->block_order;	
		buddy_descriptor_t* buddy_descriptor_ptr = descriptors_start + (page_num^(1<<current_order));
		if (!buddy_descriptor_ptr->is_free || buddy_descriptor_ptr->block_order < current_order) {
			break;
		}
		descriptor_ptr->block_order++;
	}
	return true;
}


void init_allocator(uint64_t start, uint64_t end) {
	// printf("%d", deg_2_round_up(1));
	printf("\n==================== initializing allocator =====================\n");
	uint64_t length = end - start;
	// d_num = (length - d_num*sizeof(d)) / (1<<12)
	// (1<<12)*d_num = length - d_num*sizeof(d)
	// d_num*((1<<12) - sizeof(d)) = length
	// d_num = length / ((1<<12) - sizeof(d))
	uint64_t decriptors_number = length / ((PAGE_SIZE) - sizeof(buddy_descriptor_t)) - 1;
	printf("pages number: %d, size of descriptor: %lx\n",  decriptors_number, sizeof(buddy_descriptor_t));
	MEMORY_START = (((start + decriptors_number*sizeof(buddy_descriptor_t))>>12) + 1)<<12;
	MEMORY_END = MEMORY_START + (((end - MEMORY_START)>>12)<<12);
	printf("descriptor segment length in kbytes: %d\n", (decriptors_number*sizeof(buddy_descriptor_t))>>10);

	printf("\n\nmemory segment start: %lx, end: %lx\n", MEMORY_START, MEMORY_END);
	printf("memory segment length in Mbytes: %d\n", (MEMORY_END - MEMORY_START)>>20);
	
	buddy_descriptor_t* descriptor_ptr = (buddy_descriptor_t*) start;
	descriptors_start = descriptor_ptr;
	pages_number = decriptors_number;
	page_t* page = (page_t*) MEMORY_START;
	uint64_t page_index = 0;
	// descriptor_ptr->prev = NULL;
	for (uint64_t i = 0; i < decriptors_number; i++) {
		descriptor_ptr->page_num = page_index;
		descriptor_ptr->is_free = true;
		descriptor_ptr->block_order = get_order(page_index);
		// printf("%d ", descriptor_ptr->block_order);
		descriptor_ptr->page_ptr = page;
		// descriptor_ptr->next = descriptor_ptr+1;
		// descriptor_ptr->next->prev = descriptor_ptr;
		descriptor_ptr++;
		page++;
		page_index++;
	}
	printf("==================== allocator  initialized =====================\n\n");
	// descriptor_ptr->next = NULL;

}


