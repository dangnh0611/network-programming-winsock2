#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"communicate.h"
using namespace std;

/* Process when client send a request */
u_char processRequest(LPPER_CLIENT_DATA lpClientData) {
	string request = lpClientData->buffer;
	//if request is USER or user (login with specified username)
	if (request.substr(0, 5) == "USER " || request.substr(0, 5) == "user ") {
		//if client already login
		if (lpClientData->step == AUTHENTICATED) {
			return 2;
		}
		lpClientData->username = request.substr(5);
		try {
			lpClientData->acc = account_info.at(lpClientData->username);
			//if account had been locked
			if (lpClientData->acc.status == '0') {
				lpClientData->step = UNIDENTIFIED;
				return 3;
			}
			//if username exit and account hadn't been locked
			lpClientData->step = UNAUTHENTICATED;
			return 0;
		}
		catch (out_of_range&) {
			//if username does not exit
			lpClientData->step = UNIDENTIFIED;
			return 1;
		}
	}
	else if (request.substr(0, 5) == "PASS " || request.substr(0, 5) == "pass ") {
		if (lpClientData->step == UNIDENTIFIED) {
			//if client hasn't entered valid username
			return 6;
		}
		//if client already login
		if (lpClientData->step == AUTHENTICATED) {
			return 2;
		}
		//if step==1
		//if account had been locked
		if (account_info.at(lpClientData->username).status == '0') {
			return 3;
		}
		string password = request.substr(5);
		//if password is correct
		if (password == lpClientData->acc.password) {
			lpClientData->step = AUTHENTICATED;
			return 4;
		}
		//if password is incorrect
		else {
			if (++(lpClientData->login_log[lpClientData->username]) > 3) {
				//lock this account because of 3 times fail trying to login
				EnterCriticalSection(&cs1);
				account_info.at(lpClientData->username).status = '0';
				LeaveCriticalSection(&cs1);
				//Save instance state to file
				file = fopen("account.txt", "r+");
				fseek(file, lpClientData->acc.seek_pos, SEEK_SET);
				fputc('0', file);
				fclose(file);
				printf("Locked username %s\n", lpClientData->username.c_str());
				return 7;
			}
			else 
				return 5;
		}
	}
	else if (request.substr(0, 6) == "LOGOUT" || request.substr(0, 6) == "logout") {
		//logout successfully
		if (lpClientData->step == AUTHENTICATED) {
			lpClientData->step = UNIDENTIFIED;
			return 8;
		}
		//if client hasn't login
		else {
			return 9;
		}
	}
	//if request is invalid
	else {
		return 10;
	}
}

/*	Disconnect when client exit	*/
void disconnect(LPPER_HANDLE_DATA lpHandleData) {
	shutdown(lpHandleData->socket, SD_SEND);
	closesocket(lpHandleData->socket);
	printf("Close connection with [%s:%d]\n", lpHandleData->ipAddr.c_str(), lpHandleData->port);
	try {
		//free dynamic allocated memory
		free(client_list.at(lpHandleData));
		free(lpHandleData);
		client_list.erase(lpHandleData);
	}
	catch (out_of_range&) {}
}