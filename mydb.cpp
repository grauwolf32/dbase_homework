#include <iostream>
#include "mydb.h"
#include "btree.h"
#include "allocator.h"
#include "cache.h"
#include "debug.h"

#define SUCC 1
#define FAIL 0
#define BITS_IN_BYTE 8


//--------------------------------------------------Функции инициализации-----------------------------------------------------
struct DB *dbcreate(const char *file,struct DBC conf)
{
	
 	 struct DB* new_base = new DB;	

	 new_base->config = NULL;
	 new_base->config = new struct DBC;
	 *new_base->config = conf;

 	 new_base->fd = NULL;
 	 new_base->fd = fopen(file,"wb+");

 	 if(new_base->fd == NULL)
	 	 std::cout<<"Error! File wasn't created!\n";

	 cache* db_cache = new cache(conf.mem_size,new_base);
	 new_base->db_cache = db_cache;

	 std::cout <<"Data base configuration: \n";
	 std::cout <<"Data base size:\t"<<new_base->config->db_size<<"\n";
	 std::cout <<"Chunk size:\t"<< new_base->config->chunk_size<<"\n";
	 std::cout <<"Cache size:\t"<<new_base->db_cache->cache_size<<"\n";
	
	 new_base->head =(BTreeNode*)NULL;
	 new_base->head = new BTreeNode;
		
	 /*int fseek ( FILE * stream, long int offset, int origin );
	   SEEK_SET	Beginning of file
	   SEEK_CUR	Current position of the file pointer
	   SEEK_END	End of file */
					
	 unsigned int key_cnt = BTREE_KEY_CNT, key_len = BTREE_KEY_LEN, val_len = BTREE_VAL_LEN,chld_cnt = BTREE_CHLD_CNT,errcode = 0;
	 std::cout <<"B-tree configuration: \n";
	 std::cout <<"key count:\t"<< key_cnt <<"\n";
	 std::cout <<"key length:\t"<< key_len <<"\n";
	 std::cout <<"value length:\t"<< val_len <<"\n";
	 std::cout <<"child count:\t"<< chld_cnt <<"\n";
	 /*
	  *  Можно потом дописать новый функционал, введя переменные длину ключа и значения, количество ключей
	  */

	 if((errcode = fwrite(&key_cnt,sizeof(key_cnt),1,new_base->fd)) != 1)std::cout << "key count wasn't write successesfully!\n";
	 if((errcode = fwrite(&key_len,sizeof(key_len),1,new_base->fd)) != 1)std::cout << "key len wasn't write successesfully!\n";
	 if((errcode = fwrite(&val_len,sizeof(val_len),1,new_base->fd)) != 1)std::cout << "value len wasn't write sucessesfully!\n";
	 if((errcode = fwrite(&chld_cnt,sizeof(chld_cnt),1,new_base->fd)) != 1)std::cout << "child count wasn't write sucessesfully\n";

	 if((errcode = fwrite(&new_base->config->db_size,sizeof(new_base->config->db_size),1,new_base->fd)) != 1)
	 {
		std::cout << "base size wasn't successesfull writen\n";
	 }
	 if((errcode =fwrite(&new_base->config->chunk_size,sizeof(new_base->config->chunk_size),1,new_base->fd)) != 1)
	 { 
		std::cout << "chunk size wasn't sucessesful written\n";
	 }
	 if((errcode =fwrite(&new_base->db_cache->cache_size,sizeof(new_base->db_cache->cache_size),1,new_base->fd)) != 1)
	 { 
		std::cout << "cache size wasn't sucessesful written\n";
	 }	
		
	 long int head_offset = ftell(new_base->fd);	
	 fwrite(&head_offset,sizeof(head_offset),1,new_base->fd);/* Загатовка позиции для будущего смещения корневого листа */
	 long long memory_size = (new_base->config->db_size)/(sizeof(char)*BITS_IN_BYTE) + 1;
	 fwrite(&memory_size,sizeof(memory_size),1,new_base->fd);

	 long int offset = write_offset(new_base->fd);
	 std::cout << "table offset:\t"<<offset<<"\n";
	 
	 DBAllocator* db_all = new DBAllocator(new_base,offset,memory_size);
	 db_all->db_write_table();

	 /*
	  * offset += sizeof(char)*db_all->mem_size; // Найти смещение 
	  * new_base->head_offset = offset;   //Старая версия
	  */

	 new_base->head_offset = ftell(new_base->fd);

	 long long head_page_num = 0;
	 db_all->db_add_head();

	 std::cout <<"head page num:"<<head_page_num<<"\n";
	 new_base->head->write_to_file(head_page_num,new_base);
	 	 
	 std::cout <<"head offset: " << new_base->head_offset << "\n";
	 offset = ftell(new_base->fd);

