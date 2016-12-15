#ifndef __FS_H__
#define __FS_H__

#include <alloc.h>
#include <debug.h>
#include <string.h>
#include <print.h>
#include <spinlock.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_FILE_NAME_LENGTH 128
#define MAX_FILES_IN_DIR 256
#define FS_BLOCK_SIZE 512


typedef enum node_type {
	FILE,
	DIR
} node_t;

typedef struct fs_descriptor_list {
	struct fs_descriptor_list* next_;
	struct fs_descriptor_list* prev_;
	uint64_t data_ptr_;
	
} fs_descriptor_list_t;

typedef struct fs_node {
	node_t type_;
	char name_[MAX_FILE_NAME_LENGTH];
	struct fs_node* children_;
	uint64_t children_num_;
	fs_descriptor_list_t* list_head_;

	int opened_;

} fs_node_t;


fs_node_t* open(char* file_name);
void close(fs_node_t* node_ptr);
void mkdir(char* dir_name);


void write(fs_node_t* node_ptr, uint64_t offset, uint64_t data_begin, uint64_t data_size);
// void write_to_block(fs_descriptor_list_t* block, uint64_t offset, uint64_t data_begin, uint64_t data_size);

void read(fs_node_t* node_ptr, uint64_t offset, uint64_t data_begin, uint64_t data_size);
// void read_from_block(fs_descriptor_list_t* block, uint64_t offset, uint64_t data_begin, uint64_t data_size);

void init_file_system(int blocks_num);

typedef struct dir_names_list {
	struct dir_names_list* next_;
	char const* names_;
} dir_names_list_t;

dir_names_list_t* readdir(char* dir_name);

void print_dir_names_list(dir_names_list_t* dnl);

#endif /*__FS_H__*/