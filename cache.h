#ifndef CACHE_H
#define CACHE_H

#include <map>
#include <list>
#include <iostream>

#include "mydb.h"
#include "debug.h"

#define SUCC 1
#define FAIL 0

using namespace std;

typedef map<long long,struct DBT>::iterator map_iter;
typedef map<long long,struct DBT> page_map;
typedef list<map_iter>::iterator list_iter;
typedef list<map_iter> page_list;


class cache
{
	public:
	cache(long long size,struct DB* base);
	~cache();

	int fetch(const struct DBT& page,long long page_num);
	int flush(struct DBT& page,long long page_num);

	page_map data;
	page_list priority;

	/*Add list for dirty pages*/

	struct DB* db;
	long long cache_size;
	long long mem_used;
};

#endif
