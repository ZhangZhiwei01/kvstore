#include "../include/server.h" 
#include<errno.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<stdio.h>
#include <string.h>

//逐行读取数据，处理换行符和回车符
ssize_t recv_line(int fd, char *buffer, size_t max_length,int flag) 
{
    size_t total_read = 0;// 记录已经读取的字符数
    char c;// 存储每次读取的单个字符
    int n;// 存储recv函数的返回值
    while (total_read < max_length - 1) 
    {
        //每次读取一个字符
        n = recv(fd, &c, 1, flag);
        if (n > 0) {
            if (c == '\r') {
                // 查看下一个字符
                n = recv(fd, &c, 1, MSG_PEEK);
                if (n > 0 && c == '\n') 
                {
                    // 读取'\n'
                    recv(fd, &c, 1, flag);
                }
                break;
            } 
            else if (c == '\n') 
            {
                break;
            } 
            else 
            {
                buffer[total_read++] = c;// 如果是普通字符，存储到缓冲区
            }
        } else if (n == 0) 
        {
            // 连续关闭
            break;
        } else {
            //出错
            return -1;
        }
    }

    buffer[total_read] = '\0';// 添加字符串结束符
    return total_read;
}//udp通信 蓝颜传东西 tcp wifi 物联网一个局域网的所有设备 广播 udp广播 udp可靠传输 udp并发
//粘包 分包 丢包问题 tcp不丢包 //存储引擎 你用了四种数据结构 发布订阅 范围查询 debug日志模式 内存池 中间件 基础架构 物联网 dpdk写过简单用户态协议栈 redis网络 nginx网络 skynet 内存池 线程池
typedef int (*msg_handler)(char *msg, int length, char *response);// 定义消息处理函数指针类型
static msg_handler kvs_handler;// 全局消息处理函数指针

int kk(char *msg, int length, char *response)
{
	printf("%s\n",msg);
	memcpy(response, msg, length);
	return length;

}

//处理请求
int kvs_request(struct conn *c) 
{
	//清空缓冲区
	memset(c->writebuffer,0,BUFFER_LENGTH);
    // 调用消息处理函数
	c->writelength = kvs_handler( c->readbuffer, c->readlength,c->writebuffer);
	return 0;
}

//处理响应
int kvs_response(struct conn *c) 
{

}
//这里可以把这个静态的handler丢给用户 用户写handler 就可以决定request_callback() 从而影响
//reponse_callback()

#define CONNECTION_SIZE  1024 //这里是直接定义好一个数组给携带这fd的conn用 不过最好不要直接用数组 内存开销太大 定制内存分配策略 链表 最好 
struct conn conn_pool[CONNECTION_SIZE] = {0};
int epollfd = 0; //epoll_ctl epoll_wait 都需要一个epollfd管理 我们在start函数里create 
int accept_callback(int fd);
int recv_callback(int fd);
int send_callback(int fd); //这三个函数指针 用于conn结构体里的回调函数

int set_event(int fd,int event,int flag)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = event;
	//flag 用于确定是epoll_ctl 添加还是修改 true添加 false修改
	if(flag)
		epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
	else
		epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);

	return 0;
}
//单线程 多线程 

//用于将recv send的fd添加到结构中 然后调用set_event 给epoll注册 
int event_register(int fd, int event)
{
    if(fd < 0) return -1;//出错
    conn_pool[fd].fd = fd;
    conn_pool[fd].recv_callback = recv_callback;
    conn_pool[fd].send_callback = send_callback;
	
	//清空读缓冲区
    memset(conn_pool[fd].readbuffer, 0, BUFFER_LENGTH);
	conn_pool[fd].readlength = 0;

    //清空写缓冲区
	memset(conn_pool[fd].writebuffer, 0, BUFFER_LENGTH);
	conn_pool[fd].writelength = 0;

	set_event(fd, event, 1);//注册事件
    return 0;
}

