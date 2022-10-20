﻿// proj2_keyboard.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
//#include <stdlib.h>
#define SERVER_PORT 0x4321

int main(int argc, char* argv[])
{
	SOCKET sock;
	struct sockaddr_in server;
	char* buf;
	int len;
	WSAData wsa;
	int val;
	int count = 0;

	WSAStartup(0x101, &wsa);
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("Creating socket error\n");
		return 0;
	}


	server.sin_family = AF_INET;
	server.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);//inet_addr("202.115.12.35");
	server.sin_port = htons(SERVER_PORT);

	if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
		val = WSAGetLastError();
		printf("Conneting to server error\n");
		return 0;
	}

	printf("keyboard connect to server\n");

	buf = (char*)malloc(sizeof(char)*2048);
	//scan the keboard until input string is "exit"
	while (1) {
		printf("Send = ");
		scanf_s("%s", buf, 2048);
		if (strcmp(buf, "exit") == 0)
			break;
		send(sock, buf, strlen(buf)+1, 0);
	}

	//close the socket immediately
	closesocket(sock);
	WSACleanup();
	free(buf);
	printf("passed\n");
	return 0;
}

