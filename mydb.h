#ifndef MYDB_H
#define MYDB_H

#include <stddef.h>
#include "btree.h"
#include "allocator.h"
/* check `man dbopen` */

struct DBT {
	char  *data;
	size_t size;
};

struct DBC {
     
        size_t db_size;		//Maximum on-disk file size  512MB by default
        size_t chunk_size;	//Maximum chunk (node/data chunk) size 4KB by default	


	/* For future uses - maximum cached memory size
	 * 16MB by default
	 * size_t mem_size; */
};

class DBAllocator;
class BTreeNode;

struct DB {

	int (*close)(struct DB *db);
	int (*del)(const struct DB *db, const struct DBT *key);
	
	int (*get)(const struct DB *db, const struct DBT *key, struct DBT *data);
	int (*put)(const struct DB *db, const struct DBT *key, const struct DBT *data);
	
	struct DBC* config;
	DBAllocator* db_all;
	FILE*	     fd;
	BTreeNode* head;
	long int head_offset;		

}; /* Need for supporting multiple backends (HASH/BTREE) */

int close(struct DB *db);
int put(const struct DB *db, const struct DBT *key,struct DBT *data);
int get(const struct DB *db, const struct DBT *key, struct DBT *data);


/* don't store metadata in the file */
struct DB *dbcreate(const char *file, struct DBC conf);
struct DB *dbopen  (const char *file, struct DBC conf); //const struct DBC conf не нужен здесь, он используется только при конфигурировании системы 
//Размер чанка задается только при создании базы данных 

int read_page(const struct DB* db,unsigned long page,struct DBT* node);
int write_page(const struct DB* db,unsigned long page,struct DBT* node);
long int write_offset(FILE* fd); /*Фсункция для написания смещений*/

int db_close(struct DB *db);
int db_del(const struct DB *, void *, size_t);
int db_get(const struct DB *, void *, size_t, void **, size_t *);
int db_put(const struct DB *, void *, size_t, void * , size_t  );

/* For future uses - sync cached pages with disk
 * int db_sync(const struct DB *db);
 * */
#endif
