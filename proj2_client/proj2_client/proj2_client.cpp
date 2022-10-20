// proj2_client.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
//#include <stdlib.h>
#define SERVER_PORT 0x1234

struct socket_list {
	SOCKET MainSock;
	SOCKET KeyboardSock;
	int num;
	SOCKET sock_array[64];
};
void init_list(socket_list* list)
{
	int i;
	list->MainSock = 0;
	list->KeyboardSock = 0;
	list->num = 0;
	for (i = 0; i < 64; i++) {
		list->sock_array[i] = 0;
	}
}
void insert_list(SOCKET s, socket_list* list)
{
	int i;
	for (i = 0; i < 64; i++) {
		if (list->sock_array[i] == 0) {
			list->sock_array[i] = s;
			list->num += 1;
			break;
		}
	}
}
void delete_list(SOCKET s, socket_list* list)
{
	int i;
	for (i = 0; i < 64; i++) {
		if (list->sock_array[i] == s) {
			list->sock_array[i] = 0;
			list->num -= 1;
			break;
		}
	}
}
//将socket_list填入fd_set
void make_fdlist(socket_list* list, fd_set* fd_list)
{
	int i;
	FD_SET(list->MainSock, fd_list);
	if (list->KeyboardSock > 0) FD_SET(list->KeyboardSock, fd_list);//fd_list中必须为大于0
	for (i = 0; i < 64; i++) {
		if (list->sock_array[i] > 0) {
			FD_SET(list->sock_array[i], fd_list);
		}
	}
}
int main(int argc, char* argv[])
{
	SOCKET sock, s, s_keyboard;
	struct sockaddr_in server, ser_addr, remote_addr;
	char* buf;
	int len;
	WSAData wsa;
	int val;
	fd_set readfds, writefds, exceptfds;
	unsigned long arg = 1;
	int ifsend = 0;
	char sendbuf[128];

	WSAStartup(0x101, &wsa);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Creating socket error\n");
		return 0;
	}
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		printf("Creating socket error\n");
		return 0;
	}
	//自己地址
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(0x1111);//自己地址
	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	//服务器地址
	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);//inet_addr("202.115.12.35");
	server.sin_port = htons(SERVER_PORT);

	if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
		val = WSAGetLastError();
		printf("Conneting to server error\n");
		return 0;
	}
	printf("connect to server\n");

	listen(s, 5);

	buf = (char*)malloc(2048);
	//scan the keboard until input string is "exit"
	
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);

	ioctlsocket(s, FIONBIO, &arg);

	while (1) {
		FD_SET(sock, &readfds);
		FD_SET(s, &readfds);

		val = select(0, &readfds, &writefds, &exceptfds, NULL);//有问题
		if (val == SOCKET_ERROR) {
			val = WSAGetLastError();
			break;
		}

		//看是否有键盘连接
		if (FD_ISSET(s, &readfds)) {
			len = sizeof(remote_addr);
			s_keyboard = accept(s, (sockaddr*)&remote_addr, &len);
			if (s_keyboard == SOCKET_ERROR)
				continue;
			printf("accept a keyboard connection\n");
		}
		//键盘输入
		if (FD_ISSET(s_keyboard, &readfds)) {
			val = recv(s_keyboard, sendbuf, 128, 0);
			if (val == 0) {
				closesocket(s_keyboard);
				printf("keyboard disconnected\n");
				continue;
			}
			else if (val == -1) {
				val = WSAGetLastError();
				if (val == WSAEWOULDBLOCK)
					continue;
				closesocket(s_keyboard);
				printf("keyboard disconnected\n");
				continue;
			}
			sendbuf[val] == 0;
			printf("form keyboard: %s\n", sendbuf);
			send(sock, sendbuf, strlen(sendbuf), 0);//从键盘得到立即发送
		}

		if (FD_ISSET(sock, &readfds)) {
				val = recv(sock, buf, 100, 0);
				if (val == 0) {
					closesocket(sock);
					printf("disconnected\n");
					continue;
				}
				else if (val == -1) {
					val = WSAGetLastError();
					if (val == WSAEWOULDBLOCK)
						continue;
					closesocket(sock);
					printf("disconnected\n");
					continue;
				}
				buf[val] = 0;
				printf("->: %s\n", buf);
				
		}
		
		//	printf("Send = ");
		//	scanf_s("%s", buf, 2048);
		//	if (strcmp(buf, "exit") == 0)
		//		break;
		//	send(sock, buf, strlen(buf), 0);
		//	
		//val = recv(sock, buf, 100, 0);//recv返回buf长度大小
		//if (val > 0) {
		//	//buf[val] = 0;//send处注意+1，否则就在这手动加终止符
		//	printf("%s\n", buf);
		//}
		//else
		//	break;
	}

	//close the socket immediately
	closesocket(sock);
	WSACleanup();
	free(buf);
	printf("passed\n");
	return 0;
}
