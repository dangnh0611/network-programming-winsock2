#pragma once
#include<stdio.h>
#include<WinSock2.h>
#define BUFF_SIZE 1024

typedef struct client {
	SOCKET socket;	//client socket
	SOCKADDR_IN addr;	//client address
	FILE* file;		//temp file used for client
	char filename[30];	//temp file's name for client-for clean resource purpose
	u_char control;	//0 if encrypt, 1 if decrypt
	u_char key;		//key used
} CLIENT;