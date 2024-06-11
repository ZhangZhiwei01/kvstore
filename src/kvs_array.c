#if ENABLE_ARRAY// 如果启用了数组存储
#include "../include/kvstore.h"

#define KVS_ARRAY_SIZE 512 //可以存储的最大键值对数量
//total指的是last元素 可以加快查找效率
kvs_array global_array = {0};//全局的键值存储实例

//创建并初始化一个键值存储实例
int kvs_array_create(kvs_array *inst)
{
    if(!inst) return -1;//实例为空 出错
    if (inst->table) //如果已分配
    {
		printf("table has alloc\n");
		return -1;
	}
    inst->table = kvs_malloc(KVS_ARRAY_SIZE * sizeof(kvs_array_node));//分配存储空间	
    if (!inst->table) //分配失败
    {
		return -1;
	}
    inst->total = 0;//总数初始化为0
	return 0;
}

//销毁一个键值存储实例，释放内存
void kvs_array_destory(kvs_array *inst) 
{
	if (!inst) return ;//实例空 出错
	if (inst->table) 
    {
		kvs_free(inst->table);//释放表内存
	}//c语言遵循 开闭原则 这里的inst不是在init申请的 所以不再destory释放
}

//在键值存储中添加一个新的键值对
int kvs_array_set(kvs_array*inst,char *key,char*value)
{
    if (inst == NULL || key == NULL || value == NULL) return -1;//实例或键或值为空 出错
	if (inst->total == KVS_ARRAY_SIZE) return -1;//表满了
    char *str = kvs_array_get(inst, key);//检查键是否已存在
	if (str) 
    {
		return 1; // 已经存在
	}
    char *kcopy = kvs_malloc(strlen(key)+1);//分配内存保存键的副本
    if (kcopy == NULL) return -2;//分配失败
	memset(kcopy, 0, strlen(key) + 1);//清空内存
	strncpy(kcopy, key, strlen(key));//复制键
    char *kvalue = kvs_malloc(strlen(value) + 1);//分配内存保存值的副本
	if (kvalue == NULL) return -2;//分配失败
	memset(kvalue, 0, strlen(value) + 1);//清空内存
	strncpy(kvalue, value, strlen(value));//复制值
    int i = 0;
    for( i = 0; i < inst->total ;i++)
    {
        if(inst->table[i].key == NULL)//找一个空位保存键和值
        {
            inst->table[i].key = kcopy;
            inst->table[i].value = kvalue;
            inst->total ++;
            return 0;
        }
    }
    if(!inst)//判空
    {
        printf("NULL\n");
    }
    if(i < KVS_ARRAY_SIZE && i == inst->total)//在表尾插入新键值对
    {
        inst->table[i].key = kcopy;
		inst->table[i].value = kvalue;
		inst->total ++;
    }
    return 0;
}

//删除一个键值对
int kvs_array_del(kvs_array *inst, char *key) 
{
	if (inst == NULL || key == NULL) return -1;//实例空
	int i = 0;
	for (i = 0;i < inst->total;i ++) 
    {
		if (strcmp(inst->table[i].key, key) == 0) // 找到键匹配的项，释放键、值的内存并置空
        {
			kvs_free(inst->table[i].key);
			inst->table[i].key = NULL;
			kvs_free(inst->table[i].value);
			inst->table[i].value = NULL;
			if (inst->total-1 == i) //是最后一个元素
            {
				inst->total --;
			}
			return 0;
		}
	}
	return i;//返回搜索索引，表示未找到
}

//获取一个键的值
char* kvs_array_get(kvs_array *inst,char *key)
{
    if (inst == NULL || key == NULL) return NULL;//实例空
    for(int i = 0; i < inst->total; i++)
    {
        if (inst->table[i].key == NULL) //跳过空键
        {
			continue;
		}
		if (strcmp(inst->table[i].key, key) == 0) //找到并返回值
        {
			return inst->table[i].value;
		}
    }
    return NULL;//未找到
}

//修改一个键的值
int kvs_array_mod(kvs_array *inst, char *key, char *value) {
	if (inst == NULL || key == NULL || value == NULL) return -1;//实例空
	if (inst->total == 0) //若表空，返回表的最大容量
    {
		return KVS_ARRAY_SIZE;
	}
	int i = 0;
	for (i = 0;i < inst->total;i ++) 
    {
		if (inst->table[i].key == NULL) //跳过空键
        {
			continue;
		}
		if (strcmp(inst->table[i].key, key) == 0) //找到要修改的值
        {
			kvs_free(inst->table[i].value);///释放旧值内存
			char *kvalue = kvs_malloc(strlen(value) + 1);// 分配新值的内存
			if (kvalue == NULL) return -2;//分配失败
			memset(kvalue, 0, strlen(value) + 1);//清空内存 
			strncpy(kvalue, value, strlen(value));//复制新值
			inst->table[i].value = kvalue;//保存新值
			return 0;
		}
	}
	return i;//未找到
}

//检查一个键是否存在
int kvs_array_exist(kvs_array *inst, char *key) 
{
	if (!inst || !key) return -1;	//实例或键为空
	char *str = kvs_array_get(inst, key);//获取值
	if (!str) 
    {
		return 1; // 键不存在
	}
	return 0;//存在
}

#endif