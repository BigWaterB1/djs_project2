// proj2_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// server.cpp : 
//

#include "winsock.h"
#include "stdio.h"
//#pragma comment (lib,"wsock32.lib")
struct socket_list {
	SOCKET MainSock;
	int num;
	SOCKET sock_array[64];
};
void init_list(socket_list* list)
{
	int i;
	list->MainSock = 0;
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
void make_fdlist(socket_list* list, fd_set* fd_list)
{
	int i;
	FD_SET(list->MainSock, fd_list);
	for (i = 0; i < 64; i++) {
		if (list->sock_array[i] > 0) {
			FD_SET(list->sock_array[i], fd_list);
		}
	}
}
int main(int argc, char* argv[])
{
	SOCKET s, sock;
	struct sockaddr_in ser_addr, remote_addr;
	int len;
	char buf[128];
	WSAData wsa;
	int retval;
	struct socket_list sock_list;
	fd_set readfds, writefds, exceptfds;
	timeval timeout;
	int i;
	unsigned long arg;

	WSAStartup(0x101, &wsa);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == SOCKET_ERROR)
		return 0;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(0x1234);
	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	listen(s, 5);
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	sock_list.MainSock = s;
	arg = 1;
	ioctlsocket(sock_list.MainSock, FIONBIO, &arg);
	while (1) {
		make_fdlist(&sock_list, &readfds);
		//make_fdlist(&sock_list,&writefds);
		//make_fdlist(&sock_list,&exceptfds);
		retval = select(0, &readfds, &writefds, &exceptfds, NULL);
		if (retval == SOCKET_ERROR) {
			retval = WSAGetLastError();
			break;
		}
		if (FD_ISSET(sock_list.MainSock, &readfds)) {
			len = sizeof(remote_addr);
			sock = accept(sock_list.MainSock, (sockaddr*)&remote_addr, &len);
			if (sock == SOCKET_ERROR)
				continue;
			printf("accept a connection\n");
			insert_list(sock, &sock_list);
		}
		for (i = 0; i < 64; i++) {
			if (sock_list.sock_array[i] == 0)
				continue;
			sock = sock_list.sock_array[i];
			if (FD_ISSET(sock, &readfds)) {
				retval = recv(sock, buf, 128, 0);
				if (retval == 0) {
					closesocket(sock);
					printf("close a socket\n");
					delete_list(sock, &sock_list);
					continue;
				}
				else if (retval == -1) {
					retval = WSAGetLastError();
					if (retval == WSAEWOULDBLOCK)
						continue;
					closesocket(sock);
					printf("close a socket\n");
					delete_list(sock, &sock_list);
					continue;
				}
				buf[retval] = 0;
				printf("->%s\n", buf);
				send(sock, "ACK by server", 14, 0);
			}
			//if(FD_ISSET(sock,&writefds)){
			//}
			//if(FD_ISSET(sock,&exceptfds)){

		}
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
	}
	closesocket(sock_list.MainSock);
	WSACleanup();
	return 0;
}


