#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include "mydb.h"

#define BTREE_KEY_CNT 3
#define BTREE_KEY_LEN 128
#define BTREE_VAL_LEN 4092
#define BTREE_CHLD_CNT (BTREE_KEY_CNT+1)

//Переделать под структуру так как как классы плохо проецируются на память (!!!!) Или добавить метод write и read
//Точно надо написать read и write методы т.к. хз, как будут писаться динамические массивы

class BTreeNode {
 public:
	~BTreeNode();
	BTreeNode();
	BTreeNode(const BTreeNode& rnode);
	BTreeNode& operator=(const BTreeNode& rnode);

	int	read_from_file(long int offset,struct DB* db); //Нельзя так просто, нужно организовать ноды для работы с chunk' ом
	int	write_to_file(long int offset,struct DB* db); 
	
	unsigned long page;
	unsigned long nKeys;
	int	 leaf;
	char *keys;	    //Хранит значение ключей
	unsigned long *chld;
	unsigned long *vals;//Указатель на страницу, в которой храняться данные
 private:
	unsigned int struct_size; //Хранит размер, который занимает дельная информация в классе
	
};

int search_key(BTreeNode* head,char* key,struct DB* db,BTreeNode* result);
int delete_key(BTreeNode* head,char* key,struct DB* db);
int insert_key(BTreeNode* head,char* key,struct DB* db);
int insert_nonfull(BTreeNode* head,char* key,struct DB* db);
int split_child(BTreeNode* x,unsigned int i,struct DB* db);

int keys_compare(BTreeNode* node,int key_i,int key_j);
int keys_compare(BTreeNode* node,char* key,int key_i);

void keys_copy(char* key_dest,char* key_source);  
void keys_copy(BTreeNode* node,int key_i,int key_j);
void keys_copy(BTreeNode* node,int key_i,char* key);

int disk_read_node(DB* db,unsigned long page,BTreeNode* result);
int disk_write_node(DB* db,unsigned long page,BTreeNode* result);

#endif
