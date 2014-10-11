#include <iostream>
#include "mydb.h"
#include "btree.h"
#include "allocator.h"

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

	 std::cout <<"Data base configuration: \n";
	 std::cout <<"Data base size:\t"<<new_base->config->db_size<<"\n";
	 std::cout <<"Chunk size:\t"<< new_base->config->chunk_size<<"\n";
	
	new_base->head = new BTreeNode;
	memset(new_base->head,1,sizeof(new_base->head));
		
	 /*int fseek ( FILE * stream, long int offset, int origin );
	   SEEK_SET	Beginning of file
	   SEEK_CUR	Current position of the file pointer
	   SEEK_END	End of file */
					
	 int key_cnt = BTREE_KEY_CNT, key_len = BTREE_KEY_LEN, val_len = BTREE_VAL_LEN,chld_cnt = BTREE_CHLD_CNT,errcode = 0;
	 std::cout <<"B-tree configuration: \n";
	 std::cout <<"key count:\t"<<key_cnt<<"\n";
	 std::cout <<"key length:\t"<<key_len<<"\n";
	 std::cout <<"value length:\t"<<key_cnt<<"\n";
	 std::cout <<"child count:\t"<<chld_cnt<<"\n";
	 /*
	   Можно потом дописать новый функционал, введя переменные длину ключа и значения, количество ключей
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
	 
	 long int head_offset = write_offset(new_base->fd);/* Загатовка позиции для будущего смещения корневого листа */
	 unsigned long memory_size = (conf.db_size/conf.chunk_size) + 1;
	 std::cout <<"head offset: " << head_offset << "\n";
	 fwrite(&memory_size,sizeof(memory_size),1,new_base->fd);
	
	 long int offset = write_offset(new_base->fd);
	 DBAllocator* db_all = new DBAllocator(conf,new_base,offset,memory_size);
	 
	 memset(db_all->file_stat,0,db_all->mem_size*sizeof(char));
	 db_all->db_write_table();
	
	 offset += sizeof(char)*db_all->mem_size; /*Найти смещение конца таблицы*/
	 new_base->head_offset = offset; 
	 new_base->head->write_to_file(0,new_base);
	
	 fseek(new_base->fd,head_offset,SEEK_SET);
	 fwrite(&offset,sizeof(offset),1,new_base->fd);
	 fseek(new_base->fd,offset,SEEK_SET);
	 fprintf(new_base->fd,"%ld",offset);// Отладка
  	 
	 new_base->db_all = db_all;
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
	
	 new_base->head = new BTreeNode;
	 memset(new_base->head,0,sizeof(new_base->head));

	 int key_cnt = 0, key_len = 0, val_len = 0,chld_cnt = 0,errcode = 0;

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
	 	 
	 std::cout <<"Data base configuration: \n";
	 std::cout <<"Data base size:\t"<<new_base->config->db_size<<"\n";
	 std::cout <<"Chunk size:\t"<< new_base->config->chunk_size<<"\n";
	 
	 std::cout <<"B-tree configuration: \n";
	 std::cout <<"key count:\t"<<key_cnt<<"\n";
	 std::cout <<"key length:\t"<<key_len<<"\n";
	 std::cout <<"value length:\t"<<key_cnt<<"\n";
	 std::cout <<"child count:\t"<<chld_cnt<<"\n";
	
	 long int head_offset = 0;
	 long int offset = 0;
	 unsigned long memory_size = 0;
	
	 fread(&head_offset,sizeof(head_offset),1,new_base->fd);	
	 fread(&memory_size,sizeof(memory_size),1,new_base->fd);
	 fread(&offset,sizeof(offset),1,new_base->fd);	
	
	 DBAllocator* db_all = new DBAllocator(conf,new_base,offset,memory_size);
	 db_all->db_read_table();

	 new_base->head_offset = head_offset;
	 new_base->head = new BTreeNode;
	 new_base->head->read_from_file(0,new_base);
	 
	 new_base->db_all = db_all;
	 return new_base;
} 

int close(struct DB *db)
{
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
	if(db->db_all != NULL)
	{
		delete db->db_all;
		db->db_all = NULL;
		std::cout << "Allocator was successesfully deleted\n";
	}
	delete db;
	db = NULL;
	return 0;
}

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


//----------------------Функции для тестирования-------------------------
int db_close(struct DB *db) {
	db->close(db);
}

int db_del(const struct DB *db, void *key, size_t key_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	return db->del(db, &keyt);
}

int db_get(const struct DB *db, void *key, size_t key_len,
		void **val, size_t *val_len) {
	struct DBT keyt = {
		.data = key,
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
		.data = key,
		.size = key_len
	};
	struct DBT valt = {
		.data = val,
		.size = val_len
	};
	return db->put(db, &keyt, &valt);
}
