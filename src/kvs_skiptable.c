#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include"../include/kvstore.h"
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define MAX_LEVEL 6 //跳表最大层数
kvs_skiptable global_skiptable; //定义全局跳表

//生成一个随机层数
int randomLevel()
{
    int level = 0;//层数
    while(rand() < RAND_MAX/2 && level < MAX_LEVEL)
    {
        level++;
    }
    return level;
}

//创建一个新的跳表节点
kvs_skipnode* createSkipNode(int level,char *key,char *value)
{
    kvs_skipnode *newnode = (kvs_skipnode*)kvs_malloc(sizeof(kvs_skipnode));//分配新节点的内存
    if(!newnode) return NULL;//分配失败
    newnode->key = (char *)kvs_malloc(strlen(key)+1);//分配键的内存
    if (newnode->key == NULL) return NULL;//分配失败
	memset(newnode->key, 0, strlen(key) + 1);//清空键的内存
	strncpy(newnode->key, key, strlen(key));//复制键
   
    
    newnode->value = (char *)kvs_malloc(strlen(value)+1);//分配值的内存
    if (newnode->value == NULL) return NULL;//分配失败
	memset(newnode->value, 0, strlen(value) + 1);//清空内存
	strncpy(newnode->value, value, strlen(value));//复制

    newnode->next = (kvs_skipnode**)kvs_malloc((level + 1) * sizeof(kvs_skipnode*));// 分配指向下一层节点的指针数组的内存
    if(!newnode->next)   return NULL;

    for(int i = 0;i <= level;i++)// 初始化指针数组为空
    {
        newnode->next[i] = NULL;
    }
    return newnode;
}

//创建并初始化一个跳表
int kvs_skip_create(kvs_skiptable* table)
{
    if(!table) return -1;//表空
    table->level = 0;//初始化表层数为0
    table->head = createSkipNode(MAX_LEVEL,"","");//创建一个具有最大层数的头节点
    if(!table->head) return -1;//创建失败
    return 0;//成功
}

//销毁一个跳表并释放内存
void kvs_skip_destory(kvs_skiptable* table)
{   
        kvs_skipnode *root = table->head->next[0];// 获取头节点的第0层节点     
        while(root != NULL)//若节点不为空 销毁节点 直到全空
        {
            kvs_skipnode *tem = root;
            root = root->next[0];
            kvs_free(tem->key);
            kvs_free(tem->value);
            kvs_free(tem->next);
            kvs_free(tem);
        }
        kvs_free(table);   
}

//从跳表中获取一个键的值
char* kvs_skip_get(kvs_skiptable*table,char *key)
{
    if(!key || !table) return NULL;//键或表空
    kvs_skipnode *currentnode = table->head;// 获取头节点
    for(int i = table->level; i>=0 ;i--)//从最高层开始查找
    {
        while(currentnode->next[i] != NULL && strcmp(currentnode->next[i]->key,key) < 0)/*在当前层，查找所有键值比目标键小的节点
        如果当前节点的下一个节点（currentnode->next[i]）存在且其键值比目标键小，则继续向前移动到下一个节点。*/
        {
            currentnode = currentnode->next[i];
        }
    }
    currentnode = currentnode->next[0]; //在第0层进行一次检查，因为如果上面的层次没有找到目标节点，必须检查第0层以确保最终找到或确认不存在
    if(currentnode && strcmp(key,currentnode->key) == 0)//找到了
        return currentnode->value;
    return NULL;
}

//在跳表中设置一个新的键值对
int kvs_skip_set(kvs_skiptable *table,char *key,char *value)
{
    if(!table || !key || !value) return -1;
    kvs_skipnode* updatenode[MAX_LEVEL + 1];// 定义存储需要更新的节点数组
    kvs_skipnode *currentnode = table->head;// 获取头节点
    for(int i = table->level; i>=0 ;i--)
    {
        while(currentnode->next[i] != NULL && strcmp(currentnode->next[i]->key,key) < 0)
        {
            currentnode = currentnode->next[i];
        }
        updatenode[i] = currentnode;// 存储需要更新的节点
    }
    currentnode = currentnode->next[0]; 
    if(currentnode == NULL || strcmp(currentnode->key,key) != 0)//未找到
    {
        int setlevel = randomLevel();//生成随机层数
        if(setlevel > table->level)// 如果新层数大于当前层数
        {
            for (int i = table->level + 1; i <= setlevel; ++i)
            {
                updatenode[i] = table->head;// 更新新的层数的节点
            }//update[i] 为null
            table->level = setlevel;//更新表的层数
        }
        kvs_skipnode* newnode = createSkipNode(setlevel, key, value);//创建新节点
        if(!newnode) return -1;
        for(int i = 0; i<= setlevel ;i++)//更新节点指针
        {
            newnode->next[i] = updatenode[i]->next[i];
            updatenode[i]->next[i] = newnode;
        }
        return 0;
    }
    return 1; 
}

//检查跳表中是否存在一个键
int kvs_skip_exist(kvs_skiptable *table,char *key)
{
    if(!table || !key)//表或键为空
        return -1; 
    if(kvs_skip_get(table,key) == NULL)
        return 1;//不存在
    else
        return 0; //存在
}

//修改跳表中一个键的值
int kvs_skip_mod(kvs_skiptable *table,char *key,char *value)
{
    if(!table || !key || !value) return -1;

    kvs_skipnode *currentnode = table->head;

    for(int i = table->level; i >= 0 ;i--)
    {
        while(currentnode->next[i] != NULL && strcmp(currentnode->next[i]->key,key) < 0)
        {
            currentnode = currentnode->next[i];
        }
    }
    currentnode = currentnode->next[0];
    if(currentnode && strcmp(key,currentnode->key) == 0)// 如果找到键
    {
        kvs_free(currentnode->value);// 释放旧值的内存
        currentnode->value = (char *)kvs_malloc(strlen(value) + 1);// 分配新值的内存
        if (currentnode->value == NULL) return -1; //分配失败     
        memset(currentnode->value, 0, strlen(value) + 1);
        strncpy(currentnode->value, value, strlen(value));
        return 0;//成功
    }    
    return 1;//需要修改的键不存在
}

//删除跳表中的一个键值对
int kvs_skip_del(kvs_skiptable *table,char *key)
{
    if(!table || !key) return -1;
    kvs_skipnode* updatenode[table->level + 1];// 定义存储需要更新的节点数组
    kvs_skipnode *currentnode = table->head;
    for(int i = table->level; i>=0 ;i--)
    {
        while(currentnode->next[i] != NULL && strcmp(currentnode->next[i]->key,key) < 0)
        {
            currentnode = currentnode->next[i];
        }
        updatenode[i] = currentnode;// 存储需要更新的节点
    }
    kvs_skipnode *tem = currentnode->next[0];// 获取要删除的节点
    if(!tem || strcmp(key,tem->key) != 0)
        return 1;
    for(int i = 0; i <= table->level; i++)// 更新节点的指针
    {
        if(updatenode[i]->next[i] && strcmp(key,updatenode[i]->next[i]->key) == 0)
            {
                updatenode[i]->next[i] = updatenode[i]->next[i]->next[i];
            }
        else
            break;//未找到
    }
    for(int i = 0;i<table->level;i++)//更新表的层数
    {
        if(table->head->next[i] == NULL)
        {
            table->level = MAX(0,i-1);
            break;
        }
    }
    //释放键、值、指针数组、节点的内存
    kvs_free(tem->key);
    kvs_free(tem->value);
    kvs_free(tem->next);
    kvs_free(tem);
    return 0;
}

//范围查找
