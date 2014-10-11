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
    close(db);
    return 0;
}
