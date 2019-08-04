#pragma once
#include<WinSock2.h>
#include<stdio.h>
#include<Windows.h>
#include<string>
#include<unordered_map>
#pragma comment(lib,"ws2_32.lib")

#define BUFF_SIZE 1024

// pointer to FILE contain account infomation
extern FILE* file;

/*Struct stores acccount infomations*/
typedef struct accountinfo {
	char password[50];
	char status;
	long int seek_pos;	//offset from SEEK_SET of the account.txt file
} ACCOUNTINFO, *LPACCOUNTINFO;

// map contains account info
extern std::unordered_map<std::string, ACCOUNTINFO> account_info;

//protect account_info
extern CRITICAL_SECTION cs1;

/* Operation enum type that
   indicate the state of an IO operation */
enum OPERATION {
	GET_MSG_LEN,	// last completion IO operation is to receive client request's msg length
	RECEIVE_MSG,	//last completion IO operation is to receive client request
	SEND_MSG		//last completion IO opeation is to send reply to client
};

/* Step enum type that
   indicate the steps of a session
*/
enum STEP {
	UNIDENTIFIED,		//username is invalid or not provided
	UNAUTHENTICATED,	//username is valid, but password is not verified
	AUTHENTICATED		//username and password are verified, in logged in
};

/*Struct stores infomation about a client which connect to this server*/
typedef struct per_client_data {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuff;
	CHAR buffer[BUFF_SIZE];
	const int bufLen = BUFF_SIZE;	//per client buffer's length
	OPERATION operation;	//last IO operation type

	//save the number of times client attempt to login inlegally
	std::unordered_map<std::string, char> login_log;
	STEP step;
	std::string username;	//current username
	ACCOUNTINFO acc;	//current account with username
} PER_CLIENT_DATA, *LPPER_CLIENT_DATA;

typedef struct client_info {
	SOCKET socket;
	u_short port;
	std::string ipAddr;
} CLIENT_INFO, PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

//client list
extern std::unordered_map<LPPER_HANDLE_DATA, LPPER_CLIENT_DATA> client_list;

