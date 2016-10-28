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


typedef struct free_segment_type {
	uint64_t start;
	uint64_t end;
} free_segment_t;

typedef struct free_segments_array {
	uint8_t size;
	free_segment_t segments[32];
} free_segments_array_t;

typedef struct segment_info_type {
	uint64_t descriptors_start;
	uint64_t pages_start;
	uint64_t pages_number;
} segments_info_t;

typedef struct segments_info_array_type {
	uint8_t size;
	segments_info_t infos[32];
} segments_info_array_t;


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

static free_segments_array_t free_segments;
static segments_info_array_t segments_info;
const uint64_t CANONICAL_OFFSET = 0xffff800000000000;

typedef struct longed_page_type {
	uint64_t arr[1<<9];
} longed_page_t;

uint64_t get_order(uint64_t page_num, uint8_t segment_index) {
	uint64_t res = 0;
	uint64_t current_dist = 1;
	// printf("dist: %d\n", segment_index);
	uint64_t MEMORY_START = segments_info.infos[segment_index].pages_start;
	uint64_t MEMORY_END = segments_info.infos[segment_index].pages_start+ 
						  PAGE_SIZE*segments_info.infos[segment_index].pages_number;
	buddy_descriptor_t* descriptor_ptr = 
		((buddy_descriptor_t*) segments_info.infos[segment_index].descriptors_start)+page_num;
	// printf("%llx\n%llx\n", MEMORY_END, (uint64_t) descriptor_ptr->page_num);
	while ((page_num & 1) == 0 && MEMORY_START + (current_dist+descriptor_ptr->page_num)*PAGE_SIZE < MEMORY_END) {
		// printf("%d\n", current_dist);
		page_num >>= 1;
		current_dist <<= 1;
		res++;
	}
	return res;
}

extern char text_phys_begin[];
extern char bss_phys_end[];

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


allocate_return_t b_allocate_to_segment(uint32_t size, uint8_t segment_index) {
	// printf("starting allocation\n");
	allocate_return_t res;

	uint64_t order_to_allocate =  deg_2_round_up(size);
	uint64_t pages_to_allocate = 1 << order_to_allocate;
	uint32_t page_num = 0; 
	buddy_descriptor_t* descriptor_ptr = (buddy_descriptor_t*) segments_info.infos[segment_index].descriptors_start;
	uint64_t pages_number = segments_info.infos[segment_index].pages_number;
	while (page_num < pages_number && 
		   (descriptor_ptr->block_order < order_to_allocate || !descriptor_ptr->is_free)) {
		page_num += pages_to_allocate;
		descriptor_ptr+=pages_to_allocate;
	}
	// printf("page found\n");
	if (page_num >= pages_number) {
		// printf("unable to allocate memory\n");
		res.success = false;
		return res;
	}
	buddy_descriptor_t* temp_descriptor_ptr = descriptor_ptr;
	while (temp_descriptor_ptr != descriptor_ptr + (1<<order_to_allocate)) {
		temp_descriptor_ptr->is_free = false;
		temp_descriptor_ptr++;
	}
	descriptor_ptr->block_order = order_to_allocate;
	printf("allocated %d Kbytes to segment %d, page %d, order: %d\n",
		pages_to_allocate<<2,
		segment_index,  
		page_num, 
		order_to_allocate);
	res.success = true;
	res.page_ptr = descriptor_ptr->page_ptr; 
	return res;
}

allocate_return_t b_allocate(uint32_t size) {
	allocate_return_t allocated;
	allocated.success = false;
	uint8_t segment_index = 0;
	while (!allocated.success && segment_index < segments_info.size)
	{
		allocated = b_allocate_to_segment(size, segment_index);
		segment_index++;	
	}
	return allocated;
}


bool b_free(page_t* page) {
	uint8_t segment_index = 0;
	uint64_t addr = (uint64_t) page;
	while ((addr < segments_info.infos[segment_index].pages_start || 
			addr >= segments_info.infos[segment_index].pages_start+ 
					PAGE_SIZE*segments_info.infos[segment_index].pages_number) &&
		segment_index < free_segments.size) {
		segment_index++;
	}
	if (segment_index >= free_segments.size)
	{
		return false;
	}
	uint64_t MEMORY_START = segments_info.infos[segment_index].pages_start;
	uint64_t MEMORY_END = segments_info.infos[segment_index].pages_start+
						  PAGE_SIZE*segments_info.infos[segment_index].pages_number;
	// printf("%lx\n", (uint64_t) page);
	uint64_t dist_to_start = ((uint64_t) page) - MEMORY_START;
	uint64_t page_num = dist_to_start/PAGE_SIZE; 
	if ((uint64_t) page < MEMORY_START || 
		(uint64_t) page > MEMORY_END || 
		dist_to_start/PAGE_SIZE*PAGE_SIZE != dist_to_start)
	{
		return false;
	}

	buddy_descriptor_t* descriptor_ptr = 
		((buddy_descriptor_t*) segments_info.infos[segment_index].descriptors_start)+page_num;
	clear_block(descriptor_ptr);
	while ((uint64_t) (page + (1<<descriptor_ptr->block_order)) < MEMORY_END) { 	
		uint64_t current_order = descriptor_ptr->block_order;	
		buddy_descriptor_t* buddy_descriptor_ptr = 
		((buddy_descriptor_t*) segments_info.infos[segment_index].descriptors_start) + (page_num^(1<<current_order));
		if (!buddy_descriptor_ptr->is_free || buddy_descriptor_ptr->block_order < current_order) {
			break;
		}
		descriptor_ptr->block_order++;
	}
	return true;
}

