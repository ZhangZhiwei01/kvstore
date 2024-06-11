#pragma once

#include <stdio.h>
#include <string.h>

#define NETWORK_REACTOR 	0
#define NETWORK_PROACTOR	1
#define NETWORK_NTYCO		2
#define NETWORK_SELECT		NETWORK_REACTOR
#define KVS_MAX_TOKENS		128
// 启用不同的数据结构
#define ENABLE_ARRAY		0
#define ENABLE_RBTREE		0
#define ENABLE_HASH			0
#define ENABLE_SKIPTABLE    1



// KVS命令枚举
enum {
	KVS_CMD_START = 0,
	// array
	KVS_CMD_SET = KVS_CMD_START,
	KVS_CMD_GET,
	KVS_CMD_DEL,
	KVS_CMD_MOD,
	KVS_CMD_EXIST,
	
	KVS_CMD_COUNT,
};

// 消息处理函数指针类型定义
typedef int (*msg_handler)(char *msg, int length, char *response);


// 内存分配和释放函数
void* kvs_malloc(size_t size);
void kvs_free(void* ptr);

// 网络模型启动函数声明
extern int reactor_start(unsigned short port, msg_handler handler);
extern int proactor_start(unsigned short port, msg_handler handler);
extern int coroutine_start(unsigned short port, msg_handler handler);



#if ENABLE_ARRAY

// 数组节点结构体
typedef struct kvs_array_node
{
    char *key;
    char *value;
}kvs_array_node;

// 数组结构体
typedef struct kvs_array
{
    kvs_array_node *table;
    int total;
    
}kvs_array;

extern kvs_array global_array;// 全局数组实例声明
// 数组相关操作函数声明
int kvs_array_create(kvs_array *inst);
void kvs_array_destory(kvs_array *inst);
int kvs_array_set(kvs_array *inst, char *key, char *value);
char* kvs_array_get(kvs_array *inst, char *key);
int kvs_array_del(kvs_array *inst, char *key);
int kvs_array_mod(kvs_array *inst, char *key, char *value);
int kvs_array_exist(kvs_array *inst, char *key);
#endif

#if ENABLE_HASH	


#define MAX_TABLE_SIZE	1024

// 哈希节点结构体
typedef struct kvs_hashnode
{
	char *key;
	char *value;

	struct kvs_hashnode *next;
}kvs_hashnode;
// 哈希表结构体
typedef struct kvs_hashtable
{
	kvs_hashnode **nodes;
	int max_slots;
	int count;

}kvs_hashtable;
// 全局哈希表实例声明
extern kvs_hashtable global_hash;

// 哈希表相关操作函数声明
int kvs_hash_create(kvs_hashtable *table);
void kvs_hash_destory(kvs_hashtable *table);
int kvs_hash_set(kvs_hashtable *table, char *key, char *value);
char * kvs_hash_get(kvs_hashtable *table, char *key);
int kvs_hash_mod(kvs_hashtable *table, char *key, char *value);
int kvs_hash_del(kvs_hashtable *table, char *key);
int kvs_hash_exist(kvs_hashtable *table, char *key);
#endif

#if ENABLE_SKIPTABLE
// 跳表节点结构体
typedef struct kvs_skipnode
{
	char *key;
	char *value;
	struct kvs_skipnode **next;

}kvs_skipnode;
// 跳表结构体
typedef struct kvs_skiptable
{
	kvs_skipnode *head;
	int level;
}kvs_skiptable;
// 全局跳表实例声明
extern kvs_skiptable global_skiptable;
// 跳表相关操作函数声明
int kvs_skip_create(kvs_skiptable *table);
void kvs_skip_destory(kvs_skiptable *table);
int kvs_skip_set(kvs_skiptable *table, char *key, char *value);
char * kvs_skip_get(kvs_skiptable *table, char *key);
int kvs_skip_mod(kvs_skiptable *table, char *key, char *value);
int kvs_skip_del(kvs_skiptable *table, char *key);
int kvs_skip_exist(kvs_skiptable *table, char *key);

#endif

// main -> kvstore.c reactorstart(port,func)
//reactor.c b.c c.c


