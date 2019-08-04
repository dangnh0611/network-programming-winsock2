#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib,"ws2_32.lib")
#include"communicate.h"
#include"main.h"
#include"process.h"
#include<direct.h>
u_short PORT_NUMBER;

int main(int argc, char** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 2) {
		printf("Error: expected 1 command argument, but %d were provided\n", argc - 1);
		printf("Usage:\n\tServer process request on a port number specified in argument.\n");
		printf("\tServer for Ceasar crypto system service.\n");
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
	_mkdir("temp");
	printf("NOTE: All temporary files will be stored in .\\temp\n");

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
	//Set non-blocking for listen socket
	//remove infinity blocking of accept() function
	unsigned long ul = 1;
	ioctlsocket(server_socket, FIONBIO, (unsigned long *)&ul);
	//Step 6: Start listening for request from client at the specified port in command argument
	printf("Listening at Port %d\n", PORT_NUMBER);
	if (listen(server_socket, 100)) {
		perror("ERROR: ");
		return 0;
	}
	printf("Server started.\n");
	SOCKADDR_IN clientAddr;
	int ret, clientAddrLen = sizeof(clientAddr);
	SOCKET connSock;
	CLIENT client[FD_SETSIZE];
	fd_set readfds, writefds, init_writefds, init_readfds; //use initfds to initiate readfds at the begining of every loop step

	for (int i = 0; i < FD_SETSIZE; i++) {
		client[i].socket = 0;	// 0 indicates available entry
	}
	FD_ZERO(&init_readfds);
	FD_ZERO(&init_writefds);
	FD_SET(server_socket, &init_readfds);
	int nEvents = 0;	//number of events
	//Step 7: Accept connection from clients
	ul = 0;
	while (1) {
		readfds = init_readfds;
		writefds = init_writefds;
		nEvents = select(0, &readfds, &writefds, 0, 0);
		if (nEvents < 0) {
			printf("Error! Cannot poll sockets: %d\n", WSAGetLastError());
			break;
		}

		//new client connection
		if (FD_ISSET(server_socket, &readfds)) {
			clientAddrLen = sizeof(clientAddr);
			if ((connSock = accept(server_socket, (SOCKADDR *)&clientAddr, &clientAddrLen)) < 0) {
				printf("\nError! Cannot accept new connection: %d", WSAGetLastError());
				break;
			}
			else {
				printf("Connect to client [%s:%d]\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); /* prints client's IP */
				//set client socket to be blocking, for using MSG_WAITALL flag
				ioctlsocket(connSock, FIONBIO, (unsigned long *)&ul);
				int i;
				for (i = 0; i < FD_SETSIZE; i++) {
					if (client[i].socket == 0) {
						client[i].socket = connSock;
						FD_SET(client[i].socket, &init_readfds);
						client[i].addr = clientAddr;
						break;
					}
				}
				if (i == FD_SETSIZE) {
					printf("Too many clients.\n");
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}
		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i].socket == 0)
				continue;
			if (FD_ISSET(client[i].socket, &readfds)) {
				ret = receiveProcess(&client[i]);
				if (ret == 0);
				else if (ret == 1) {	//finish receiving
					FD_CLR(client[i].socket, &init_readfds);
					FD_SET(client[i].socket, &init_writefds);
				}
				else {	//error occur,clean up and disconnect client
					FD_CLR(client[i].socket, &init_readfds);
					FD_CLR(client[i].socket, &init_writefds);
					closesocket(client[i].socket);
					client[i].socket = 0;
					fclose(client[i].file);
					remove(client[i].filename);
					printf("Close connection with client[%s:%d]\n", inet_ntoa(client[i].addr.sin_addr), ntohs(client[i].addr.sin_port));
				}
			}
			//Send data to client
			else if (FD_ISSET(client[i].socket, &writefds)) {
				ret = sendProcess(&client[i]);
				if (ret == 0);
				else if (ret == 1) {	//finish sending
					FD_CLR(client[i].socket, &init_writefds);
					FD_SET(client[i].socket, &init_readfds);
				}
				else {	//error occur, cleanup and disconnect client
					FD_CLR(client[i].socket, &init_readfds);
					FD_CLR(client[i].socket, &init_writefds);
					closesocket(client[i].socket);
					client[i].socket = 0;
					fclose(client[i].file);
					remove(client[i].filename);
					printf("Close connection with client[%s:%d]\n", inet_ntoa(client[i].addr.sin_addr), ntohs(client[i].addr.sin_port));
				}
			}
			if (--nEvents <= 0)
				continue; //no more event
		}
	}
	//cleanup
	closesocket(server_socket);
	WSACleanup();
	return 0;
}