	 fseek(new_base->fd,head_offset,SEEK_SET);
	 fwrite(&new_base->head_offset,sizeof(new_base->head_offset),1,new_base->fd);
	 fseek(new_base->fd,offset,SEEK_SET);

	 new_base->db_all = db_all;
	 
	 new_base->close = close;
	 new_base->put = put;
	 new_base->get = get;
	 new_base->del = del;
	 
	 return new_base;
}

struct DB *dbopen(const char *file, struct DBC conf)
{
	struct DB* new_base = new DB;	

	new_base->config = NULL;
	new_base->config = new struct DBC;

 	new_base->fd = NULL;
 	new_base->fd = fopen(file,"rb+");

 	if(new_base->fd == NULL)
		std::cout<<"Error! File wasn't open!\n";

	 new_base->head = (BTreeNode*)NULL;
	 new_base->head = new BTreeNode;

	 unsigned int key_cnt = 0, key_len = 0, val_len = 0,chld_cnt = 0,errcode = 0;
	 long long cache_size = 0;

	 if((errcode = fread(&key_cnt,sizeof(key_cnt),1,new_base->fd)) != 1)std::cout << "key count wasn't read successesfully!\n";
	 if((errcode = fread(&key_len,sizeof(key_len),1,new_base->fd)) != 1)std::cout << "key len wasn't read successesfully!\n";
	 if((errcode = fread(&val_len,sizeof(val_len),1,new_base->fd)) != 1)std::cout << "value len wasn't read sucessesfully!\n";
	 if((errcode = fread(&chld_cnt,sizeof(chld_cnt),1,new_base->fd)) != 1)std::cout << "child count wasn't read sucessesfully\n";

	 if((errcode = fread(&new_base->config->db_size,sizeof(new_base->config->db_size),1,new_base->fd)) != 1)
	 {
		std::cout << "base size wasn't successesfull writen\n";
	 }
	 if((errcode = fread(&new_base->config->chunk_size,sizeof(new_base->config->chunk_size),1,new_base->fd)) != 1)
	 {
		std::cout << "chunk size wasn't sucessesful written\n";
	 }
	 if((errcode = fread(&cache_size,sizeof(cache_size),1,new_base->fd)) != 1)
	 {
		std::cout << "cache size wasn't sucessesful written\n";
	 }
	
	 cache* db_cache = new cache(cache_size,new_base);
	 new_base->db_cache = db_cache;
	 	 
	 std::cout <<"Data base configuration: \n";
	 std::cout <<"Data base size:\t"<<new_base->config->db_size<<"\n";
	 std::cout <<"Chunk size:\t"<< new_base->config->chunk_size<<"\n";
	 
	 std::cout <<"B-tree configuration: \n";
	 std::cout <<"key count:\t"<<key_cnt<<"\n";
	 std::cout <<"key length:\t"<<key_len<<"\n";
	 std::cout <<"value length:\t"<<val_len<<"\n";
	 std::cout <<"child count:\t"<<chld_cnt<<"\n";
	
	 long int head_offset = 0;
	 long int offset = 0;
	 long long memory_size = 0;
	
	 fread(&head_offset,sizeof(head_offset),1,new_base->fd);	
	 fread(&memory_size,sizeof(memory_size),1,new_base->fd);
	 fread(&offset,sizeof(offset),1,new_base->fd);	
	
	 DBAllocator* db_all = new DBAllocator(new_base,offset,memory_size);
	 db_all->db_read_table();

	 new_base->head_offset = head_offset;
	 new_base->head->read_from_file(0,new_base);
	 
	 new_base->db_all = db_all;

	 new_base->close = close;
	 new_base->put = put;
	 new_base->get = get;
	 new_base->del = del;

	 return new_base;
} 
/*------------------------Внутренние функции DB-------------------------*/
int close(struct DB *db)
{

	if(db->db_all != NULL)
	{
		delete db->db_all;
		db->db_all = NULL;
		std::cout << "Allocator was successesfully deleted\n";
	}
	if(db->fd != NULL)
	{
		fclose(db->fd);
		db->fd = NULL;
		std::cout << "File was successesfully deleted!\n";
	}

	if(db->head != NULL)
	{
		delete db->head;
		db->head = NULL;
		std::cout << "Tree root was successesfully deleted!\n";
	}
	
	if(db->config != NULL)
	{
		delete db->config;
		db->config = NULL;
		std::cout << "Tree config was successesfully deleted!\n";
	}
	if(db->db_cache != NULL)
	{
		delete db->db_cache;
		db->db_cache = NULL;
		std::cout << "Cache was successesfully deleted!\n";
	}
	delete db;
	db = NULL;
	return 0;
}

