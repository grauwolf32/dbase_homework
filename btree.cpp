#include <string.h>
#include <iostream>
#include "btree.h"
#include "mydb.h"
#include "allocator.h"

#define SUCC 1
#define FAIL 0

//---------------------------------------Функции класса--------------------------------
BTreeNode::BTreeNode()
{
	chld = new long long[BTREE_CHLD_CNT];
	keys = new char[BTREE_KEY_CNT*BTREE_KEY_LEN];
	vals = new long long[BTREE_CHLD_CNT];
	

	memset(keys,0,BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	for(int i = 0; i < BTREE_CHLD_CNT;i++)chld[i] = -1;
	for(int i = 0; i < BTREE_CHLD_CNT;i++)vals[i] = -1;
	
	struct_size = 0;
	struct_size += sizeof(page);
	struct_size += sizeof(nKeys);
	struct_size += sizeof(leaf);
	struct_size += BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char);
	struct_size += BTREE_CHLD_CNT*sizeof(long long);
	struct_size += BTREE_KEY_CNT*sizeof(long long);	

	page = -1;
	nKeys = 0;
	leaf = 1;
}


BTreeNode::~BTreeNode()
{
	if(chld != NULL)delete[] chld;
	if(keys != NULL)delete[] keys;
	if(vals != NULL)delete[] vals;
	chld = NULL;
	keys = NULL;
	vals = NULL;
}

BTreeNode& BTreeNode::operator=(const BTreeNode& rnode)
{
	if(&rnode == this)return *this;
	
	if(chld != NULL) delete[] chld;
	if(keys != NULL) delete[] keys;
	if(vals != NULL) delete[] vals;
	
	if(rnode.keys != NULL && rnode.chld != NULL && rnode.vals != NULL)
	{	
		chld = new long long[BTREE_CHLD_CNT];
		keys = new char [BTREE_KEY_CNT*BTREE_KEY_LEN];
		vals = new long long[BTREE_CHLD_CNT];
		for(long long i = 0; i < BTREE_CHLD_CNT;i++) chld[i] = rnode.chld[i];
		for(long long i = 0; i < BTREE_KEY_CNT*BTREE_KEY_LEN;i++) keys[i] = rnode.keys[i];
		for(long long i = 0; i < BTREE_CHLD_CNT;i++) vals[i] = rnode.vals[i];
	
	}
	
	page = rnode.page;
	nKeys = rnode.nKeys;
	leaf = rnode.leaf;
	
	return *this;
}

int 	 BTreeNode::read_from_file(long long page,const struct DB* db)
{
	struct DBT node;
	unsigned int ptr = 0;
	
	node.data = new char[db->config->chunk_size];
	node.size = db->config->chunk_size;
	memset(node.data,0,node.size);

	read_page(db,page,&node);
	ptr = 0;

	memcpy((char*)&this->page,node.data,sizeof(page));	
	ptr += 	sizeof(page);
	memcpy((char*)&this->nKeys,(node.data + ptr),sizeof(nKeys));
	ptr += sizeof(nKeys);
	memcpy((char*)&this->leaf,(node.data + ptr),sizeof(leaf));
	ptr += sizeof(leaf);
	memcpy((char*)this->keys,(node.data + ptr),BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	ptr += BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char);
	memcpy((char*)this->chld,(node.data + ptr),BTREE_CHLD_CNT*sizeof(long long));
	ptr += BTREE_CHLD_CNT*sizeof(long long);
	memcpy((char*)this->vals,(node.data + ptr),BTREE_KEY_CNT*sizeof(long long));
	ptr += BTREE_KEY_CNT*sizeof(long long);
	
	delete[] node.data;
	return SUCC;
}
int	 BTreeNode::write_to_file(long long page,const struct DB* db)
{
	struct DBT node;
	unsigned int ptr = 0;
	node.data = new char[struct_size];
	node.size = struct_size;

	memset(node.data,0,struct_size);
	memcpy(node.data,&page,sizeof(page));	
	ptr += 	sizeof(page);
	memcpy((node.data + ptr),&nKeys,sizeof(nKeys));
	ptr += sizeof(nKeys);
	memcpy((node.data + ptr),&leaf,sizeof(leaf));
	ptr += sizeof(leaf);
	memcpy((node.data + ptr),keys,BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	ptr += BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char);
	memcpy((node.data + ptr),chld,BTREE_CHLD_CNT*sizeof(long long));
	ptr += BTREE_CHLD_CNT*sizeof(long long);
	memcpy((node.data + ptr),vals,BTREE_KEY_CNT*sizeof(long long));
	ptr += BTREE_KEY_CNT*sizeof(long long);
	
	write_page(db,page,&node);
	
		//-----------------------------------------------------------------
	std::cout <<"WWWWW"<<"page "<<page<<"nKeys "<<nKeys<<"leaf "<<leaf<<"chld[0]"<<chld[0] <<"vals[0]: "<<vals[0]<<"vals[1]: "<<vals[1]<<"vals[2]: "<<vals[2]<<"\n";
	//-----------------------------------------------------------------

	delete[] node.data;

	return SUCC;
}

