#include <stdio.h>
#include <liburing.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
//gcc -o   .... -luring

#define EVENT_ACCEPT   	0
#define EVENT_READ		1
#define EVENT_WRITE		2

#define ENTRIES_LENGTH		1024
#define BUFFER_LENGTH		1024

typedef int (*msg_handler)(char *msg, int length, char *response);//定义消息处理函数指针类型
static msg_handler kvs_handler;//全局消息处理函数指针

struct conn//连接结构体
{
    int fd;//文件描述符
    int event;//事件类型
};

//设置接收事件
int set_event_recv(struct io_uring *ring,int clientfd,char* buffer,int len,int flag)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//获取提交队列条目
    struct conn accept_info = {
		.fd = clientfd,//设置文件描述符
		.event = EVENT_READ,//设置事件类型为读事件
	};
    io_uring_prep_recv(sqe, clientfd, buffer,len, flag);//准备接收事件
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn));//将连接信息复制到用户数据
    return 0;
}

//设置发送事件
int set_event_send(struct io_uring *ring,int clientfd,char* buffer,int len,int flag)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);//获取提交队列条目
    struct conn accept_info = {
		.fd = clientfd,
		.event = EVENT_WRITE,//设置事件类型为写事件
	};
    io_uring_prep_send(sqe, clientfd, buffer,len, flag);//准备发送事件
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn));
    return 0;
}

//设置接受连接事件
int set_event_accept(struct io_uring *ring,int serverfd,struct sockaddr *addr,socklen_t *addrlen, int flags)
{
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    struct conn accept_info = {
		.fd = serverfd,
		.event = EVENT_ACCEPT,//设置事件为接受连接事件
	};
    io_uring_prep_accept(sqe, serverfd, (struct sockaddr*)addr, addrlen, flags);//准备接受连接事件
    memcpy(&sqe->user_data, &accept_info, sizeof(struct conn));

    return 0;
}

//初始化服务器套接字，绑定端口并开始监听
int init_server(unsigned short port)
{
    int serverfd = socket(AF_INET, SOCK_STREAM, 0);	
	struct sockaddr_in serveraddr;	
	memset(&serveraddr, 0, sizeof(struct sockaddr_in));	
	serveraddr.sin_family = AF_INET;	
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);	
	serveraddr.sin_port = htons(port);	
	if (-1 == bind(serverfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr))) 
    {		
		perror("bind");		
		return -1;	
	}	
	listen(serverfd, 10);
	return serverfd;
}

//启动服务器并进入事件循环
int proactor_start(unsigned short port, msg_handler handler)
{
    int serverfd = init_server(port);//初始化服务器
    kvs_handler = handler;//设置全局消息处理函数指针
    struct io_uring_params params;//定义io_uring参数结构体
    memset(&params, 0, sizeof(params));//清空结构体
    struct io_uring ring;//定义io_uring环形队列
    io_uring_queue_init_params(ENTRIES_LENGTH, &ring, &params);//初始化环形队列
    struct sockaddr_in clientaddr;	//定义客户端地址结构体
	socklen_t len = sizeof(clientaddr);// 客户端地址长度
	set_event_accept(&ring, serverfd, (struct sockaddr*)&clientaddr, &len, 0);// 设置接受连接事件
    char buffer[BUFFER_LENGTH] = {0};// 定义接收缓冲区
	char response[BUFFER_LENGTH] = {0};// 定义响应缓冲区
    while(1)
    {
        io_uring_submit(&ring);//提交所有准备好的事件
        struct io_uring_cqe *cqe;// 定义完成队列条目指针
		io_uring_wait_cqe(&ring, &cqe);// 等待一个完成的事件
        struct io_uring_cqe *cqes[128];// 定义完成队列条目数组
		int nready = io_uring_peek_batch_cqe(&ring, cqes, 128);// 获取完成的事件
        for(int i = 0; i<nready; i++)// 处理所有完成的事件
        {
            struct io_uring_cqe *entries = cqes[i];// 获取当前事件
			struct conn result;// 定义连接结果结构体
            memcpy(&result, &entries->user_data, sizeof(struct conn));// 将用户数据复制到连接结果
            if (result.event == EVENT_ACCEPT) //接受连接事件
            {
				set_event_accept(&ring, serverfd, (struct sockaddr*)&clientaddr, &len, 0);// 重新设置接受连接事件
				//printf("set_event_accept\n"); //
				int connfd = entries->res;// 获取新的客户端套接字
				set_event_recv(&ring, connfd, buffer, BUFFER_LENGTH, 0);//设置接收事件
			}
            else if (result.event == EVENT_READ) //读事件
            {  
			    int ret = entries->res;//获取读取结果
			    if (ret == 0) //连接关闭
                {
				    close(result.fd);
			    } 
                else if (ret > 0) //读取成功
                {  
                        //int kvs_protocol(char *msg, int length, char *response);
                    ret = kvs_handler(buffer, ret, response);  //获取响应数据
                    set_event_send(&ring, result.fd, response, ret, 0);//设置发送事件
                }
			}  
            else if (result.event == EVENT_WRITE) //写事件
            {
				int ret = entries->res;//获取写入结果
				//printf("set_event_send ret: %d, %s\n", ret, buffer);
				set_event_recv(&ring, result.fd, buffer, BUFFER_LENGTH, 0);//重新设置接收事件
			}
        }
        io_uring_cq_advance(&ring, nready);//前进完成队列
    }

}
/*
int kv_store(char *msg, int length, char *response)
{
    sprintf(response,msg,length);
    return length;
}

int main()
{
    proactor_start(9999,kv_store);
}
*/