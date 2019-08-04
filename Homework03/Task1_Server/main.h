#pragma once
#include<stdio.h>
#include<process.h>
#include<string>
#include<unordered_map>
#include<forward_list>
#include<WinSock2.h>
#pragma comment (lib,"ws2_32.lib")
/*
Struct contain infomation about an account with specified username
*/
typedef struct accountinfo {
	char password[50];
	char status;
	long int seek_pos;	//offset from SEEK_SET of the account.txt file
} ACCOUNTINFO;

/*
Struct contain infomation about a client which connect to this server
*/
typedef struct client {
	SOCKET clientSocket;
	std::string ipAddr;
	u_short port;
} CLIENT;


// pointer to FILE contain account infomation
extern FILE* file;

// Map contains account info
extern std::unordered_map<std::string, ACCOUNTINFO> account_info;

/* Critical section for threads synchronization */
// accout_info.status can be modified by server
// protect account_info.status
extern CRITICAL_SECTION cs1;

// Single-linked-list to the SOCKET list
extern std::forward_list<CLIENT> client_list;

// Critical section for threads synchronization
// protect client_list
extern CRITICAL_SECTION cs2;


