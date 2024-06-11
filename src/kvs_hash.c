#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "../include/kvstore.h"

#if ENABLE_HASH	//如果使用哈希表

kvs_hashtable global_hash;//定义全局哈希表
//#define MAX_TABLE_SIZE	1024 头文件中定义哈希表最大的大小

//计算哈希索引
int hash_index_compute(char *key,int size)
{
    if(!key) return -1;//键空
    int sum = 0;//初始化哈希值
    int i = 0;//初始化索引
    while(key[i] != 0)//遍历键的每个字符
    {
        sum += key[i];//累加字符的ASCII值
        i++;
    }
    return sum % size;//返回哈希值的模结果
}

//创建一个新的哈希节点
kvs_hashnode* hash_create_node(char *key,char *value)
{
    kvs_hashnode *node = (kvs_hashnode*)kvs_malloc(sizeof(kvs_hashnode));// 分配哈希节点内存
    if(!node) return NULL;//分配失败
    char* keycopy = kvs_malloc(strlen(key)+1);// 分配键的内存
    if (keycopy == NULL) return NULL;//分配失败
	memset(keycopy, 0, strlen(key) + 1);//清空内存
	strncpy(keycopy, key, strlen(key));//复制键
    node->key = keycopy;//设置节点的键
    char* valuecopy = kvs_malloc(strlen(value)+1);//分配值内存
    if (valuecopy == NULL) {return NULL; kvs_free(keycopy);}//分配失败
	memset(valuecopy, 0, strlen(value) + 1);
	strncpy(valuecopy, value, strlen(value));//复制值
    node->value = valuecopy;// 设置节点的值
    node->next = NULL;//设置节点的下一个指针为NULL
	return node;
}

//创建并初始化一个新的哈希表
int kvs_hash_create(kvs_hashtable *inst)
{
    if(!inst) return -1;//实例空
    inst->nodes = (kvs_hashnode**)kvs_malloc(sizeof(kvs_hashnode*) * MAX_TABLE_SIZE);//分配哈希表节点数组内存
    if (!inst->nodes) return -1;//分配失败
    inst->max_slots = MAX_TABLE_SIZE;//哈希表最大槽数
	inst->count = 0; // 初始化键值对计数为0
	return 0;
}

//销毁一个哈希表并释放内存
void kvs_hash_destory(kvs_hashtable *inst)
{
    if (!inst) return;//实例空
    for(int i = 0; i < MAX_TABLE_SIZE ;i++)//遍历哈希表每个槽
    {
        kvs_hashnode *node = inst->nodes[i];// 获取槽中的第一个节点
        while(node != NULL)//遍历链表 更新链表节点
        {
            kvs_hashnode *tmp = node;
			node = node->next;
			inst->nodes[i] = node;			
			kvs_free(tmp);
        }
    }
    kvs_free(inst->nodes);// 释放哈希表节点数组内存
}

//在哈希表中设置一个新的键值对
int kvs_hash_set(kvs_hashtable *table,char *key,char *value)
{
    if (!table || !key || !value) return -1;
    int index = hash_index_compute(key,MAX_TABLE_SIZE);// 计算哈希索引
    kvs_hashnode *flag = table->nodes[index];// 获取索引对应的链表头节点
    while(flag != NULL)
    {
        if(strcmp(key,flag->key) == 0)
        return 1; //重复
        flag = flag->next;
    }
    kvs_hashnode *new_node = hash_create_node(key, value);// 创建新的哈希节点
    new_node->next = table->nodes[index];
    table->nodes[index] = new_node; 
    table->count++;
    return 0;
}

//从哈希表中获取一个键的值
char *kvs_hash_get(kvs_hashtable *table,char *key)
{
    char *res = NULL;// 初始化返回值
    if (!table || !key) return res;//表或键为空   
    int index = hash_index_compute(key,MAX_TABLE_SIZE);
    kvs_hashnode *flag = table->nodes[index];
    while(flag != NULL)
    {
        if(strcmp(key,flag->key) == 0)//找到匹配节点
        {
            return flag->value;
        }

        flag = flag->next;
    }
    return res;//返回NULL 未找到
}

//修改哈希表中一个键的值
int kvs_hash_mod(kvs_hashtable *table, char *key, char *value)
{
    if (!table || !key || !value) return -1;

    int index = hash_index_compute(key,MAX_TABLE_SIZE);

    kvs_hashnode *flag = table->nodes[index];

    while(flag != NULL)
    {
        if (strcmp(flag->key, key) == 0) //找到匹配节点
        {
			break;
		}
        flag = flag->next;
    }

    if(flag == NULL)//未找到
    {
        return 1;
    }

    kvs_free(flag->value);
    char *valuecopy = kvs_malloc(strlen(value) + 1);
	if (valuecopy == NULL) return -2;
	memset(valuecopy, 0, strlen(value) + 1);
	strncpy(valuecopy, value, strlen(value));
	flag->value = valuecopy;
	return 0;   
}

//获取哈希表中键值对的数量
int kvs_hash_count(kvs_hashtable *table) 
{
	return table->count;
}

//检查哈希表中是否存在一个键
int kvs_hash_exist(kvs_hashtable *table, char *key) 
{
	char *value = kvs_hash_get(table, key);//获取键的值
	if (!value) return 1;//不存在
	return 0;//存在
}

//删除哈希表中的一个键值对
int kvs_hash_del(kvs_hashtable *table, char *key) {
	if (!table || !key) return -2;
	int idx = hash_index_compute(key, MAX_TABLE_SIZE);
	kvs_hashnode *head = table->nodes[idx];	
    if(head == NULL)  return 1;  //如果链表头为空，键不存在   
    if (strcmp(head->key, key) == 0) // 如果链表头节点的键匹配
    {
		kvs_hashnode *tmp = head->next;
		table->nodes[idx] = tmp;		
        kvs_free(head->key);
        kvs_free(head->value);
		kvs_free(head);
		table->count --;	
		return 0;
	}
	
	while (head->next != NULL) 
    {
		if (strcmp(head->next->key, key) == 0) break;//找到键匹配的节点      
        head = head->next; 
	}
	if (head->next == NULL) return 1; //未找到
	kvs_hashnode *tmp = head->next;
	head->next = tmp->next;
	kvs_free(tmp->key);
	kvs_free(tmp->value);
	kvs_free(tmp);	
	table->count --;
	return 0;
}

#endif