//初始化服务器，返回监听套接字
int init_server(unsigned short port) 
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0
	servaddr.sin_port = htons(port); // 0-1023, 
	if (-1 == bind(sockfd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr))) {
		printf("bind failed: %s\n", strerror(errno));
	}
	listen(sockfd, 10);
	//printf("listen finshed: %d\n", sockfd); // 3 0-2分别是标准输入 输出 错误
	return sockfd;
}//初始化一个listen fd 并返回其fd

//处理新的连接请求
int accept_callback(int fd)
{
    struct sockaddr_in  clientaddr;
	socklen_t len = sizeof(clientaddr);

	int clientfd = accept(fd, (struct sockaddr*)&clientaddr, &len);
	if (clientfd < 0) 
	{
		printf("accept errno: %d --> %s\n", errno, strerror(errno));
		return -1;
	}
    event_register(clientfd, EPOLLIN);  //// 将新连接注册到epoll中，监听读事件
}

//处理接收到的数据
int recv_callback(int fd)
{
	// int norecvend recvend 
	memset(conn_pool[fd].readbuffer, 0, BUFFER_LENGTH);//清空读缓冲区
	int count = recv_line(fd, conn_pool[fd].readbuffer, BUFFER_LENGTH, 0);//逐行读取数据
	if (count == 0) 
    { //连接断开
		//printf("client disconnect: %d\n", fd);
		close(fd);//连接断开
		epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL); // 删除文件描述符
		return 0;
	} 
    else if (count < 0) //出错
    { 
		printf("count: %d, errno: %d, %s\n", count, errno, strerror(errno));
		close(fd);
		epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
		return 0;
	}

	conn_pool[fd].readlength = count;//设置读取到的数据长度
	//printf("RECV: %s\n", conn_list[fd].rbuffer);
	kvs_request(&conn_pool[fd]);//处理请求

	set_event(fd, EPOLLOUT, 0);//设为可读事件 这里可以解决粘包问题 写一个状态机 把粘包问题丢给上层的kv_request协议
	//kv_request协议决定是否读完 并设置状态 这里根据状态判断 改为epoll_out还是epoll_in
	return count;
}

//处理发送的数据
int send_callback(int fd)
{
	kvs_response(&conn_pool[fd]); //这里丢给上层协议 他去设置writebuffer和发送长度
	int count = 0;
	if (conn_pool[fd].writelength != 0) 
	{
		count = send(fd, conn_pool[fd].writebuffer, conn_pool[fd].writelength, 0);//发送数据
	}	
	set_event(fd, EPOLLIN, 0);//设置为可读事件
	return count;
}

//启动反应堆模式的事件循环
int reactor_start(unsigned short port, msg_handler handler)
{
	kvs_handler = handler;//设置全局消息处理函数指针
    epollfd = epoll_create(1);//创建epoll实例 历史原因 原先epoll是数组 现在是链表 不写0写什么都行
    int serverfd = init_server(port);//初始化服务器
    //接下来需要给listenfd 绑定对应的事件 即recv函数
    conn_pool[serverfd].fd = serverfd;
    conn_pool[serverfd].recv_callback = accept_callback;//1 accept 2 clientfd epoll 3 conn
	set_event(serverfd, EPOLLIN, 1);//注册监听套接字的读事件
	while (1) 
	{ //主循环
		struct epoll_event events[1024] = {0};//事件数组
		int nready = epoll_wait(epollfd, events, 1024, -1);//等待事件发生
		int i = 0;
		for (i = 0;i < nready;i ++) //处理所有发生的事件
		{
			int connfd = events[i].data.fd;//获取事件关联的文件描述符
			if (events[i].events & EPOLLIN) //读事件
            {
				conn_pool[connfd].recv_callback(connfd); //你怎么知道下一步要设置成epool out
			} 
			if (events[i].events & EPOLLOUT)//写事件 
            {
				conn_pool[connfd].send_callback(connfd);
			}
		}
	}
}

int main()
{
	reactor_start(8888,kk);
}

//继承 main函数（两个参数）入口函数 编译过不了 
// 








