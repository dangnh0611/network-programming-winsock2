#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"communicate.h"
#include"threads.h"
using namespace std;
#define BUFF_SIZE 1024

//As server terminated, all accounts infomation will be saved
/*client: username
	server: *0-username exits and user is not blocked
			*1-username doesn't exit
			*2-client already login at that time, first, log out before re-login
			*3-user was locked before
  client: password
	server: *2-client already login at that time, first, log out before re-login
			*3-user was locked before
			*4-password is correct, login successfully
			*5-password is incorrect
			*6-client must first enter username
			*7-user is locked because client enter wrong password over 3 times
  client: logout
	server: *8-logout successfully
	server: *9-user hasn't been login
  client: invalid request
	server: *10-invalid request
*/
u_short PORT_NUMBER;
unordered_map<std::string, ACCOUNTINFO> account_info;
CRITICAL_SECTION cs1;
forward_list<CLIENT> client_list;
CRITICAL_SECTION cs2;
FILE* file;

int main(int argc, char** argv) {
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
		if (temp<0 || temp>USHRT_MAX || ptr_end - argv[1] < strlen(argv[1])) {
			printf("Error: invalid command argument <server_port>\n");
			return -1;
		}
		PORT_NUMBER = (u_short)temp;
	}

	//Step 2:Read account infomations from file	
	printf("Account infomation stored in ./account.txt\n");
	InitializeCriticalSection(&cs1);
	file = fopen("account.txt", "r+");
	if (!file) {
		printf("Can't open \"account.txt\" to read account infomations!\n");
		return -1;
	}
	else printf("Open \"account.txt\" successfully!\n");
	printf("Reading accounts infomation.\n");
	//Step 3: Read account infomation from file
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

	//Step 4: Start Winsock application
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");

	//Step 5: Construct Socket
	SOCKET server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Bind the socket
	SOCKADDR_IN serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUMBER);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(server_socket, (SOCKADDR*)(&serverAddress), sizeof(serverAddress))) {
		printf("ERROR in binding socket\n");
		WSACleanup();
		return 0;
	}
	//Step 6: Start listening for request from client at the specified port in command argument
	printf("Listening at Port %d\n", PORT_NUMBER);
	if (listen(server_socket, 100)) {
		perror("ERROR: ");
		return 0;
	}
	printf("Server started.\n");
	InitializeCriticalSection(&cs2);
	CLIENT client;
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	//Step 7: Accept connection from clients
	//Create worker threads to process requests from each clients
	while (1) {
		client.clientSocket = accept(server_socket, (SOCKADDR *)&clientAddr,
			&clientAddrLen);
		client.ipAddr = inet_ntoa(clientAddr.sin_addr);
		client.port = ntohs(clientAddr.sin_port);
		printf("Connect to client [%s:%d]\n", client.ipAddr.c_str(), client.port);
		//enter critical section to update the client list
		EnterCriticalSection(&cs2);
		client_list.push_front(client);
		LeaveCriticalSection(&cs2);
		//start a new worker thread
		_beginthreadex(0, 0, WorkerThread, (void *)&client, 0, 0);
	}
	//Step 8: clean up, exit
	DeleteCriticalSection(&cs1);
	DeleteCriticalSection(&cs2);
	fclose(file);
	WSACleanup();
	return 0;
}