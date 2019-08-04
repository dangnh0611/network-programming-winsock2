#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#include"communicate.h"
#include"process.h"
#pragma comment (lib,"ws2_32.lib")
#define BUFF_SIZE 1000

u_short PORT_NUMBER;
int main(int argc, char** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 2) {
		printf("Error: expected 1 command argument, but %d were provided\n", argc - 1);
		printf("Usage:\n\tServer process request on a port number specified in argument.\n");
		printf("\tDetach string into alpha-only string and digit-only string.\n");
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

	//Step 2: Start Winsock application
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");

	//Step 3: Construct Socket
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
	//Step 4: Start listening for request from client at the specified port in command argument
	printf("Listening at Port %d\n", PORT_NUMBER);
	if (listen(server_socket, 10)) {
		perror("ERROR: ");
		return 0;
	}
	printf("Server started!\n");
	SOCKADDR_IN clientAddress;
	char buff[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddress);
	char error_msg[] = "Error: Input string contains non-digit character!";

	//Infinity loop for listening for request from client
	//This case, server can only serve only 1 client at a time
	while (1) {
		//Step 5: Create a TCP connection between server and client
		SOCKET connSock;
		printf("Waiting for connection request..");
		connSock = accept(server_socket, (SOCKADDR*)&clientAddress, &clientAddrLen);
		printf("\nConnect to client[%s:%d]\n",
			inet_ntoa(clientAddress.sin_addr),
			ntohs(clientAddress.sin_port));
		//Step 6: Loop for communicating with a client
		while (1) {
			ret = Receive(connSock, buff, BUFF_SIZE);
			if (ret == -1) {
				break;
			}
			//if client send an empty string, end connection
			if (strlen(buff) == 0) break;
			printf("%s\n", buff);
			ret = NSLookup(connSock, buff);
			if (ret == -1) {
				printf("Unexpected error!\n");
				break;
			}
		}
		//Step 7: Shutdown connection and close socket
		printf("Connection closed!\n");
		shutdown(connSock, SD_SEND);
		closesocket(connSock);
	}
	//close socket and cleanup for exit
	closesocket(server_socket);
	WSACleanup();
	return 0;
}