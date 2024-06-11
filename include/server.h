#pragma once
//kv存储我们采用三种网络 reactor封装的epoll iouring 协程（如果windows 可以用iocp）
//协程我们采用两种 一种是自己实现的ucontext 一种是调用开源框架 汇编编写的 协程的好处就是
//同步的书写 异步的执行 我们直接send recv调用就行 send recv已经被hook住
//执行recv 若缓冲区无数据 直接返回到协程调度器 由他进行调度 进入wait状态

//这个头文件是给reactor.c用的
#define BUFFER_LENGTH 1024

typedef int (*RCALLBACK)(int fd); //函数指针 

//send() sendto() char senddata[1024] 
// bind() listen() -> (int fd)accept() ->send(fd,char *,len,timeval)  recv(fd,char *,1024,timeval)

struct conn
{
    int fd;

    char readbuffer[BUFFER_LENGTH];
    unsigned int readlength;

    char writebuffer[BUFFER_LENGTH];
    unsigned int writelength; //send的参数

    RCALLBACK send_callback;
    RCALLBACK recv_callback; // ->kvs_handler
};//reactor 封装每个io的fd 让其携带事件 由事件驱动

//int reactorstart();

int kvs_request(struct conn*);
int kvs_response(struct conn*);//.c文件实现里 我们要实现针对kv存储的事件
