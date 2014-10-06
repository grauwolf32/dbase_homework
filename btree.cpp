#include <string.h>
#include "btree.h"
#include "mydb.h"
#include "allocator.h"

BTreeNode::BTreeNode()
{
	chld = new unsigned long[BTREE_CHLD_CNT];
	keys = new unsigned long[BTREE_KEY_CNT*BTREE_KEY_LEN];
	vals = new unsigned long[BTREE_CHLD_CNT];
	
	memset(chld,0,BTREE_CHLD_CNT*sizeof(char));
	memset(keys,0,BTREE_KEY_CNT*BTREE_KEY_LEN*sizeof(char));
	memset(vals,0,BTREE_CHLD_CNT);
	
	page = 0;
	parentPage = 0;
	nKeys = 0;
	leaf = 1;
}
BTreeNode::~BTreeNode()
{
	if(chld != NULL)delete[] chld;
	if(keys != NULL)delete[] keys;
	if(vals != NULL)delete[] vals;
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
		keys = new unsigned long[BTREE_KEY_CNT*BTREE_KEY_LEN];
		vals = new unsigned long[BTREE_CHLD_CNT];
	
		for(unsigned long i = 0; i < BTREE_CHLD_CNT;i++) chld[i] = rnode.chld[i];
		for(unsigned long i = 0; i < BTREE_KEY_CNT*BTREE_KEY_LEN;i++) keys[i] = rnode.keys[i];
		for(unsigned long i = 0; i < BTREE_CHLD_CNT;i++) vals[i] = rnode.vals[i];
	
	}
	
	page = rnode.page;
	parentPage = rnode.parentPage;
	nKeys = rnode.nKeys;
	leaf = rnode.leaf;
	
	return *this;
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

int disk_read_node(DB* db,unsigned long page,BTreeNode* result)
{
	if(fread_db(db->fd,result,page,sizeof(char),sizeof(BTreeNode)) != sizeof(BTreeNode)) return 0;
	return 1;
}

int disk_write_node(DB* db,unsigned long page,BTreeNode* source)
{
	if(fwrite_db(db->fd,source,page,sizeof(char),sizeof(BTreeNode)) != sizeof(BTreeNode)) return 0;
	return 1;
}

int search_key(BTreeNode* head,char* key,struct DB* db,BTreeNode* result)
{
	unsigned int i = 0;
	while(i < head->nKeys && keys_compare(head,key,i) > 0)i++;
	if(i < head->nKeys && keys_compare(head,key,i) == 0)
	{
		*result = *head; 
		return 1;
	} 
	if(head->leaf)return 0;
	else 
	{
		disk_read_node(db,head->chld[i],result);
		search_key(result,key,db,result); 
	}
}

int split_child(BTreeNode* x,unsigned int i,struct DB* db)
{
	BTreeNode z,y;
	y = *x;
	int c = (BTREE_KEY_CNT/2);
	z.leaf = y.leaf;
	db->db_all->db_alloc(z.page);
	z.nKeys = c - 1;

	for(int j = 0;j < c - 1;j++)
	{
		keys_copy(&z,j,j+c);
	}

	if(!y.leaf)
	{
		for(int j = 0;j < c;j++)
			z.chld[j] = y.chld[j+c];
	}

	y.nKeys = c - 1;
	for(int j = x->nKeys-1;j > i+1;j--)
	{
		x->chld[j+1] = x->chld[j];
	} 
	x->chld[i+1] = z.page;

	for(int j = x->nKeys; j > i;j--)
	{
		keys_copy(x,j+1,j);
	}
	x->nKeys++;

	int res = 1;
	res &= disk_write_node(db,y.page,&y);
	res &= disk_write_node(db,z.page,&z);
	res &= disk_write_node(db,x->page,x);	
	
	return res;
}

int insert_key(BTreeNode* head,char* key,struct DB* db)
{
	BTreeNode* root = head;
	if(root->nKeys == (2*BTREE_KEY_CNT-1))
	{
		BTreeNode s;
		db->db_all->db_alloc(s.page);
		*head = s;
		s.leaf = 0;
		s.nKeys = 0;
		s.keys[0] = root->page;
		split_child(&s,1,db); 
		insert_nonfull(&s,key,db);
	}
	else {
	insert_nonfull(root,key,db);
	}
}

int insert_nonfull(BTreeNode* x,char* key,struct DB* db)
{
	unsigned long i = x->nKeys;
	if(x->leaf)
	{
		while(i >= 0 && keys_compare(x,key,i) < 0)
		{
			keys_copy(x,i+1,i);
			i--;
		}
		keys_copy(x,i+1,key);
		x->nKeys++;
		return disk_write_node(db,x->page,x);
	}
	else
	{
		while(i >= 0 && keys_compare(x,key,i) < 0) i--;
		i = i + 1;
		BTreeNode c_i;
		disk_read_node(db,x->chld[i],&c_i);
		if(c_i.nKeys == (2*BTREE_KEY_CNT-1))
		{
			split_child(x,i,db);
			if(keys_compare(x,key,i) > 0)i++;
		}
		insert_nonfull(&c_i,key,db);			
	}
}

