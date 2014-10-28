#include <iostream>
#include "mydb.h"

using namespace std;

int main()
{
    struct DBC config;
    config.chunk_size = 4096;
    config.db_size = 1024*1024*2;
    struct DB* db = dbcreate("base.db",config);
    close(db);
    db = dbopen("base.db",config);

    char* key = new char[BTREE_KEY_LEN];
    char* value = new  char[BTREE_VAL_LEN];
    memset(key,0x11,BTREE_KEY_LEN);
    memset(value,0x22,BTREE_VAL_LEN);
    struct DBT Key;
    struct DBT Value;
    Value.data = value;
    Value.size = BTREE_VAL_LEN;
    Key.data = key;
    Key.size = BTREE_KEY_LEN;
    put(db,&Key,&Value);

    close(db);
    return 0;
}

