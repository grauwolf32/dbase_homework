#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include "mydb.h"
#include "debug.h"

#define BTREE_KEY_CNT 11
#define BTREE_KEY_LEN 128
#define BTREE_VAL_LEN 4092
#define BTREE_CHLD_CNT (BTREE_KEY_CNT+1)

#define MAX_TREE_LEVEL 1000

//Переделать под структуру так как как классы плохо проецируются на память (!!!!) Или добавить метод write и read
//Точно надо написать read и write методы т.к. хз, как будут писаться динамические массивы

class BTreeNode {
 public:
	~BTreeNode();
	BTreeNode();
	BTreeNode(const BTreeNode& rnode);
	BTreeNode& operator=(const BTreeNode& rnode);

	int	read_from_file(long long page,const struct DB* db); //Нельзя так просто, нужно организовать ноды для работы с chunk' ом
	int	write_to_file(long long page,const struct DB* db); 
	unsigned int size();
	void print_node();
	
	long long page;
	long long nKeys;
	int	 leaf;
	char 	*keys;	    //Хранит значение ключей
	long long *chld;
	long long *vals;//Указатель на страницу, в которой храняться данные
 private:
	unsigned int struct_size; //Хранит размер, который занимает дельная информация в классе
	
};

int search_key(BTreeNode* head,char* key,const struct DB* db,BTreeNode* result); //Возвращает номер ключа
int delete_key(BTreeNode* head,char* key,const struct DB* db);
int split_chld(BTreeNode* head, int k,const struct DB* db);

int insert_key(BTreeNode* head,char* key,const long long data_page,const struct DB* db);
int insert_nonefull(BTreeNode* head,char* key,const long long data_page,const struct DB* db);

int keys_compare(BTreeNode* node,int key_i,int key_j);
int keys_compare(BTreeNode* node,char* key,int key_i);

void keys_copy(char* key_dest,char* key_source);  
void keys_copy(BTreeNode* node,int key_i,int key_j);
void keys_copy(BTreeNode* node,int key_i,char* key);
void keys_copy(BTreeNode* node1,int key_i,BTreeNode* node2,int key_j);
void erase_i_key(BTreeNode* head,int i,int shift,const struct DB* db);

void print_tree(BTreeNode* head,struct DB* db,int n);
int disk_read_node(const struct DB* db,long long page,BTreeNode* result);
int disk_write_node(const struct DB* db,long long page,BTreeNode* result);

#endif
