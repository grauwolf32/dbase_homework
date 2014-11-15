#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mydb.h"
#include "debug.h"


class DBAllocator
{
 public:
	DBAllocator(struct DB* dat_base,long int Offset,size_t size);
	~DBAllocator();

	//friend void   set_bit(char* ptr,size_t size,long long n);
	//friend bool   get_bit(char* ptr,size_t size,long long n);
	//friend void unset_bit(char* ptr,size_t size,long long n);

	int    db_alloc(long long& page_num);
	void   db_free(long long page );
	void   db_add_head();

	void   db_write_table();
	void   db_read_table();
	void   db_refresh();
	void   db_remove();	
 private:
	struct DB* d_base;
	long long mem_size;
	long long last_ptr; //bits
	long long mem_used; //bits
	long long offset;   //bits
	char* 	 file_stat;
};

size_t last_set_bit(char* ptr,size_t size);
size_t memory_used(char* ptr,size_t size);

void   set_bit(char* ptr,size_t size,long long n);
bool   get_bit(char* ptr,size_t size,long long n);
void unset_bit(char* ptr,size_t size,long long n);

size_t fread_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count);
size_t fwrite_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count);
#endif 

