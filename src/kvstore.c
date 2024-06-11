#include "../include/kvstore.h"

#include <string.h>
#include<stdlib.h>

extern const char *command[] = 
{
	"SET", "GET", "DEL", "MOD", "EXIST",
};
//内存分配
void *kvs_malloc(size_t size)
{
	return malloc(size);
}
//内存释放
void kvs_free(void *ptr)
{
	return free(ptr);
}

//根据空格将输入字符串 msg 拆分为多个子字符串，并存储在 tokens 数组中。返回令牌的数量
int kvs_split_token(char *msg,char *tokens[])
{
	if(msg == NULL || tokens == NULL)
		return -1;
	
	int index = 0;
	char* rest = msg;
	char* token;


    while ((token = strtok_r(rest, " ", &rest))) {
        tokens[index++] = token;
    }


	return index;

}

//根据解析出的命令和参数执行相应的操作（SET, GET, DEL, MOD, EXIST），并返回结果字符串
//根据编译时的宏定义，选择使用不同的数据结构（数组、哈希表、跳表）执行操作
int kvs_filter_protocol(char **tokens, int count, char *response) 
{
	if (tokens[0] == NULL || count == 0 || response == NULL) return -1;

	int cmd = KVS_CMD_START;
	for (cmd = KVS_CMD_START;cmd < KVS_CMD_COUNT;cmd ++) {
		if (strcmp(tokens[0], command[cmd]) == 0) {
			break;
		} 
	}
	int length = 0;
	int ret = 0;
	char *key = tokens[1];
	char *value = tokens[2];
	switch(cmd) {
#if ENABLE_ARRAY
	case KVS_CMD_SET:
		ret = kvs_array_set(&global_array ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "EXIST\r\n");
		} 
		
		break;
	case KVS_CMD_GET: {
		char *result = kvs_array_get(&global_array, key);
		if (result == NULL) {
			length = sprintf(response, "NO EXIST\r\n");
		} else {
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_DEL:
		ret = kvs_array_del(&global_array ,key);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_MOD:
		ret = kvs_array_mod(&global_array ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_EXIST:
		ret = kvs_array_exist(&global_array ,key);
		if (ret == 0) {
			length = sprintf(response, "EXIST\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
#endif

#if ENABLE_HASH
	case KVS_CMD_SET:
		ret = kvs_hash_set(&global_hash ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "EXIST\r\n");
		} 
		
		break;
	case KVS_CMD_GET: {
		char *result = kvs_hash_get(&global_hash, key);
		if (result == NULL) {
			length = sprintf(response, "NO EXIST\r\n");
		} else {
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_DEL:
		ret = kvs_hash_del(&global_hash ,key);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_MOD:
		ret = kvs_hash_mod(&global_hash ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_EXIST:
		ret = kvs_hash_exist(&global_hash ,key);
		if (ret == 0) {
			length = sprintf(response, "EXIST\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
#endif

#if ENABLE_SKIPTABLE
	case KVS_CMD_SET:
		ret = kvs_skip_set(&global_skiptable ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "EXIST\r\n");
		} 
		
		break;
	case KVS_CMD_GET: {
		char *result = kvs_skip_get(&global_skiptable, key);
		if (result == NULL) {
			length = sprintf(response, "NO EXIST\r\n");
		} else {
			length = sprintf(response, "%s\r\n", result);
		}
		break;
	}
	case KVS_CMD_DEL:
		ret = kvs_skip_del(&global_skiptable ,key);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_MOD:
		ret = kvs_skip_mod(&global_skiptable ,key, value);
		if (ret < 0) {
			length = sprintf(response, "ERROR\r\n");
 		} else if (ret == 0) {
			length = sprintf(response, "OK\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
	case KVS_CMD_EXIST:
		ret = kvs_skip_exist(&global_skiptable ,key);
		if (ret == 0) {
			length = sprintf(response, "EXIST\r\n");
		} else {
			length = sprintf(response, "NO EXIST\r\n");
		}
		break;
#endif

	}
	return length;
}


//键值存储协议的接口函数，接收输入消息并处理命令，返回响应结果
int kvs_protocol(char *msg, int length, char *response) {  //
	
// SET Key Value
// GET Key
// DEL Key
	if (msg == NULL || length <= 0 || response == NULL) return -1;

	//printf("recv %d : %s\n", length, msg);

	char *tokens[KVS_MAX_TOKENS] = {0};

	int count = kvs_split_token(msg, tokens);
	if (count == -1) return -1;

	//memcpy(response, msg, length);

	return kvs_filter_protocol(tokens, count, response);
}



//一个简单的示例处理函数，将输入消息复制到响应中
int kv_store(char *msg, int length, char *response)
{
    sprintf(response,msg,length);
    return length;
}

//销毁键值存储结构，释放资源
void dest_(void) {
#if ENABLE_ARRAY
	kvs_array_destory(&global_array);
kvs_hash
#elif ENABLE_RBTREE
	kvs_rbtree_destory(&global_rbtree);

#elif ENABLE_HASH
	kvs_hash_destory(&global_hash);
#elif ENABLE_SKIPTABLE
	kvs_skip_destory(&global_skiptable);
#endif

}

//初始化键值存储结构
int init_()
{	
#if ENABLE_ARRAY
	memset(&global_array, 0, sizeof(kvs_array));
	kvs_array_create(&global_array);
#elif ENABLE_HASH	
	memset(&global_hash, 0, sizeof(kvs_hashtable));
	kvs_hash_create(&global_hash);
#elif ENABLE_SKIPTABLE
	memset(&global_skiptable, 0, sizeof(kvs_skiptable));
	kvs_skip_create(&global_skiptable);
#endif
}

//初始化键值存储结构，启动服务器，并在结束时销毁资源。
//根据编译时的宏定义，选择使用不同的网络模型（Reactor、协程、Proactor）启动服务器
int main()
{
	init_();
    int port = 9998;

#if (NETWORK_SELECT == NETWORK_REACTOR)
	reactor_start(port, kvs_protocol);  //
#elif (NETWORK_SELECT == NETWORK_NTYCO)
	coroutine_start(port, kvs_protocol);
#elif (NETWORK_SELECT == NETWORK_PROACTOR)
	proactor_start(port, kvs_protocol);
#endif
	dest_();
}
