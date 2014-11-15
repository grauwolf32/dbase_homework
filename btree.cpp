#include <string.h>
#include <iostream>
#include "btree.h"
#include "mydb.h"
#include "allocator.h"
#include "debug.h"

#define SUCC 1
#define FAIL 0

/*-------------------Members of class functions-------------------*/
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
	int res = SUCC;
	unsigned int ptr = 0;
	
	node.data = new char[db->config->chunk_size];
	node.size = db->config->chunk_size;
	memset(node.data,0,node.size);

	res = read_page(db,page,&node);
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
	return res;
}
int	 BTreeNode::write_to_file(long long page,const struct DB* db)
{
	struct DBT node;
	int res = SUCC;
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
	
	res = write_page(db,page,&node);
	
	delete[] node.data;

	return res;
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
/*-------------------Utility functions-------------------*/
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


/*------------------- B-tree function realization-------------------*/
int search_key(BTreeNode* head,char* key,const struct DB* db,BTreeNode* result)
{
	#ifdef _DEBUG_
		std::cout << "Searching key...\n";
	#endif

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

int insert_key(BTreeNode* head,char* key,const long long data_page,const struct DB* db)
{
	#ifdef _DEBUG_
		std::cout << "Inserting key...\n";
	#endif

	if(head->nKeys > BTREE_KEY_CNT)
	{
		std::cout<<"There is too many keys: "<<head->nKeys<<" on page: "<<head->page<<"...\n";
		return FAIL;
	}
	
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
	#ifdef _DEBUG_
		std::cout << "Inserting into none full node...\n";
	#endif

	int i = head->nKeys - 1;
	if(head->leaf)
	{
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
	#ifdef _DEBUG_
		std::cout << "Splitting child...\n";
	#endif

	BTreeNode x,y;
	int res = SUCC;
	int middle = BTREE_KEY_CNT/2;
	
	if(head->chld[k] == -1)
	{
		std::cout <<"Child "<< k <<" is empty!\n";
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

void erase_i_key(BTreeNode* head,int i,int shift,const struct DB* db)
{
	if(!head->leaf)
	{
		for(int j = i;j < head->nKeys-shift;j++)
		{
			head->chld[j+shift] = head->chld[j+shift+1];
		}
		head->chld[head->nKeys] = -1;
	}

	for(int j = i;i < head->nKeys-1;i++)
	{
		keys_copy(head,j,j+1);
		head->vals[j] = head->vals[j+1];
	}
	head->nKeys--;
	head->write_to_file(head->page,db);

	return;
}

int delete_key(BTreeNode* head,char* key,const struct DB* db)
{
	#ifdef _DEBUG_
		std::cout << "Deleting key...\n";
	#endif

	BTreeNode left,right,temp;
	int is_left = 0,is_right = 0,res = SUCC;
	int t = BTREE_KEY_CNT/2 + 1; 
	unsigned int i = 0;

	if(head->leaf)
	{	
		while(i < head->nKeys && keys_compare(head,key,i) > 0)i++;
		if(i < head->nKeys && keys_compare(head,key,i) == 0)
		{
			db->db_all->db_free(head->vals[i]); /* Free page in the database */
			erase_i_key(head,i,0,db);
			return SUCC;
		} 
		else return FAIL;
	}

	while(i < head->nKeys && keys_compare(head,key,i) > 0)i++;

	/* If key is in this leaf */
	if(i < head->nKeys && keys_compare(head,key,i) == 0) 
	{
		/* Reading adjoint nodes */
		if(head->chld[i] != -1)
		{
			res &= left.read_from_file(head->chld[i],db);
			if(res == SUCC)is_left = 1;	
		}
	
		if(is_left && left.nKeys >= t)
		{
			db->db_all->db_free(head->vals[i]);
			keys_copy(head,i,&left,left.nKeys-1); 
			head->vals[i] = left.vals[left.nKeys-1];

			res &= head->write_to_file(head->page,db);
			res &= delete_key(&left,left.keys+(left.nKeys-1)*BTREE_KEY_LEN,db); /* ?????????? */
			return res;
		}

		/* Reading adjoint nodes */
		if(i < head->nKeys)
		{
			if(head->chld[i+1] != -1)
			{
				res &= right.read_from_file(head->chld[i],db);
				if(res == SUCC)is_right = 1;
			}
		}

		if(is_right && right.nKeys >= t)
		{
			db->db_all->db_free(head->vals[i]);
			keys_copy(head,i,&right,0); /*Shuld i copy childs and delete head->vals[i]*/
			head->vals[i] = right.vals[0];

			res &= head->write_to_file(head->page,db);
			res &= delete_key(&right,right.keys,db); /* ??????????*/
			return res;
		}

		if(is_left && is_right) /*If left and right adjoint nodes exists, and both contain t-1 keys */
		{
			int ptr = 0;
			int key_pos = left.nKeys;

			keys_copy(&left,left.nKeys,head->keys+i*BTREE_KEY_LEN);
			left.vals[key_pos] = head->vals[i];
			/*Merging left and right pages into left */
			for(int j = left.nKeys+1;j < left.nKeys + right.nKeys+1;j++)
			{
				keys_copy(&left,j,&right,ptr);
				left.vals[j] = right.vals[ptr];
				left.nKeys++;
				ptr = ptr+1;
			}

			ptr = 0;
			for(int j = key_pos+1;j < left.nKeys+1;j++)
			{
				left.chld[j] = right.chld[ptr];
				ptr = ptr + 1;
			}
			/* Cleaning head */
			erase_i_key(head,i,1,db);

			/* Updating pages in data base */
			left.write_to_file(left.page,db);

			/*Deleting page from db*/
			db->db_all->db_free(right.page); 

			res &= delete_key(&left,left.keys + key_pos*BTREE_KEY_LEN,db);
			return res;
		}

		if(is_left) /* If there is only left subtree with t-1 keys */
		{
			int key_pos = left.nKeys;
			keys_copy(&left,left.nKeys,head,i);
			
			key_pos = left.nKeys;
			left.nKeys++;
			left.write_to_file(left.page,db);
			
			erase_i_key(head,i,1,db);  //Correct ? M.b. smthng lost here ? 

			res &= delete_key(&left,left.keys + key_pos*BTREE_KEY_LEN,db);
			return res;
		}

		if(is_right) /* If there is only right subtree with t-1 keys */
		{
			int key_pos = right.nKeys;
			keys_copy(&right,right.nKeys,head,i);
		
			key_pos = right.nKeys;
			right.nKeys++;
			right.write_to_file(right.page,db);
			
			erase_i_key(head,i,0,db);

			res &= delete_key(&right,right.keys + key_pos*BTREE_KEY_LEN,db);
			return res;
		}

		erase_i_key(head,i,0,db); /* If there no adjoining subtrees */
	}

	else /* If key is in the subleaf */
	{
		temp.read_from_file(head->chld[i],db);
		if(temp.nKeys >= t)
		{
			res &= delete_key(&temp,head->keys + i*BTREE_KEY_LEN,db);
			return res;
		}
		/* Reading adjoint nodes */
		if(i > 0)
		{
			if(head->chld[i-1] != -1)
			{
				res &= left.read_from_file(head->chld[i-1],db);
				if(res == SUCC)is_left = 1;	
			}	
		}
		if(i < head->nKeys)
		{
			if(head->chld[i+1] != -1)
			{
				res &= right.read_from_file(head->chld[i],db);
				if(res == SUCC)is_right = 1;
			}
		}

		if(is_left && left.nKeys >= t)
		{
			keys_copy(&temp,temp.nKeys,head,i);
			temp.vals[temp.nKeys] = head->vals[i];

			temp.nKeys++;

			keys_copy(head,i,&left,left.nKeys-1);
			return res;
		}

		if(is_right && right.nKeys >= t)
		{
			keys_copy(&temp,temp.nKeys,head,i);
			temp.vals[temp.nKeys] = head->vals[i];

			temp.nKeys++;

			keys_copy(head,i,&left,left.nKeys-1);
			return res;
		}
		
	
	return res;

	}
}


