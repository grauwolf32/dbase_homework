#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "mydb.h"


class DBAllocator
{
 public:
	friend struct DB *dbcreate(const char *file, const struct DBC conf);
	DBAllocator(struct DBC config,struct DB* dat_base,long int Offset,size_t size);
	~DBAllocator();
	
	int    db_alloc(unsigned long& page_num);
	void   db_free(unsigned long page );
	void   db_write_table();	
 private:
	struct DB* d_base;
	unsigned long mem_size;
	unsigned long last_ptr; //bits
	unsigned long mem_used; //bits
	unsigned long offset;   //bits
	char* 	 file_stat;
};

size_t last_set_bit(char* ptr,size_t size);
size_t memory_used(char* ptr,size_t size);

void   set_bit(char* ptr,size_t size,unsigned long n);
bool   get_bit(char* ptr,size_t size,unsigned long n);
void unset_bit(char* ptr,size_t size,unsigned long n);

size_t fread_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count);
size_t fwrite_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count);
#endif 

