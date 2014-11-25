#ifndef MYDB_H
#define MYDB_H

#include <stddef.h>
#include "btree.h"
#include "allocator.h"
#include "cache.h"
#include "debug.h"
/* check `man dbopen` */

struct DBT {
	char  *data;
	size_t size;
};

struct DBC {
     
        size_t db_size;		//Maximum on-disk file size  512MB by default
        size_t chunk_size;	//Maximum chunk (node/data chunk) size 4KB by default	
	size_t mem_size;

	/* For future uses - maximum cached memory size
	 * 16MB by default
	 */
};

class DBAllocator;
class BTreeNode;
class cache;

struct DB {

	int (*close)(struct DB *db);
	int (*del)(const struct DB *db, const struct DBT *key);
	
	int (*get)(const struct DB *db, const struct DBT *key, struct DBT *data);
	int (*put)(const struct DB *db, const struct DBT *key, const struct DBT *data);
	
	struct DBC* config;
	DBAllocator* db_all;
	FILE*	     fd;
	BTreeNode* head;
	cache* db_cache;
	long int head_offset;		

}; /* Need for supporting multiple backends (HASH/BTREE) */

int close(struct DB *db);
int put(const struct DB *db, const struct DBT *key,const struct DBT *data);
int get(const struct DB *db, const struct DBT *key, struct DBT *data);
int del(const struct DB *db, const struct DBT *key);

int read_page(const struct DB* db,long long page,struct DBT* node);
int write_page(const struct DB* db,long long page,const struct DBT* node);
long int write_offset(FILE* fd); /*Фсункция для написания смещений*/

#ifdef __cplusplus
extern "C" {
#endif
/* don't store metadata in the file */
struct DB *dbcreate(const char *file, struct DBC conf);
struct DB *dbopen  (const char *file, struct DBC conf); //const struct DBC conf не нужен здесь, он используется только при конфигурировании системы 
//Размер чанка задается только при создании базы данных 

int db_close(struct DB *db);
int db_del(const struct DB *, void *, size_t);
int db_get(const struct DB *, void *, size_t, void **, size_t *);
int db_put(const struct DB *, void *, size_t, void * , size_t  );
#ifdef __cplusplus
}
#endif

/* For future uses - sync cached pages with disk
 * int db_sync(const struct DB *db);
 * */
#endif