free_segments_array_t get_free_segments(multiboot_info_t* mbt)
{
	printf("----------getting free segmens----------\n");
	free_segments_array_t res;
	res.size = 0;

    uint64_t os_start = (uint64_t) text_phys_begin;
    uint64_t os_end = (uint64_t) bss_phys_end;

    for (multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)(uint64_t) mbt->mmap_addr;
         mmap < (multiboot_memory_map_t*) (uint64_t) (mbt->mmap_addr + mbt->mmap_length);
         mmap = (multiboot_memory_map_t*) (uint64_t) ( (uint64_t) mmap + mmap->size + sizeof(mmap->size) ))  
    {
    	printf("\t new segment, length %lx\n", mmap->len);
    	if (mmap->addr >= 0x80000000 || mmap->addr + mmap->len >= 0x80000000) {
    		printf("not first 4GB or not properly aligned\n");
    		continue;
    	}
        if (mmap->type != 1)
        {
        	printf("non-empty segment\n");
            continue;
        }
        if (mmap->addr >= os_end || mmap->addr + mmap->len <= os_start)
        {
        	printf("full segment\n");
            // segment_start....segment_end.......os_start....os_end  or 
            // os_start....os_end.......segment_start....segment_end
            // printf("from: %lx to: %lx\n", mmap->addr, mmap->addr + mmap->len);
            free_segment_t next_segment;
            next_segment.start = mmap->addr+CANONICAL_OFFSET;
            next_segment.end = mmap->addr + mmap->len+CANONICAL_OFFSET;
            res.segments[res.size] = next_segment;
            res.size++;
            continue; 
        }
        if (mmap->addr < os_start)
        {
        	printf("pre-kernel segment");
        	if (os_start - mmap->addr >= 4*PAGE_SIZE)
	        {  
		        printf("\n");
	        	free_segment_t next_segment;
	            next_segment.start = mmap->addr+CANONICAL_OFFSET;
	            next_segment.end = os_start+CANONICAL_OFFSET;
	            res.segments[res.size] = next_segment;
	            res.size++;
	        }
	        else {
	        	printf(", too small\n");
	        }
        }
        if (mmap->addr + mmap->len > os_end)
        {
        	printf("post-kernel segment");
        	if (mmap->addr + mmap->len - os_end < 4*PAGE_SIZE)
	        {
	        	printf(", too small\n");
	            continue;
	        }  
	        printf("\n");
            // os_end....segment_end
            free_segment_t next_segment;
            next_segment.start = os_end+CANONICAL_OFFSET;
            next_segment.end = mmap->addr + mmap->len+CANONICAL_OFFSET;
            res.segments[res.size] = next_segment;
            res.size++;
        }
    }
    printf("----------got free segments-------------\n\n");
    return res;
}


void init_allocator(multiboot_info_t* mbt) {
	
	// printf("%d", deg_2_round_up(1));
	printf("\n==================== initializing allocator =====================\n");


	free_segments = get_free_segments(mbt);
	segments_info.size = free_segments.size;

	for (uint8_t i = 0; i < free_segments.size; i++) {
		uint64_t start = free_segments.segments[i].start;
		uint64_t end = free_segments.segments[i].end;
		uint64_t length = end - start;
		// d_num = (length - d_num*sizeof(d)) / (1<<12)
		// (1<<12)*d_num = length - d_num*sizeof(d)
		// d_num*((1<<12) - sizeof(d)) = length
		// d_num = length / ((1<<12) + sizeof(d))
		uint64_t decriptors_number = length / ((PAGE_SIZE) + sizeof(buddy_descriptor_t)) - 1;
		printf("\n\n\t segment %d\npages number: %d, size of descriptor: %lx\n",
			i,  
			decriptors_number, 
			sizeof(buddy_descriptor_t));
		uint64_t MEMORY_START = (((start + decriptors_number*sizeof(buddy_descriptor_t))>>12) + 1)<<12;
		uint64_t MEMORY_END = MEMORY_START + (((end - MEMORY_START)>>12)<<12);
		printf("descriptor segment length in kbytes: %d\n", (decriptors_number*sizeof(buddy_descriptor_t))>>10);

		printf("\nmemory segment start: %lx, end: %lx\n", MEMORY_START, MEMORY_END);
		printf("memory segment length in Mbytes: %d\n", (MEMORY_END - MEMORY_START)>>20);
		
		buddy_descriptor_t* descriptor_ptr = (buddy_descriptor_t*) start;
		segments_info.infos[i].descriptors_start = (uint64_t) descriptor_ptr;
		segments_info.infos[i].pages_number = decriptors_number;
		segments_info.infos[i].pages_start = MEMORY_START;
		page_t* page = (page_t*) MEMORY_START;
		uint64_t page_index = 0;
		// descriptor_ptr->prev = NULL;
		for (uint64_t j = 0; j < decriptors_number; j++) {
			descriptor_ptr->page_num = page_index;
			descriptor_ptr->is_free = true;
			descriptor_ptr->block_order = get_order(page_index, i);
			// if (i == 0) {
			// 	printf("%d ", descriptor_ptr->block_order);
			// }
			
			descriptor_ptr->page_ptr = page;
			// descriptor_ptr->next = descriptor_ptr+1;
			// descriptor_ptr->next->prev = descriptor_ptr;
			descriptor_ptr++;
			page++;
			page_index++;
		}
		// if (i == 0) {
		// 	printf("\n");
		// }
	}
	
	printf("==================== allocator  initialized =====================\n\n");
	// descriptor_ptr->next = NULL;

}


