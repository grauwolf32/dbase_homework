#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "allocator.h"

#define BITS_IN_BYTE 8

DBAllocator::DBAllocator(struct DBC config,struct DB* dat_base,long int Offset,size_t size)
{
	d_base = dat_base; 
	offset = Offset;
	mem_size = size;

	file_stat = new char[size];
	if(fread_db(d_base->fd,file_stat,offset,sizeof(char),size) != size)
		std::cout <<"Reading allocator table error!\n";
	
	last_ptr = last_set_bit(file_stat,size);
	mem_used = memory_used(file_stat,size);
}
DBAllocator::~DBAllocator()
{
	db_write_table();
	if(file_stat != NULL)
		delete[] file_stat;

	d_base = NULL;
}
void  DBAllocator::db_write_table()
{
	if(file_stat != NULL && d_base->fd != NULL && mem_size != 0)
		if(fwrite_db(d_base->fd,file_stat,offset,sizeof(char),mem_size) != mem_size)
			std::cout <<"Writing allocator table error!\n";
}
int DBAllocator::db_alloc()
{
	
	if(mem_used < mem_size*BITS_IN_BYTE*sizeof(char))
	{
		if(last_ptr < mem_size*BITS_IN_BYTE*sizeof(char))		
		{	
			last_ptr++;
			mem_used++;
			set_bit(file_stat,mem_size,last_ptr);
			page_num = last_ptr;

			return 1;
		}
	
		for(int i = 0;i < mem_size;i++)
			for(int j = 0;j < sizeof(char)*BITS_IN_BYTE;j++)
				if(get_bit(file_stat,mem_size,i*sizeof(char)*BITS_IN_BYTE + j) == 0)	
					page_num =  i*sizeof(char)*BITS_IN_BYTE + j;
		mem_used++;
		set_bit(file_stat,mem_size,page_num);
		return 1;	
	}

	else return 0;	
}
void DBAllocator::db_free(unsigned long page_num)
{
	if(get_bit(file_stat,mem_size,page_num) == 0)return;
	if(page_num == last_ptr && last_ptr != 0)last_ptr--;
	unset_bit(file_stat,mem_size,page_num);
	mem_used--;
	return;
}
size_t fread_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count)
{
	int curr_offset = ftell(fd);
	size_t res = 0;
	fseek(fd,offset,SEEK_SET);
	res = fread(ptr,size,count,fd);
	fseek(fd,curr_offset,SEEK_SET);
	return res;
}
size_t fwrite_db(FILE* fd,void* ptr,long int offset,size_t size,size_t count)
{
	int curr_offset = ftell(fd);
	size_t res = 0;
	fseek(fd,offset,SEEK_SET);
	res = fwrite(ptr,size,count,fd);
	fseek(fd,curr_offset,SEEK_SET);
	return res;
}
size_t last_set_bit(char* ptr,size_t size)
{
	size_t last_not_null = 0;
	unsigned int offset = 0;
	for(size_t i = 0; i < size;i++)
	{
		for(char mask = 1; mask != 0;mask <<= 1)
		{
			if(ptr[i] and mask > 0)last_not_null = i*sizeof(char)*BITS_IN_BYTE + offset;
			offset++;
		}
	offset = 0;
	}
	return last_not_null;
}
size_t memory_used(char* ptr,size_t size)
{
	char mask = 1;
	size_t mem_used = 0;
	size_t last_not_null = 0;
	unsigned int offset = 0;
	for(size_t i = 0; i < size;i++)
	{
		for(char mask = 1; mask != 0;mask <<= 1)
		{
			if(ptr[i] and mask > 0)mem_used++;
			offset++;
		}
	offset = 0;
	}
	return mem_used;
}
void   set_bit(char* ptr,size_t size,unsigned long n)
{
	unsigned offset = n % sizeof(char)*BITS_IN_BYTE;
	unsigned segment = n / sizeof(char)*BITS_IN_BYTE;
	char mask = 1;
	mask <<= offset;
	ptr[segment] |= mask;
}
void unset_bit(char* ptr,size_t size,unsigned long n)
{
	unsigned offset = n % sizeof(char)*BITS_IN_BYTE;
	unsigned segment = n / sizeof(char)*BITS_IN_BYTE;
	char mask = 1;
	mask <<= offset;
	ptr[segment] &= ~mask;
}

bool   get_bit(char* ptr,size_t size,unsigned long n)
{
	unsigned offset = n % sizeof(char)*BITS_IN_BYTE;
	unsigned segment = n / sizeof(char)*BITS_IN_BYTE;
	char mask = 1;
	mask <<= offset;
	if(mask and ptr[segment] > 0)return 1;
	return 0;
}

