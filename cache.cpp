#include <algorithm>
#include <iostream>
#include "cache.h"
#include "debug.h"

#define DEFAULT_CACHE_SIZE 16*1024*1024 
/* DEFAULT CACHE SIZE IN BYTES */

cache::cache(long long size,struct DB* base)
{
	cache_size = size;
	mem_used = 0;
	db = base;
}

cache::~cache()
{
	size_t size = data.size();
	#ifdef _DEBUG_
		std::cout << "Cache destructor...\n";
		for(map_iter it = data.begin();it != data.end();it++)
		{
			std::cout << "Page : " << it->first <<"\n";	
			std::cout << "Is null: " << (it->second.data == NULL ? 0 : 1) << "\n";
			std::cout << "Size : " << it->second.size <<"\n";	
		}
			
	#endif
	if(!data.empty())
	{
		if(size > 0)
		{
			for(map_iter it = data.begin();it != data.end();it++)
			{	
				if(it->second.data != NULL)
					delete[] it->second.data;
			}	
		}
	}
}
/* 
 * Кэш создает в памяти новые элементы и копирует в них значение из указателя
 * Если элемент уже существует в кэше, а его обновили, он перезаписывается
 * Запись в кэш происходит после чтения элемента, и после записи элемента в базу данных
 * В кэше хранится страница полностью
 * Неплохо было бы написать кастомный аллокатор памяти 
 */

int cache::fetch(const struct DBT& page,long long page_num)
{
	#ifdef _DEBUG_
		std::cout << "Fetching page...\n";
	#endif
	map_iter it = data.find(page_num);
	if(it->first == page_num && it != data.end())
	{
		#ifdef _DEBUG_
		std::cout << "Node was found on page : "<<page_num<<"...\n";
		#endif
		it->second.size = page.size;
		for(int i = 0;i < it->second.size;i++)
			it->second.data[i] = page.data[i];

		return SUCC;
	}

	if(mem_used < cache_size - 1)
	{	
		#ifdef _DEBUG_
		std::cout << "Page wasn't found...\n";
		#endif
		struct DBT new_page;
		new_page.data = new char[page.size];
		new_page.size = page.size;
		
		for(int i = 0;i < new_page.size;i++)
			new_page.data[i] = page.data[i];

		map_iter iter = (data.insert(pair<long long,struct DBT>(page_num,new_page) )).first;
		priority.push_front(iter);
		mem_used++;
		return SUCC;
	}

	list_iter cold = priority.end();
	it = *cold;

	delete[] it->second.data;
	it->second.data = NULL;

	data.erase(it);
	priority.pop_back();
	mem_used--;

	return this->fetch(page,page_num);
}

int cache::flush(struct DBT& page,long long page_num)
{
	#ifdef _DEBUG_
		std::cout << "Flushing page...\n";
	#endif
	map_iter it = data.find(page_num);
	if(it->first == page_num)
	{
		if(page.data == NULL)
			page.data = new char[it->second.size];
		if(page.size != it->second.size)
			return FAIL;

		page.size = it->second.size;
		for(int i = 0;i < it->second.size;i++)
			page.data[i] = it->second.data[i];

		list_iter lit = find(priority.begin(),priority.end(),it);
		priority.erase(lit);
		priority.push_front(it);
	}
	
	return SUCC;
}