unsigned int BTreeNode::size()
{
	return struct_size;
}

void BTreeNode::print_node()
{
	std::cout <<"page: "<<page<<"\n";
	std::cout <<"nKeys: "<<nKeys<<"\n";
	std::cout <<"is leaf: "<<leaf<<"\n";
	for(int i = 0;i < BTREE_KEY_CNT;i++)
	{
		std::cout << "key "<<i<<": ";
		for(int j = 0;j < BTREE_KEY_LEN;j++)
		{	
			std::cout << keys[i*BTREE_KEY_LEN + j];
		}
		std::cout << "\n";
	}
	for(int i = 0;i < BTREE_CHLD_CNT;i++)
	{
		std::cout << "child "<<i<<": "<< chld[i]<<"\n";
	}
	for(int i = 0;i < BTREE_CHLD_CNT;i++)
	{
		std::cout << "vals "<<i<<": "<< vals[i]<<"\n";
	}
	std::cout<<"\n\n";
}
//------------------------------------Внешние функции------------------------------------------
void print_tree(BTreeNode* head,struct DB* db,int n)
{
	if(n > MAX_TREE_LEVEL)return;
	head->print_node();
	if(head->leaf == 1)return;

	BTreeNode node;

	if(head->nKeys >= 0 && head->nKeys+1 <= BTREE_CHLD_CNT)
	{
		for(int i = 0;i < head->nKeys+1;i++)
		{
			if(head->chld[i] != -1)
			{
				if(node.read_from_file(head->chld[i],db) == SUCC)
				{
					print_tree(&node,db,n+1);
				}
			}
			else std::cout <<"Empty child\n";
		}
	}
 	return;
}


int keys_compare(BTreeNode* node,int key_i,int key_j)
{
	if(key_i < BTREE_KEY_CNT && key_j < BTREE_KEY_CNT && node->keys != NULL)
		return memcmp((node->keys + key_i*BTREE_KEY_LEN),(node->keys + key_j*BTREE_KEY_LEN),BTREE_KEY_LEN);
	else return 0;
}
int keys_compare(BTreeNode* node,char* key,int key_i)
{
	if(node->keys != NULL && key != NULL)
		return memcmp(key,(node->keys + key_i*BTREE_KEY_LEN),BTREE_KEY_LEN);
	else return 0;
}

void keys_copy(char* key_dest,char* key_source)
{
	memcpy(key_dest,key_source,BTREE_KEY_LEN);
}
void keys_copy(BTreeNode* node,int key_i,int key_j)
{
	memcpy((node->keys + key_i*BTREE_KEY_LEN),(node->keys + key_j*BTREE_KEY_LEN),BTREE_KEY_LEN);
}

void keys_copy(BTreeNode* node,int key_i,char* key)
{
	memcpy((node->keys + key_i*BTREE_KEY_LEN),key,BTREE_KEY_LEN);
}

void keys_copy(BTreeNode* node1,int key_i,BTreeNode* node2,int key_j)
{
	memcpy((node1->keys + key_i*BTREE_KEY_LEN),(node2->keys + key_j*BTREE_KEY_LEN),BTREE_KEY_LEN);
}


int disk_read_node(const struct DB* db,long long page,BTreeNode* result)
{
	return result->read_from_file(page,db);
}

int disk_write_node(const struct DB* db,long long page,BTreeNode* source)
{
	return source->write_to_file(page,db);
}


//--------------------------Реализация функций структуры B-дерева---------------------------------------------
int search_key(BTreeNode* head,char* key,const struct DB* db,BTreeNode* result)
{
	unsigned int i = 0;
	while(i < head->nKeys && keys_compare(head,key,i) > 0)i++;
	if(i < head->nKeys && keys_compare(head,key,i) == 0)
	{
		*result = *head; 
		return i;
	} 
	if(head->leaf)return -1;
	else 
	{
		disk_read_node(db,head->chld[i],result);
		search_key(result,key,db,result); 
	}
}



