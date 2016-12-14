#include <file_system.h>


void strcopy(char* from, char* to) { // assumes that sizes are ok
	char* current_symb = to;
	for (char* c = from; *c != '\0'; c++)
	{
		*current_symb = *c;
		current_symb++;
	}
	*current_symb = '\0';
}

static int streq(char* from, char* to) { // assumes that sizes are ok
	char* current_symb = to;
	for (char* c = from; *c != '\0'; c++)
	{
		if (*current_symb != *c) {
			return 0;
		}
		current_symb++;
	}
	return 1;
}




static struct spinlock fs_spinlock;
static fs_descriptor_list_t* fs_descriptors_head;
static fs_node_t* fs_root_ptr;

void lock() {
	spin_lock(&fs_spinlock);
}
void unlock() {
	spin_unlock(&fs_spinlock);
}

void allocate_fs_blocks(uint64_t blocks_num) {
	fs_descriptors_head = NULL;
	for (uint64_t i = 0; i < blocks_num; i++) {
		fs_descriptor_list_t* new_decsriptor = 
			(fs_descriptor_list_t*) mem_alloc(sizeof(fs_descriptor_list_t));
		new_decsriptor->prev_ = NULL;
		new_decsriptor->next_ = fs_descriptors_head;
		if (fs_descriptors_head != NULL) {
			fs_descriptors_head->prev_ = new_decsriptor;
		}

		new_decsriptor->data_ptr_ = (uint64_t) mem_alloc(FS_BLOCK_SIZE);
		fs_descriptors_head = new_decsriptor;
	}
}

fs_descriptor_list_t* get_fs_block() {
	if (fs_descriptors_head == NULL) {
		BUG("no filesystem free blocks left\n");
	}
	fs_descriptor_list_t* res = fs_descriptors_head;
	fs_descriptors_head = fs_descriptors_head->next_;
	if (fs_descriptors_head != NULL) {
		fs_descriptors_head->prev_ = NULL;
	}
	return res;
}

void init_fs_node(fs_node_t* node_ptr, char* name, node_t type) {
	node_ptr->type_ = type;
	strcopy(name, node_ptr->name_);
	if (node_ptr->type_ == DIR) {
		node_ptr->children_ = (fs_node_t*) mem_alloc(sizeof(fs_node_t) * MAX_FILES_IN_DIR);
		node_ptr->list_head_ = NULL;
	}
	else {
		node_ptr->children_ = NULL;
		node_ptr->list_head_ = NULL;
	}
	node_ptr->blocks_num_ = 0;
	node_ptr->children_num_ = 0;
	node_ptr->opened_ = 0;
}


typedef struct name_array {
	char** array_;
	int levels_num_;
} name_array_t;


void delete_name_array(name_array_t array) {
	for (int i = 0; i < array.levels_num_; i++) {
		mem_free(array.array_[i]);
	}
	mem_free(array.array_);
}

name_array_t name_to_levels_array(char* file_name) {
	int levels_num = 0;	
	int current_level_num = 0;
	int length = strlen(file_name);
	for (char* c = file_name; *c != '\0'; c++) {
		if (*c == '/') {
			levels_num++;
		}
	}
	char** levels = (char**) mem_alloc(sizeof(char*) * levels_num);
	int level_start = 1;
	int level_end = 1;
	int i;
	for (i = 1; i < length; i++) {
		if (file_name[i] == '/') {
			level_end = i;
			BUG_ON(level_end == level_start);
			char* current_level = (char*) mem_alloc(level_end - level_start + 1);
			for (int j = level_start; j < level_end; j++) {
				
				current_level[j - level_start] = file_name[j];
			}
			current_level[level_end - level_start] = '\0';
			levels[current_level_num] = current_level;
			level_start = i + 1;

			current_level_num++;
		}
	}

	level_end = i;
	BUG_ON(level_end == level_start);
	char* current_level = (char*) mem_alloc(level_end - level_start + 1);
	for (int j = level_start; j < level_end; j++) {
		current_level[j - level_start] = file_name[j];
	}
	current_level[level_end - level_start] = '\0';
	levels[current_level_num] = current_level;
	level_start = i + 1;
	current_level_num++;


	name_array_t res;
	res.array_ = levels;
	res.levels_num_ = levels_num;
	return res;
}

void print_name_array(name_array_t levels) {
	for (int i = 0; i < levels.levels_num_; i++) {
		printf(levels.array_[i]);
		printf(" ");
	}
	printf("\n");
}

fs_node_t* find_node(char* name, node_t type) { 
	// returns pointer to element
	name_array_t levels = name_to_levels_array(name);
	fs_node_t* current_node_ptr = fs_root_ptr;
	for (int i = 0; i < levels.levels_num_ - 1; i++) {
		BUG_ON(current_node_ptr->type_ != DIR);
		uint64_t j;
		uint64_t temp_max_j = current_node_ptr->children_num_;
		for (j = 0; j < current_node_ptr->children_num_; j++) {
			// printf(levels.array_[i]);
			// printf("\n");
			// printf(current_node_ptr->children_[j].name_, "\n" );
			// printf("\n%d\n", streq(levels.array_[i], current_node_ptr->children_[j].name_));
			if (streq(levels.array_[i], current_node_ptr->children_[j].name_)) {
				current_node_ptr = &current_node_ptr->children_[j];
				break;
			}
		}
		BUG_ON(j == temp_max_j);
	}
	BUG_ON(current_node_ptr->type_ != DIR);
	for (uint64_t j = 0; j < current_node_ptr->children_num_; j++) {
		if (streq(levels.array_[levels.levels_num_ - 1], current_node_ptr->children_[j].name_)) {
			delete_name_array(levels);
			return &current_node_ptr->children_[j];
		}
	}
	BUG_ON(current_node_ptr->children_num_ > 256);
	current_node_ptr->children_num_++;;
	init_fs_node(&current_node_ptr->children_[current_node_ptr->children_num_ - 1],
				 levels.array_[levels.levels_num_ - 1],
				 type);
	delete_name_array(levels);
	return &current_node_ptr->children_[current_node_ptr->children_num_ - 1];
}

