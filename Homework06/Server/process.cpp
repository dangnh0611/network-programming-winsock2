#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"communicate.h"
using namespace std;

/* Process when client send a request */
int processRequest(int index) {
	CLIENT& client = client_list[index];
	static char buff[BUFF_SIZE];
	static int ret;
	static char ret_code;
	SOCKET clientSock = client.socket;
	//receive request
	ret = Receive(clientSock, buff, BUFF_SIZE);
	if (ret == -1) {
		return -1;
	}
	string request = buff;
	//processing request from client
	//if request is USER or user (login with specified username)
	if (request.substr(0, 5) == "USER " || request.substr(0, 5) == "user ") {
		//if client already login
		if (client.step == 2) {
			ret_code = 2;
			return Send(clientSock, &ret_code, 1);
		}
		client.username = buff + 5;
		try {
			client.acc = account_info.at(client.username);
			//if account had been locked
			if (client.acc.status == '0') {
				client.step = 0;
				ret_code = 3;
				return Send(clientSock, &ret_code, 1);
			}
			//if username exit and account hadn't been locked
			client.step = 1;
			ret_code = 0;
			return Send(clientSock, &ret_code, 1);
		}
		catch (out_of_range&) {
			//if username does not exit
			client.step = 0;
			ret_code = 1;
			return Send(clientSock, &ret_code, 1);
		}
	}
	else if (request.substr(0, 5) == "PASS " || request.substr(0, 5) == "pass ") {
		if (client.step == 0) {
			//if client hasn't entered valid username
			ret_code = 6;
			return Send(clientSock, &ret_code, 1);
		}
		//if client already login
		if (client.step == 2) {
			ret_code = 2;
			return Send(clientSock, &ret_code, 1);
		}
		//if step==1
		//if account had been locked
		if (account_info.at(client.username).status == '0') {
			ret_code = 3;
			return Send(clientSock, &ret_code, 1);
		}
		string password = buff + 5;
		//if password is correct
		if (password == client.acc.password) {
			client.step = 2;
			ret_code = 4;
			return Send(clientSock, &ret_code, 1);
		}
		//if password is incorrect
		else {
			if (++client.login_log[client.username] > 3) {
				//lock this account because of 3 times fail trying to login
				EnterCriticalSection(&cs1);
				account_info.at(client.username).status = '0';
				LeaveCriticalSection(&cs1);
				//Save instance state to file
				file = fopen("account.txt", "r+");
				fseek(file, client.acc.seek_pos, SEEK_SET);
				fputc('0', file);
				fclose(file);
				printf("Locked username %s\n", client.username.c_str());
				ret_code = 7;
			}
			else ret_code = 5;
			return Send(clientSock, &ret_code, 1);
		}
	}
	else if (request.substr(0, 6) == "LOGOUT" || request.substr(0, 6) == "logout") {
		//logout successfully
		if (client.step == 2) {
			client.step = 0;
			ret_code = 8;
			return Send(clientSock, &ret_code, 1);
		}
		//if client hasn't login
		else {
			ret_code = 9;
			return Send(clientSock, &ret_code, 1);
		}
	}
	//if request is invalid
	else {
		ret_code = 10;
		return Send(clientSock, &ret_code, 1);
	}
}

/*	Disconnect when client exit	*/
int disconnect(int index) {
	CLIENT& client = client_list[index];
	if (client.socket == 0)
		return 0;
	shutdown(client.socket, SD_SEND);
	closesocket(client.socket);
	printf("Close connection with [%s:%d]\n", client.ipAddr.c_str(), client.port);
	client.socket = 0;
	EnterCriticalSection(&cs2);
	nEvents--;
	LeaveCriticalSection(&cs2);
	return 0;
}