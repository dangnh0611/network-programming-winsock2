#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"threads.h"
#include"communicate.h"
#include"main.h"
using namespace std;

#define BUFF_SIZE 100

unsigned __stdcall WorkerThread(void *param) {
	CLIENT client = *(CLIENT*)param;
	SOCKET connectedSocket = client.clientSocket;
	char* buff = (char*)malloc(BUFF_SIZE);
	//save the number of times client attempt to login inlegally
	unordered_map<string, char> login_log;
	char ret_code = 0;
	string username = "", password = "";
	//step indicated status of session
	//0: unidentified- username isn't invalid
	//1: unauthenticated -password is not verified
	//2: authenticated -username and password are verified, in logged in
	int step = 0;
	ACCOUNTINFO acc;

	int ret;
	while (1) {
		//receive request
		ret = Receive(connectedSocket, buff, BUFF_SIZE);
		if (ret == -1) {
			break;
		}
		string request = buff;
		//processing request from client
		//if request is USER or user (login with specified username)
		if (request.substr(0, 5) == "USER " || request.substr(0, 5) == "user ") {
			//if client already login
			if (step == 2) {
				ret_code = 2;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			username = buff + 5;
			try {
				EnterCriticalSection(&cs1);
				acc = account_info.at(username);
				//if account had been locked
				if (acc.status == '0') {
					step = 0;
					ret_code = 3;
					Send(connectedSocket, &ret_code, 1);
					LeaveCriticalSection(&cs1);
					continue;
				}
				else LeaveCriticalSection(&cs1);
				//if username exit and account hadn't been locked
				step = 1;
				ret_code = 0;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			catch (out_of_range&) {
				//if username does not exit
				step = 0;
				ret_code = 1;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
		}
		else if (request.substr(0, 5) == "PASS " || request.substr(0, 5) == "pass ") {
			if (step == 0) {
				//if client hasn't entered valid username
				ret_code = 6;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			//if client already login
			if (step == 2) {
				ret_code = 2;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			//if step==1
			//if account had been locked
			EnterCriticalSection(&cs1);
			if (account_info.at(username).status == '0') {
				ret_code = 3;
				Send(connectedSocket, &ret_code, 1);
				LeaveCriticalSection(&cs1);
				continue;
			}
			else LeaveCriticalSection(&cs1);
			password = buff + 5;
			//if password is correct
			if (password == acc.password) {
				step = 2;
				ret_code = 4;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			//if password is incorrect
			else {
				if (++login_log[username] > 3) {
					EnterCriticalSection(&cs1);
					//lock this account because of 3 times fail trying to login
					account_info.at(username).status = '0';
					//Save instance state to file
					fseek(file, acc.seek_pos, SEEK_SET);
					fputc('0', file);
					printf("Locked username %s\n", username.c_str());
					LeaveCriticalSection(&cs1);
					ret_code = 7;
				}
				else ret_code = 5;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
		}
		else if (request.substr(0, 6) == "LOGOUT" || request.substr(0, 6) == "logout") {
			//logout successfully
			if (step == 2) {
				step = 0;
				ret_code = 8;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
			//if client hasn't login
			else {
				ret_code = 9;
				Send(connectedSocket, &ret_code, 1);
				continue;
			}
		}
		//if request is invalid
		else {
			ret_code = 10;
			Send(connectedSocket, &ret_code, 1);
			continue;
		}
	}
	shutdown(connectedSocket, SD_SEND);

	EnterCriticalSection(&cs2);
	client_list.remove(client);
	LeaveCriticalSection(&cs2);

	printf("Close connection with [%s:%d]\n", client.ipAddr.c_str(), client.port);
	closesocket(connectedSocket);
	free(buff); //free memory
	return 0;
}