int is_opened(char* file_name) {
	lock();
	return find_node(file_name, FILE)->opened_;
	unlock();
}
void open(char* file_name) {
	lock();
	fs_node_t* node_ptr = find_node(file_name, FILE);
	BUG_ON(node_ptr->opened_);
	node_ptr->opened_ = 1;
	unlock();
}
void close(char* file_name) {
	lock();
	fs_node_t* node_ptr = find_node(file_name, FILE);
	node_ptr->opened_ = 0;
	unlock();
}
void mkdir(char* dir_name) {
	lock();
	find_node(dir_name, DIR);
	unlock();
}


dir_names_list_t* readdir(char* dir_name) {
	lock();
	fs_node_t* node_ptr = find_node(dir_name, DIR);
	int children_num = node_ptr->children_num_;
	dir_names_list_t head_np;
	dir_names_list_t* head = &head_np;
	dir_names_list_t* current_node = head;
	for (int i = 0; i < children_num; i++) {
		dir_names_list_t* next_node = (dir_names_list_t*) mem_alloc(sizeof(dir_names_list_t));\
		next_node->names_ = node_ptr->children_[i].name_;
		current_node->next_ = next_node;
		next_node->next_ = NULL;
		current_node = next_node;
	}
	head = head->next_;
	unlock();
	return head;
}


void write_to_block(fs_descriptor_list_t* block, uint64_t offset, uint64_t data_begin, uint64_t data_size) {
	if (offset > FS_BLOCK_SIZE) {
		if (block->next_ == NULL) {
			fs_descriptor_list_t* next_block = get_fs_block();
			block->next_ = next_block;
			next_block->prev_ = block;
		}
		write_to_block(block->next_, offset - FS_BLOCK_SIZE, data_begin, data_size);
	}	
	uint8_t* block_byte = (uint8_t*) block->data_ptr_ + offset;
	uint8_t* byte;
	for (byte = (uint8_t*) data_begin; 
		 byte < (uint8_t*) (data_begin + data_size) && 
		 	byte < ((uint8_t*) data_begin) + FS_BLOCK_SIZE - offset;
		 byte++) {
		*block_byte = *byte;
		block_byte++;
	}
	if ((uint64_t) byte < data_begin + data_size) {
		if (block->next_ == NULL) {
			fs_descriptor_list_t* next_block = get_fs_block();
			block->next_ = next_block;
			next_block->prev_ = block;
		}
		write_to_block(block->next_, 0, (uint64_t) byte, data_size - FS_BLOCK_SIZE);
	}
}

void write(char* name, uint64_t offset, uint64_t data_begin, uint64_t data_size) {
	lock();
	fs_node_t* node_ptr = find_node(name, FILE);
	BUG_ON(!node_ptr->opened_);
	fs_descriptor_list_t* current_block = node_ptr->list_head_;
	if (current_block == NULL) {
		current_block = get_fs_block();
		node_ptr->list_head_ = current_block;
	}
	write_to_block(current_block, offset, data_begin, data_size);
	unlock();
}


void read_from_block(fs_descriptor_list_t* block, uint64_t offset, uint64_t data_begin, uint64_t data_size) {
	if (offset > FS_BLOCK_SIZE) {
		read_from_block(block->next_, offset - FS_BLOCK_SIZE, data_begin, data_size);
	}	
	uint8_t* block_byte = (uint8_t*) block->data_ptr_ + offset;
	uint8_t* byte;
	for (byte = (uint8_t*) data_begin; 
		 byte < (uint8_t*) (data_begin + data_size) && 
		 	byte < ((uint8_t*) data_begin) + FS_BLOCK_SIZE - offset;
		 byte++) {
		*byte = *block_byte;
		block_byte++;
	}
	if ((uint64_t) byte < data_begin + data_size) {
		read_from_block(block->next_, 0, (uint64_t) byte, data_size - FS_BLOCK_SIZE);
	}
}

void read(char* name, uint64_t offset, uint64_t data_begin, uint64_t data_size) {
	lock();
	fs_node_t* node_ptr = find_node(name, FILE);
	BUG_ON(!node_ptr->opened_);

	fs_descriptor_list_t* current_block = node_ptr->list_head_;
	BUG_ON(current_block == NULL);
	BUG_ON(node_ptr->blocks_num_ * FS_BLOCK_SIZE >= offset + data_size);
	read_from_block(current_block, offset, data_begin, data_size);
	unlock();
}

void init_file_system(int blocks_num) {
	spin_setup(&fs_spinlock);
	lock();
	allocate_fs_blocks(blocks_num);
	fs_root_ptr = mem_alloc(sizeof(fs_node_t));
	init_fs_node(fs_root_ptr, "/", DIR);
	unlock();
}




void print_dir_names_list(dir_names_list_t* dnl) {
	while (dnl != NULL) {
		printf(dnl->names_);
		printf(", ");
		dnl = dnl->next_;
	}
	printf("\n");
}

