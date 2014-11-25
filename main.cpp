#include <iostream>
#include <time.h>
#include "mydb.h"
#include "debug.h"

using namespace std;

int main()
{
    struct DBC config;
    config.chunk_size = 4096;
    config.mem_size = 4*1024;
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
    int n = 3;
    std::cout <<"Enter n: ";
    std::cin >> n;
    int count = 0;
    time_t time = clock();
      for(int i = 0;i < n;i++)
      {
    	put(db,&Key,&Value);
      }	
    //time = clock() - time;
    //std::cout <<"Estimated time: "<<(double)time/CLOCKS_PER_SEC<<"\n";
    count = 0;
    tree_keys_count(db->head,count,db);
    std::cout <<"------------------------------------------------------------------------\n";
    std::cout << "Keys count : "<<count;
    print_tree(db->head,db,0);
   
     for(int i = 0;i < n;i++)
      {
	//std::cout <<"------------------------------------------------------------------\n";
    	delete_key(db->head,key,db);
	//count = 0;
       // tree_keys_count(db->head,count,db);
       // std::cout << "Keys count : "<<count;
	//print_tree(db->head,db,0);
	//std::cout <<"------------------------------------------------------------------\n";
      }	

    count = 0;
    tree_keys_count(db->head,count,db);
    std::cout <<"------------------------------------------------------------------------\n";
    std::cout << "Keys count : "<<count;
    print_tree(db->head,db,0);
    //erase_i_key(db->head,0,0,db);
   
   
    delete[] key;
    delete[] value;
	
    close(db);
    return 0;
}

