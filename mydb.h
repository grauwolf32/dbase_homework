#ifndef MYDB_H
#define MYDB_H

#include <stddef.h>
#include "btree.h"
#include "allocator.h"
/* check `man dbopen` */

struct DBT {
	void  *data;
	size_t size;
};

class DBAllocator;
class BTreeNode;

struct DB {

	int (*close)(struct DB *db);
	int (*del)(const struct DB *db, const struct DBT *key);
	
	int (*get)(const struct DB *db, const struct DBT *key, struct DBT *data);
	int (*put)(const struct DB *db, const struct DBT *key, const struct DBT *data);
	
	DBAllocator* db_all;
	FILE*	     fd;
	BTreeNode* head;
	long int head_offset;		
	

}; /* Need for supporting multiple backends (HASH/BTREE) */

struct DBC {
     
        size_t db_size;		//Maximum on-disk file size  512MB by default
        size_t chunk_size;	//Maximum chunk (node/data chunk) size 4KB by default	


	/* For future uses - maximum cached memory size
	 * 16MB by default
	 * size_t mem_size; */
};

/* don't store metadata in the file */
struct DB *dbcreate(const char *file, const struct DBC conf);
struct DB *dbopen  (const char *file, const struct DBC conf);

int db_close(struct DB *db);
int db_del(const struct DB *, void *, size_t);
int db_get(const struct DB *, void *, size_t, void **, size_t *);
int db_put(const struct DB *, void *, size_t, void * , size_t  );

/* For future uses - sync cached pages with disk
 * int db_sync(const struct DB *db);
 * */
#endif
