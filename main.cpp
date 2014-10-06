#include <iostream>
#include "mydb.h"

using namespace std;

int main()
{
    struct DBC conf;
    conf.db_size = 512*1024;
    conf.chunk_size = 1024;
    struct DB* base = dbcreate("my_first_db.db",conf);
    return 0;
}
