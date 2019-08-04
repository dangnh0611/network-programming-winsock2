#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"workerThread.h"
#include"process.h"
#include"communicate.h"
using namespace std;

/*###################################################################
#	EVENT SELECT TCP SERVER WITH MULTITHREADING						#
#	ADDITIONAL THREADS: 4											#
#	MAXIMUM NUMBER OF CLIENTS:64*5-1=319							#
#	MAIN THREAD ACCEPT REQUEST AND CAN PROCESS MAXIMUM 63 CLIENTS	#
#	EACH OTHER THREAD CAN PROCESS MAXIMUM 64 CLIENTS				#
###################################################################*/


/*##############################***PROTOCOL***#######################################
#	client: username																#
#	->server: *0-username exits and user is not blocked								#
#			*1-username doesn't exit												#
#			*2-client already login at that time, first, log out before re-login	#
#			*3-user was locked before												#
#	client: password																#
#	->server: *2-client already login at that time, first, log out before re-login	#
#			*3-user was locked before												#
#			*4-password is correct, login successfully								#
#			*5-password is incorrect												#
#			*6-client must first enter username										#
#			*7-user is locked because client enter wrong password over 3 times		#
#	client: logout																	#
#	->server: *8-logout successfully												#
#			*9-user hasn't been login												#
#	client: invalid request															#
#	->server: *10-invalid request													#
###################################################################################*/

SOCKET listenSock;
u_short PORT_NUMBER;
unordered_map<string, ACCOUNTINFO> account_info;
CLIENT client_list[MAX_EVENTS];
WSAEVENT events[MAX_EVENTS];
int nEvents = 0;
CRITICAL_SECTION cs1;
CRITICAL_SECTION cs2;
FILE* file;

int main(int argc, char ** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 2) {
		printf("Error: expected 1 command argument, but %d were provided\n", argc - 1);
		printf("Usage:\n\tServer process request on a port number specified in argument.\n");
		printf("\tServer login services.\n");
		printf("%s <server_port>\n", argv[0]);
		return -1;
	}
	else {
		char* ptr_end;
		long temp = strtol(argv[1], &ptr_end, 0);
		if (temp<0 || temp>USHRT_MAX || (size_t)(ptr_end - argv[1]) < strlen(argv[1])) {
			printf("Error: invalid command argument <server_port>\n");
			return -1;
		}
		PORT_NUMBER = (u_short)temp;
	}
	printf("LOGIN SERVER, ADDITIONAL THREAD: 4, MAXIMUM CLIENTS: 319.\n");
	/*Step 2:Read account infomations from file	*/
	printf("Account infomation stored in ./account.txt\n");
	file = fopen("account.txt", "r+");
	if (!file) {
		printf("Can't open \"account.txt\" to read account infomations!\n");
		return -1;
	}
	else printf("Open \"account.txt\" successfully!\n");
	printf("Reading accounts infomation.\n");
	ACCOUNTINFO acc;
	string username;
	char temp[50];
	while (1) {
		if (fscanf(file, "%s%s %c", temp, acc.password, &acc.status) == 3) {
			printf("User: %s\n", temp);
			printf("\tpass: %s\n", acc.password);
			printf("\tstatus: %c\n", acc.status);
			username = temp;
			acc.seek_pos = ftell(file) - 1;
			account_info.insert(pair<string, ACCOUNTINFO >(username, acc));
		}
		else break;
	}
	fclose(file);

	/*Step 3: Start Winsock application*/
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");

	/*Step 4: Construct Socket*/
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Bind the socket
	SOCKADDR_IN serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUMBER);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenSock, (SOCKADDR*)(&serverAddress), sizeof(serverAddress))) {
		printf("ERROR in binding socket\n");
		WSACleanup();
		return 0;
	}

	/*Step 5: Start listening for requests*/
	printf("Listening at Port %d\n", PORT_NUMBER);
	if (listen(listenSock, 1024)) {
		perror("ERROR: ");
		return 0;
	}

	InitializeCriticalSection(&cs1);
	InitializeCriticalSection(&cs2);

	//Initialize clients array and events array
	for (int i = 0; i < MAX_EVENTS; i++) {
		client_list[i].socket = 0;
		events[i] = WSACreateEvent();
	}
	client_list[0].socket = listenSock;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(client_list[0].socket, events[0], FD_ACCEPT | FD_CLOSE);
	EnterCriticalSection(&cs2);
	nEvents = 1;
	LeaveCriticalSection(&cs2);

	/* Step 6: Create worker threads */
	//index array to pass pointer to thread
	int thread_index[N_THREADS];
	for (int i = 0; i < N_THREADS; i++) {
		thread_index[i] = i + 1;
	}
	//create and pass argument to thread to indicate thread's index
	for (int i = 0; i < N_THREADS; i++) {
		_beginthreadex(0, 0, WorkerThread, (void *)(thread_index + i), 0, 0);
	}

	printf("Server started.\n");
	/*Step 7: Loop for accept new connections*/
	char hello_msg[] = "Hello Client\n";
	int hello_msg_len = strlen(hello_msg) + 1;
	WSANETWORKEVENTS sockEvent;
	SOCKET connSock;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int index = 0, ret = 0;
	while (1) {
		//wait for network events on first 64 sockets
		index = WSAWaitForMultipleEvents(WSA_MAXIMUM_WAIT_EVENTS, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
			break;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(client_list[index].socket, events[index], &sockEvent);

		// if event is accepting
		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				printf("FD_ACCEPT failed with error %d\n", sockEvent.iErrorCode[FD_ACCEPT_BIT]);
				disconnect(index);
				continue;
			}
			connSock = accept(listenSock, (SOCKADDR *)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET)
				continue;
			//check if server is full
			if (nEvents == MAX_EVENTS) {
				printf("Too much clients!\n");
				continue;
			}

			//initialize client info and status
			CLIENT client;
			client.socket = connSock;
			client.ipAddr = inet_ntoa(clientAddr.sin_addr);
			client.port = ntohs(clientAddr.sin_port);
			client.step = 0;
			client.username = "";
			printf("Connected to client [%s:%d]\n", client.ipAddr.c_str(), client.port);
			ret = Send(connSock, hello_msg, hello_msg_len);
			//if there is an error, disconnect
			if (ret == -1) {
				disconnect(index);
				continue;
			}
			//add client to client_list
			for (int i = 1; i < MAX_EVENTS; i++) {
				if (client_list[i].socket == 0) {
					client_list[i] = client;
					WSAResetEvent(events[i]);
					WSAEventSelect(client_list[i].socket, events[i], FD_READ | FD_CLOSE);
					EnterCriticalSection(&cs2);
					nEvents++;
					LeaveCriticalSection(&cs2);
					break;
				}
			}
		}

		//if event is reading
		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				disconnect(index);
				continue;
			}
			//if there is an error, disconnect
			ret = processRequest(index);
			if (ret == -1) {
				disconnect(index);
			}
			WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				disconnect(index);
				continue;
			}
			//Release socket and event
			disconnect(index);
		}
	}

	//Step 8: Cleanup
	DeleteCriticalSection(&cs1);
	DeleteCriticalSection(&cs2);
	WSACleanup();
	return 0;
}