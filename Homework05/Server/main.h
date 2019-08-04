#pragma once
#include<WinSock2.h>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<unordered_map>
#include<forward_list>
#pragma comment(lib,"ws2_32.lib")

#define WM_SOCKET WM_USER + 1
#define SERVER_PORT 5500
#define BUFF_SIZE 1024

extern SOCKET listenSock;	//listen Socket

/*Struct stores acccount infomations*/
typedef struct accountinfo {
	char password[50];
	char status;
	long int seek_pos;	//offset from SEEK_SET of the account.txt file
} ACCOUNTINFO;

/*Struct stores infomation about a client which connect to this server*/
typedef struct client {
	std::string ipAddr;
	u_short port;

	//save the number of times client attempt to login inlegally
	std::unordered_map<std::string, char> login_log;
	/*step indicated status of session
		0: unidentified- username isn't invalid
		1: unauthenticated -password is not verified
		2: authenticated -username and password are verified, in logged in*/
	int step;
	std::string username;	//current username
	ACCOUNTINFO acc;	//current account with username
} CLIENT;

// pointer to FILE contain account infomation
extern FILE* file;

// Map contains account info
extern std::unordered_map<std::string, ACCOUNTINFO> account_info;

// Map contains clients and their status
extern std::unordered_map<SOCKET, CLIENT> client_list;