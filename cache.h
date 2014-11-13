#ifndef CACHE_H
#define CACHE_H

#include <set>
#include "mydb.h"

using namespace std;

class cache
{
	public:
	cache();
	~cache();
	void fetch(struct DBT* page);
	void flush(struct DBT* page);
	set<struct DBT> data;
	private:
	long long cache_size;
	long long mem_used;
};

#endif