int get(const struct DB *db, const struct DBT *key, struct DBT *data)
{
	BTreeNode* result = new BTreeNode;
	int page = -1;
	if((page = search_key(db->head,key->data,db, result)) < 0);
		return FAIL;
	return read_page(db,result->vals[page],data);
	
}
int put(const struct DB *db, const struct DBT *key,const struct DBT *data)
{
	int key_num = 0;
	long long data_page = 0;
        int res = 1;
	res &= db->db_all->db_alloc(data_page);
	res &= write_page(db,data_page,data);
	if(!res)
		std::cout << "Error occured while puting element into database\n";

	key_num = insert_key(db->head,key->data,data_page,db);
	return key_num;	
} 

int del(const struct DB *db, const struct DBT *key)
{
	int res = SUCC;
	res &= delete_key(db->head,key->data,db);
	return res;
}
/*---------------------------------------------------------------------*/
long int write_offset(FILE* fd)
{
	long int offset = ftell(fd);
	fwrite(&offset,sizeof(offset),1,fd);

	long int current_offset = ftell(fd);

	fseek(fd,offset,SEEK_SET);
	fwrite(&current_offset,sizeof(current_offset),1,fd);
	fseek(fd,current_offset,SEEK_SET);

	return current_offset;
}

int read_page(const struct DB* db,long long page,struct DBT* node)/* Создан ли уже node нужного размера ?*/
{
	long long mem_read = 0;
	long int offset = 0;

	if(db->db_cache->flush(*node,page) == SUCC)
	{	
		return SUCC;
	}

	struct DBT* data_page = new struct DBT;

	data_page->size = db->config->chunk_size;
	data_page->data = new char[db->config->chunk_size];
	
	offset = db->head_offset + db->config->chunk_size*page;
	if((mem_read = fread_db(db->fd,data_page->data,offset,sizeof(char),db->config->chunk_size)) != db->config->chunk_size)
	{
		std::cout <<"Error! Node isn't read successesful!\n";
		std::cout <<"Read: "<< mem_read <<"\n";
		std::cout <<"Chunk size: "<<db->config->chunk_size <<"\n";	

		if(data_page->data != NULL)delete[] data_page->data;
		if(data_page != NULL)delete data_page;
		return FAIL;
	}

	memcpy(node->data,data_page->data,node->size);
	db->db_cache->fetch(*data_page,page);

	if(data_page->data != NULL)delete[] data_page->data;
	if(data_page != NULL)delete data_page;

	return SUCC;
	
}

int write_page(const struct DB* db,long long page,const struct DBT* node)
{
	long long mem_write = 0;
	long int offset = 0;
	
	struct DBT* data_page = new struct DBT;

	data_page->size = db->config->chunk_size;
	data_page->data = new char[db->config->chunk_size];
	memset(data_page->data,0,db->config->chunk_size);	
	
	offset = db->head_offset + db->config->chunk_size*page;
	if(node == NULL)
	{
		fwrite_db(db->fd,data_page->data,offset,sizeof(char),db->config->chunk_size);

		if(data_page->data != NULL)delete[] data_page->data;
		if(data_page != NULL)delete data_page;

		return SUCC;
	}
	
	memcpy(data_page->data,	node->data,node->size);

	if((mem_write = fwrite_db(db->fd,data_page->data,offset,sizeof(char),db->config->chunk_size)) != db->config->chunk_size) 
	{ 
		std::cout <<"Error! Node isn't writen successesful!\n";
		std::cout <<"Write: "<< mem_write <<"\n";
		std::cout <<"Chunk size: "<<db->config->chunk_size <<"\n";
		
		if(data_page->data != NULL)delete[] data_page->data;
		if(data_page != NULL)delete data_page;		

		return FAIL;
	}
	
	db->db_cache->fetch(*data_page,page);

	if(data_page->data != NULL)delete[] data_page->data;
	if(data_page != NULL)delete data_page;

	return SUCC;
}

/*----------------------Функции для тестирования-------------------------*/

int db_close(struct DB *db) {
	db->close(db);
}

int db_del(const struct DB *db, void *key, size_t key_len) {
	struct DBT keyt = {
		.data = (char*)key,
		.size = key_len
	};
	return db->del(db, &keyt);
}

int db_get(const struct DB *db, void *key, size_t key_len,
		void **val, size_t *val_len) {
	struct DBT keyt = {
		.data = (char*)key,
		.size = key_len
	};
	struct DBT valt = {0, 0};
	int rc = db->get(db, &keyt, &valt);
	*val = valt.data;
	*val_len = valt.size;
	return rc;
}

int db_put(const struct DB *db, void *key, size_t key_len,
		void *val, size_t val_len) {
	struct DBT keyt = {
		.data = (char*)key,
		.size = key_len
	};
	struct DBT valt = {
		.data = (char*)val,
		.size = val_len
	};
	return db->put(db, &keyt, &valt);
}
