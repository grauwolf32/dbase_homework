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

	int	read_from_file(unsigned long page,const struct DB* db); //Нельзя так просто, нужно организовать ноды для работы с chunk' ом
	int	write_to_file(unsigned long page,const struct DB* db); 
	unsigned int size();
	
	unsigned long page;
	unsigned long nKeys;
	int	 leaf;
	char *keys;	    //Хранит значение ключей
	unsigned long *chld;
	unsigned long *vals;//Указатель на страницу, в которой храняться данные
 private:
	unsigned int struct_size; //Хранит размер, который занимает дельная информация в классе
	
};

int search_key(BTreeNode* head,char* key,const struct DB* db,BTreeNode* result); //Возвращает номер ключа
int delete_key(BTreeNode* head,char* key,const struct DB* db);
int insert_key(BTreeNode* head,char* key,const unsigned long data_page,const struct DB* db); //Возвращает номер ключа
int insert_nonfull(BTreeNode* head,char* key,const unsigned long data_page,const struct DB* db); //Возвращает номер ключа
int split_child(BTreeNode* x,unsigned int i,const struct DB* db);

int keys_compare(BTreeNode* node,int key_i,int key_j);
int keys_compare(BTreeNode* node,char* key,int key_i);

void keys_copy(char* key_dest,char* key_source);  
void keys_copy(BTreeNode* node,int key_i,int key_j);
void keys_copy(BTreeNode* node,int key_i,char* key);

int disk_read_node(const struct DB* db,unsigned long page,BTreeNode* result);
int disk_write_node(const struct DB* db,unsigned long page,BTreeNode* result);

#endif