int insert_key_test(BTreeNode* head,char* key,const long long data_page,const struct DB* db)
{
	
	if(head->nKeys > BTREE_KEY_CNT)
	{
		std::cout<<"There is too many keys: "<<head->nKeys<<" on page: "<<head->page<<"\n";
	}

	BTreeNode temp;
	if(head->nKeys == BTREE_KEY_CNT)
	{
		int i = 0;
		
			if(head->chld[i] == -1)
			{
				db->db_all->db_alloc(head->chld[i]);
				head->leaf = 0;
				keys_copy(&temp,i,key);
				temp.nKeys++;
				temp.page = head->chld[i];
				temp.vals[i] = data_page;
				head->write_to_file(head->page,db);
				temp.write_to_file(temp.page,db);
				return SUCC;
			}

		temp.read_from_file(head->chld[i],db);
		return insert_key_test(&temp,key,data_page,db);
	}		
	else {
		keys_copy(head,head->nKeys,key);
     		head->vals[head->nKeys] = data_page;
		head->nKeys++;
		head->write_to_file(head->page,db);
		}	
}

int insert_key(BTreeNode* head,char* key,const long long data_page,const struct DB* db)
{
	std::cout << "Inserting key...\n";
	if(head->nKeys == BTREE_KEY_CNT)
	{
		BTreeNode z;
		long long head_page = 0;
		db->db_all->db_alloc(z.page);
		head_page = head->page;
		head->page = z.page;
		head->write_to_file(head->page,db);
		z.page = head_page;
		z.leaf = 0;
		z.nKeys = 0;
		z.chld[0] = head->page;
		z.write_to_file(z.page,db);
		*(head) = z;
		split_chld(head,0,db);
		return insert_nonefull(head,key,data_page,db);
	}
	return insert_nonefull(head,key,data_page,db);
}

int insert_nonefull(BTreeNode* head,char* key,const long long data_page,const struct DB* db)
{
	std::cout << "Insert to none full element...\n";
	int i = head->nKeys - 1;
	if(head->leaf)
	{
		std::cout <<"The element is leaf...\n";
		while(keys_compare(head,key,i) < 0 && i >= 0)i--;
		i = i + 1;
		for(int j = head->nKeys-1;j > i;j--)
		{
			keys_copy(head,j,j-1);
			head->vals[j] = head->vals[j-1];
		}
		for(int j = head->nKeys;j > i;j--)
		{
			head->chld[i] = head->chld[i-1];
		}
		keys_copy(head,i,key);
		head->vals[i] = data_page;
		head->nKeys++;
		head->write_to_file(head->page,db);

		return SUCC;
	}
	else
	{
		std::cout <<"The element is not leaf...\n";
		while(keys_compare(head,key,i) < 0 && i >= 0)i--;
		i = i+1;
		BTreeNode ci;
		ci.read_from_file(head->chld[i],db);
		if(ci.nKeys == BTREE_KEY_CNT)
		{
			split_chld(head,i,db);
			if(keys_compare(head,key,i) > 0) i = i+1; 
			ci.read_from_file(head->chld[i],db);
		}
		return insert_nonefull(&ci,key,data_page,db);
	}
}

int split_chld(BTreeNode* head, int k,const struct DB* db)
{ 
	std::cout <<"Splitting childs...\n";
	BTreeNode x,y;
	int res = SUCC;
	int middle = BTREE_KEY_CNT/2;
	
	if(head->chld[k] == -1)
	{
		std::cout <<"Child "<<k<<" is empty!\n";
		return FAIL;
	}
	res = x.read_from_file(head->chld[k],db);
	db->db_all->db_alloc(y.page);
	y.leaf = x.leaf;

	if(!x.leaf)
	{
		for(int i = 0;i <= middle;i++)
		{
			y.chld[i] = x.chld[i+middle];
		}
	}
	for(int i = 0;i < middle;i++)
	{
		keys_copy(&y,i,&x,i+middle+1);
		y.vals[i] = x.vals[i+middle+1];
	}
	x.nKeys =  middle;
	y.nKeys =  middle;
	
	for(int i = head->nKeys;i >= k;i--) 
	{
		head->chld[i+1] = head->chld[i];
	}
	for(int i = head->nKeys-1; i >= k;i--)
	{
		keys_copy(head,i+1,i);
	}
	head->chld[k] = y.page;

	keys_copy(head,k,&x,middle);
	head->nKeys++;

	res &= head->write_to_file(head->page,db);
	res &= y.write_to_file(y.page,db);
	res &= x.write_to_file(x.page,db);

	return res;
}



