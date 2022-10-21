// proj2_server.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
// server.cpp : 
//

#include "winsock.h"
#include "stdio.h"

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
	if(list->KeyboardSock>0) FD_SET(list->KeyboardSock, fd_list);//fd_list中必须为大于0
	for (i = 0; i < 64; i++) {
		if (list->sock_array[i] > 0) {
			FD_SET(list->sock_array[i], fd_list);
		}
	}
}
int main(int argc, char* argv[])
{
	SOCKET s, sock, sock_keyboard, skey;
	struct sockaddr_in ser_addr, remote_addr, keyboard_addr;
	int len;
	char buf[128], sendbuf[128];
	WSAData wsa;
	int retval;
	struct socket_list sock_list;
	fd_set readfds, writefds, exceptfds;
	timeval timeout;
	int i;
	unsigned long arg;
	int ifsend = 0;

	WSAStartup(0x101, &wsa);
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == SOCKET_ERROR)
		return 0;

	sock_keyboard = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_keyboard == SOCKET_ERROR)
		return 0;

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(0x1234);
	bind(s, (sockaddr*)&ser_addr, sizeof(ser_addr));

	keyboard_addr.sin_family = AF_INET;
	keyboard_addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
	keyboard_addr.sin_port = htons(0x4321);
	bind(sock_keyboard, (sockaddr*)&keyboard_addr, sizeof(keyboard_addr));

	listen(s, 5);
	listen(sock_keyboard, 5);

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	init_list(&sock_list);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
	sock_list.MainSock = s;
	arg = 1;
	sock = 0;
	ioctlsocket(sock_list.MainSock, FIONBIO, &arg);

	ioctlsocket(sock_keyboard, FIONBIO, &arg);

	while (1) {
		make_fdlist(&sock_list, &readfds);
		//make_fdlist(&sock_list,&writefds);
		//make_fdlist(&sock_list,&exceptfds);

		FD_SET(sock_keyboard, &readfds);

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
			printf("accept a client connection\n");
			insert_list(sock, &sock_list);
		}
		if (FD_ISSET(sock_keyboard, &readfds)) {
			len = sizeof(remote_addr);
			skey = accept(sock_keyboard, (sockaddr*)&remote_addr, &len);
			if (skey == SOCKET_ERROR)
				continue;
			printf("accept a keyboard connection\n");
			sock_list.KeyboardSock = skey;
		}
		for (i = 0; i < 64; i++) {
			if (FD_ISSET(sock_list.KeyboardSock, &readfds)) {
				retval = recv(skey, sendbuf, 128, 0);
				if (retval == 0) {
					closesocket(skey);
					printf("keyboard disconnected\n");
					sock_list.KeyboardSock = 0;
					continue;
				}
				else if (retval == -1) {
					retval = WSAGetLastError();
					if (retval == WSAEWOULDBLOCK)
						continue;
					closesocket(skey);
					printf("keyboard disconnected\n");
					sock_list.KeyboardSock = 0;
					continue;
				}
				sendbuf[retval] == 0;
				//ifsend = 1;//标志，从键盘接收到的消息保证只发一次
				if (sock > 0) {
					if (send(sock, sendbuf, strlen(sendbuf), 0) > 0) {
						printf("you: %s\n", sendbuf);
					}//收到即发，单用户可以这样做
					else {
						printf("failed to send");
					}
				}
				else {
					printf("no avaliable socket conneted");
					continue;
				}
				//ifsend = 0;
			}
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
				buf[retval] = 0;//因为不能保证发送方的send函数都+1了
				printf("->%s\n", buf);
				//send(sock, "ACK by server", 14, 0);
			}
			if(FD_ISSET(sock,&writefds)){
			//对于多个client的话，最好在这个地方发送，而不是收到即发，可能会发错人
					/*printf("you: %s", sendbuf);
					send(sock, sendbuf, strlen(sendbuf), 0);
					ifsend = 0;*/
			
			}
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


