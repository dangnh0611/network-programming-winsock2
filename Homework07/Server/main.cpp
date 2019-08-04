#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"workerThread.h"
#include"process.h"
#include"communicate.h"
using namespace std;


/* COMPLETION PORT IO SERVER
						 MESSAGE STRUCTURE
+--------------------------------+--------------------------------------------------+
| HEADER (2 bytes): msg's length |				 BODY: msg's body				    |
+--------------------------------+--------------------------------------------------+
*/

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

//global variables
u_short PORT_NUMBER;
unordered_map<string, ACCOUNTINFO> account_info;
unordered_map<LPPER_HANDLE_DATA, LPPER_CLIENT_DATA> client_list;
CRITICAL_SECTION cs1;
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

	/*Step 4: Construct Socket and Completion Port */
	SOCKET listenSock;
	//create an overlapped socket
	if ((listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("WSASocket() failed with error %d\n", WSAGetLastError());
		return 0;
	}
	//create new completion port
	HANDLE completionPort;
	if ((completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) {
		printf("Initialize CreateIoCompletionPort() failed with error %d\n", GetLastError());
		return -1;
	}
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


	/*Step 6: Create workder threads */
	SYSTEM_INFO systemInfo;
	GetSystemInfo(&systemInfo);
	//number of additional threads is equal to the number of system processers
	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors; i++) {
		_beginthreadex(0, 0, WorkerThread, (void *)completionPort, 0, 0);
	}

	printf("Server started.\n");
	/*Step 7: Loop for accept new connections */

	char hello_msg[] = "Hello Client\n";
	int hello_msg_len = strlen(hello_msg) + 1;
	SOCKET connSock;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	int index = 0, ret = 0;
	while (1) {
		//accept new connection
		if ((connSock = WSAAccept(listenSock, (SOCKADDR*)&clientAddr, &clientAddrLen, NULL, 0)) == SOCKET_ERROR) {
			printf("WSAAccept() failed with error %d\n", WSAGetLastError());
			return -1;
		}

		//initialize per-handle data
		LPPER_HANDLE_DATA lpHandleData = new PER_HANDLE_DATA;
		lpHandleData->socket = connSock;
		lpHandleData->ipAddr = inet_ntoa(clientAddr.sin_addr);
		lpHandleData->port = ntohs(clientAddr.sin_port);

		//initialize per-client-data
		//each client has it's own PER_CLIENT_DATA structure to control it's own IO operation
		LPPER_CLIENT_DATA lpClientData = new PER_CLIENT_DATA;
		lpClientData->dataBuff.buf = lpClientData->buffer;
		lpClientData->dataBuff.len = hello_msg_len + 2;
		lpClientData->buffer[0] = hello_msg_len / 256;
		lpClientData->buffer[1] = hello_msg_len % 256;
		strcpy(lpClientData->buffer + 2, hello_msg);
		lpClientData->operation = SEND_MSG;
		lpClientData->step = UNIDENTIFIED;
		lpClientData->username = "";
		//add client to client_list
		client_list.insert(pair<LPPER_HANDLE_DATA, LPPER_CLIENT_DATA>(lpHandleData, lpClientData));
		//Associate accepted socket to the completion port
		if (CreateIoCompletionPort((HANDLE)connSock, completionPort, (ULONG_PTR)lpHandleData, 0) == NULL) {
			printf("CreateIoCompletionPort() failed with error %d\n", GetLastError());
			return 1;
		}
		printf("Connected to client [%s:%d]\n", lpHandleData->ipAddr.c_str(), lpHandleData->port);
		//send Hello msg to client
		ret = Send(connSock, &lpClientData->dataBuff, NULL, &lpClientData->overlapped);
		//if there is an error, disconnect
		if (ret == -1) {
			disconnect(lpHandleData);
			continue;
		}
	}

	//Step 8: Cleanup
	DeleteCriticalSection(&cs1);
	WSACleanup();
	return 0;
}