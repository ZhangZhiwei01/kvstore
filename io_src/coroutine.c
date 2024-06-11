#include "nty_coroutine.h"
#include <arpa/inet.h>
//gcc -o t -I ../NtyCo/core/  -L ../NtyCo/  coroutine.c   -lntyco -lpthread -ldl
typedef int (*msg_handler)(char *msg, int length, char *response);//定义消息处理函数指针类型
static msg_handler kvs_handler;//全局消息处理函数指针

//从套接字读取一行数据
ssize_t recv_line(int fd, char *buffer, size_t max_length,int flag)
{
    size_t total_read = 0;//已读的总字数
    char c;//每次读取的字符
    int n;//recv返回值
    while (total_read < max_length - 1) 
    {
        //每次读取一个字符
        n = recv(fd, &c, 1, flag);
        if (n > 0) 
        {
            if (c == '\r') 
            {
                //查看下一个字符是不是换行
                n = recv(fd, &c, 1, MSG_PEEK);
                if (n > 0 && c == '\n') 
                {
                    //是换行符
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
                buffer[total_read++] = c;//将字符存储到缓冲区
            }
        } 
        else if (n == 0) 
        {
            //连接关闭
            break;
        } 
        else 
        {
            // 出错
            return -1;
        }
    }
    buffer[total_read] = '\0';//添加字符串结束符
    return total_read;//返回读取的总字数
}

//接受新的客户端连接并创建新的协程
void server_reader(void *arg) 
{
	int fd = *(int *)arg;// 获取客户端套接字
	int ret = 0;//recv和send的返回值

	while (1) 
    {
		char buf[1024] = {0};//接收到的数据
		ret = recv(fd, buf, 1024, 0);// 从客户端套接字接收数据
		if (ret > 0) 
        {
			char response[1024] = {0};//响应数据
			int slength = kvs_handler(buf, ret, response);//调用消息处理函数，获取响应长度
			ret = send(fd, response, slength, 0);//把响应发到客户端
			if (ret == -1) //发送失败
            {
				close(fd);
				break;
			}
		} 
        else if (ret == 0) 
        {	
			close(fd);
			break;
		}
	}
}

//负责接受客户端连接的协程函数
void server(void *arg)
{
    unsigned short port = *(unsigned short *)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return ;
    struct sockaddr_in local, remote;//定义本地和远程地址结构体
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr*)&local, sizeof(struct sockaddr_in));
	listen(fd, 20);
    while (1) 
    {
		socklen_t len = sizeof(struct sockaddr_in);//远程地址长度
		int cli_fd = accept(fd, (struct sockaddr*)&remote, &len);//接受连接
		nty_coroutine *read_co;//定义协程指针
		nty_coroutine_create(&read_co, server_reader, &cli_fd);//创建新的协程处理客户端请求
	}
}

//初始化服务器并启动调度器，通过NtyCo库管理协程
int coroutine_start(unsigned short port, msg_handler handler) 
{
	//int port = atoi(argv[1]);
	kvs_handler = handler;//设置全局消息处理函数指针
	nty_coroutine *co = NULL;//定义协程指针
	nty_coroutine_create(&co, server, &port);//创建服务器协程
	nty_schedule_run();//启动协程调度器
}
/* 测试
int kv_store(char *msg, int length, char *response)
{
    sprintf(response,msg,length);
    return length;
}

int main()
{
    coroutine_start(9999,kv_store);
}
*/