#include "mydb.h"

struct DB *dbcreate(const char *file, const struct DBC conf)
{
 struct DB* new_base = new DB;	
 new_base->fd = NULL;
 new_base->fd = fopen(file,"wb+");

 if(new_base->fd == NULL)
	printf("Error! File wasn't created!\n");

 new_base->head = new BTreeNode;
 memset(new_base->head,0,sizeof(new_base->head));

 fprintf(new_base->fd,"%d%d%d%d",BTREE_KEY_CNT,BTREE_KEY_LEN,BTREE_VAL_LEN,BTREE_CHLD_CNT);
 fprintf(new_base->fd,"%d%d",(int)conf.db_size,(int)conf.chunk_size);

 
 long int offset = ftell(new_base->fd); //Узнаем текущее смещение
 db_all = new DBAllocator(conf,new_base,offset,(conf.db_size/conf.chunk_size) + 1);//Пишем таблицу аллокатора

 fprintf(new_base->fd,"%d",db_all->mem_size*sizeof(char));
 memset(db_all->file_stat,0,db_all->mem_size*sizeof(char));
 fwrite(db_all->file_stat,sizeof(char),db_all->mem_size,fd);
	
 offset = ftell(new_base->fd); //Узнаем текущее смещение
 fprintf(new_base->fd,"%d",(int)(offset + sizeof(offset))); //Пишем смещение главного листа B-дерева
 fwrite(new_base->head,1,sizeof(new_base->head),new_base->fd);

 head_offset = offset + sizeof(offset);
 
 return new_base;
}

int close(struct DB *db);
{
	if(db->head != NULL)
	{
		delete db->head;
		db->head = NULL;
	}
	if(db->fd != NULL)
	{
		fclose(db->fd);
	}
	if(db->db_all != NULL)
	{
		delete db->db_all;
	}
	delete db;
	return 0;
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
