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
	chld = new unsigned long[BTREE_CHLD_CNT];
	keys = new char[BTREE_KEY_CNT*BTREE_KEY_LEN];
	vals = new unsigned long[BTREE_CHLD_CNT];
	
	memset(chld,0,BTREE_CHLD_CNT*sizeof(char));
	memset(keys,0,BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	memset(vals,0,BTREE_CHLD_CNT);
	
	struct_size = 0;
	struct_size += sizeof(page);
	struct_size += sizeof(nKeys);
	struct_size += sizeof(leaf);
	struct_size += BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char);
	struct_size += BTREE_CHLD_CNT*sizeof(unsigned long);
	struct_size += BTREE_KEY_CNT*sizeof(unsigned long);
	

	page = 0;
	nKeys = 0;
	leaf = 1;
}


BTreeNode::~BTreeNode()
{
	if(chld != NULL)delete chld;
	if(keys != NULL)delete keys;
	if(vals != NULL)delete vals;
}

BTreeNode& BTreeNode::operator=(const BTreeNode& rnode)
{
	if(&rnode == this)return *this;
	
	if(chld != NULL) delete chld;
	if(keys != NULL) delete keys;
	if(vals != NULL) delete vals;
	
	if(rnode.keys != NULL && rnode.chld != NULL && rnode.vals != NULL)
	{	
		chld = new unsigned long[BTREE_CHLD_CNT];
		keys = new char [BTREE_KEY_CNT*BTREE_KEY_LEN];
		vals = new unsigned long[BTREE_CHLD_CNT];
	
		for(unsigned long i = 0; i < BTREE_CHLD_CNT;i++) chld[i] = rnode.chld[i];
		for(unsigned long i = 0; i < BTREE_KEY_CNT*BTREE_KEY_LEN;i++) keys[i] = rnode.keys[i];
		for(unsigned long i = 0; i < BTREE_CHLD_CNT;i++) vals[i] = rnode.vals[i];
	
	}
	
	page = rnode.page;
	nKeys = rnode.nKeys;
	leaf = rnode.leaf;
	
	return *this;
}

int 	 BTreeNode::read_from_file(unsigned long page,const struct DB* db)
{
	struct DBT node;
	unsigned int ptr = 0;
	unsigned long read_size = 0;
	
	node.data = new char[struct_size];
	node.size = struct_size;

	read_page(db,page,&node);

	memcpy(&page,node.data,sizeof(page));	
	ptr += 	sizeof(page);
	memcpy(&nKeys,(node.data + ptr),sizeof(nKeys));
	ptr += sizeof(nKeys);
	memcpy(&leaf,(node.data + ptr),sizeof(leaf));
	ptr += sizeof(leaf);
	memcpy(keys,(node.data + ptr),BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	ptr += BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char);
	memcpy(chld,(node.data + ptr),BTREE_CHLD_CNT*sizeof(unsigned long));
	ptr += BTREE_CHLD_CNT*sizeof(unsigned long);
	memcpy(vals,(node.data + ptr),BTREE_KEY_CNT*sizeof(unsigned long));
	ptr += BTREE_KEY_CNT*sizeof(unsigned long);
	
	delete node.data;
	return SUCC;
}
int	 BTreeNode::write_to_file(unsigned long page,const struct DB* db)
{
	struct DBT node;
	unsigned int ptr = 0;
	unsigned long mem_read = 0;
	
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
	memcpy((node.data + ptr),chld,BTREE_CHLD_CNT*sizeof(unsigned long));
	ptr += BTREE_CHLD_CNT*sizeof(unsigned long);
	memcpy((node.data + ptr),vals,BTREE_KEY_CNT*sizeof(unsigned long));
	ptr += BTREE_KEY_CNT*sizeof(unsigned long);
	
	write_page(db,page,&node);

	delete node.data;

	return SUCC;
}

unsigned int BTreeNode::size()
{
	return struct_size;
}
//------------------------------------Внешние функции------------------------------------------
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


int disk_read_node(const struct DB* db,unsigned long page,BTreeNode* result)
{
	return result->read_from_file(page,db);
}

int disk_write_node(const struct DB* db,unsigned long page,BTreeNode* source)
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

int split_child(BTreeNode* x,unsigned int i,const struct DB* db)
{
	BTreeNode z,y;
	int c = (BTREE_KEY_CNT/2);

	disk_read_node(db,x->chld[i],&y);

	z.leaf = y.leaf;
	db->db_all->db_alloc(z.page);
	z.nKeys = c;

	for(int j = 0;j <= c - 1;j++) //Error, may be just j < c
	{
		keys_copy(&z,j,&y,j+c+1);
		z.vals[j] = y.vals[j+c+1];
	}

	if(!y.leaf)
	{
		for(int j = 0;j <= c;j++)
			z.chld[j] = y.chld[j+c+1];
	}

	y.nKeys = c;
	for(int j = x->nKeys;j > i;j--)
	{
		x->chld[j+1] = x->chld[j];
	} 
	x->chld[i] = z.page;

	for(int j = x->nKeys; j > i;j--)
	{
		keys_copy(x,j+1,j);
		x->vals[j+1] = x->vals[j];
	}
	x->nKeys++;

	int res = 1;
	res &= disk_write_node(db,y.page,&y);
	res &= disk_write_node(db,z.page,&z);
	res &= disk_write_node(db,x->page,x);	
	
	return res;
}

int insert_key(BTreeNode* head,char* key,const unsigned long data_page,const struct DB* db)
{
	BTreeNode* root = head;
	if(root->nKeys == BTREE_KEY_CNT)
	{
		BTreeNode s;
		db->db_all->db_alloc(s.page);
		*head = s;
		s.leaf = 0;
		s.nKeys = 0;
		s.chld[0] = root->page;
		split_child(&s,1,db); 
		return insert_nonfull(&s,key,data_page,db);
	}
	else {
	return insert_nonfull(root,key,data_page,db);
	}
}

int insert_nonfull(BTreeNode* x,char* key,const unsigned long data_page,const struct DB* db)
{
	int i = x->nKeys-1;
	if(x->leaf)
	{
		while(i >= 0 && keys_compare(x,key,i) < 0)
		{
			keys_copy(x,i+1,i);
			x->vals[i+1] = x->vals[i];
			i--;
		}
		i = i + 1;
		keys_copy(x,i,key);
		x->vals[i] = data_page;

		x->nKeys++;
		disk_write_node(db,x->page,x);
		return i;
	}
	else
	{
		while(i >= 0 && keys_compare(x,key,i) < 0) i--;
		i = i + 1;
		BTreeNode c_i;
		disk_read_node(db,x->chld[i],&c_i);
		if(c_i.nKeys == BTREE_KEY_CNT)
		{
			split_child(x,i,db);
			if(keys_compare(x,key,i) > 0)i++;
		}
		return insert_nonfull(&c_i,key,data_page,db);			
	}
}

