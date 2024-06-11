#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define MAX_MSG_LENGTH		1024
#define TIME_SUB_MS(tv1, tv2)  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

int connect_tcpserver(const char *ip, unsigned short port) {

	int connfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	if (0 !=  connect(connfd, (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in))) {
		perror("connect");
		return -1;
	}
	
	return connfd;
	
}


int send_msg(int connfd, char *msg, int length) {

	int res = send(connfd, msg, length, 0);
	if (res < 0) {
		perror("send");
		exit(1);
	}
	return res;
}

int recv_msg(int connfd, char *msg, int length) {
	int res = recv(connfd, msg, length, 0);
	if (res < 0) {
		perror("recv");
		exit(1);
	}
	return res;
}

void testcase(int connfd, char *msg, char *pattern, char *casename) {

	if (!msg || !pattern || !casename) return ;

	send_msg(connfd, msg, strlen(msg));

	char result[MAX_MSG_LENGTH] = {0};
	recv_msg(connfd, result, MAX_MSG_LENGTH);

	
    if (strcmp(result, pattern) == 0) {
		printf("==> PASS ->  %s\n", casename);
	} else {
		printf("==> FAILED -> %s, '%s' != '%s' \n", casename, result, pattern);
		exit(1);
	}
    
    

}

void array_testcase(int connfd) {

	
    testcase(connfd, "SET Teacher haoyu\r\n", "OK\r\n", "SET-Teacher");
	testcase(connfd, "GET Teacher\r\n", "haoyu\r\n", "GET-Teacher");
	testcase(connfd, "MOD Teacher tianqi\r\n", "OK\r\n", "MOD-Teacher");
	testcase(connfd, "GET Teacher\r\n", "tianqi\r\n", "GET-Teacher");
	testcase(connfd, "EXIST Teacher\r\n", "EXIST\r\n", "GET-Teacher");
	testcase(connfd, "DEL Teacher\r\n", "OK\r\n", "DEL-Teacher");
	testcase(connfd, "GET Teacher\r\n", "NO EXIST\r\n", "GET-Teacher");
	testcase(connfd, "MOD Teacher haoyu\r\n", "NO EXIST\r\n", "MOD-Teacher");
	testcase(connfd, "EXIST Teacher\r\n" ,"NO EXIST\r\n", "GET-Teacher");

}

void array_testcase_1w(int connfd) {

	int count = 90000;
	int i = 0;

	struct timeval tv_begin;
	gettimeofday(&tv_begin, NULL);

	for (i = 0;i < count;i ++) {

		testcase(connfd, "SET Teacher haoyu\r\n", "OK\r\n", "SET-Teacher");
        testcase(connfd, "GET Teacher\r\n", "haoyu\r\n", "GET-Teacher");
        testcase(connfd, "MOD Teacher tianqi\r\n", "OK\r\n", "MOD-Teacher");
        testcase(connfd, "GET Teacher\r\n", "tianqi\r\n", "GET-Teacher");
        testcase(connfd, "EXIST Teacher\r\n", "EXIST\r\n", "GET-Teacher");
        testcase(connfd, "DEL Teacher\r\n", "OK\r\n", "DEL-Teacher");
        testcase(connfd, "GET Teacher\r\n", "NO EXIST\r\n", "GET-Teacher");
        testcase(connfd, "MOD Teacher haoyu\r\n", "NO EXIST\r\n", "MOD-Teacher");
        testcase(connfd, "EXIST Teacher\r\n" ,"NO EXIST\r\n", "GET-Teacher");

	}

	struct timeval tv_end;
	gettimeofday(&tv_end, NULL);

	int time_used = TIME_SUB_MS(tv_end, tv_begin); // ms

	printf("array testcase --> time_used: %d, qps: %d\n", time_used, 90000 * 1000 / time_used);

}

// testcase 127.0.0.1  9999
int main(int argc, char *argv[]) {

	if (argc != 4) {
		printf("arg error\n");
		return -1;
	}

	char *ip = argv[1];
	int port = atoi(argv[2]);
	int mode = atoi(argv[3]);

	int connfd = connect_tcpserver(ip, port);

	
	if (mode == 1) 
		array_testcase_1w(connfd);

    getchar();
	



}