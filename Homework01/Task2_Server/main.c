#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#include<Ws2tcpip.h>
#include"validate.h"
#pragma comment (lib,"ws2_32.lib")

u_short PORT_NUMBER;

int main(int argc, char** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 2) {
		printf("Error: expected 1 command argument, but %d were provided\n", argc - 1);
		printf("Usage:\n\tServer process request on a port number specified in argument.\n");
		printf("\tResolving hostname from IP address and vice versa.\n");
		printf("%s <server_port>\n", argv[0]);
		return -1;
	}
	else {
		char* ptr_end;
		long temp = strtol(argv[1], &ptr_end, 0);
		if (temp<0 || temp>USHRT_MAX || ptr_end - argv[1] < strlen(argv[1])) {
			printf("Error: invalid command argument: <server_port>\n");
			return -1;
		}
		PORT_NUMBER = (u_short)temp;
	}

	//Step 2: Start Winsock application
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");
	printf("Server started!\n");

	//Step 3: Construct Socket
	SOCKET server_socket;
	server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
	printf("Listening at Port %d\n", PORT_NUMBER);

	//Step 4: Processing request from client
	char buff[NI_MAXHOST];
	SOCKADDR_IN clientAddress;
	int ret, addrlen = sizeof(clientAddress);
	//Infinitiy loop for listening
	while (1) {
		ret = recvfrom(server_socket, buff, NI_MAXHOST, 0, (SOCKADDR*)(&clientAddress), &addrlen);
		if (ret == SOCKET_ERROR) {
			printf("ERROR %d\n", WSAGetLastError());
		}
		else {
			printf("[%s:%d]:%s\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port, buff);
		}

		HOSTENT * host_info = NULL;	//Store info about the host
		unsigned long ipv4_addr = inet_addr(buff);
		int state = 0;

		//if the client sent a string that is an IPv4 address
		if (isIpv4Form(buff)) {
			//if the addr is invalid,send error code to client as result
			if (ipv4_addr == INADDR_NONE) {
				state = 0;
				//send result code to client
				ret = sendto(server_socket, (const char*)&state, sizeof(state), 0,
					(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
			}
			//else if can't find info about address
			else {
				host_info = gethostbyaddr(&ipv4_addr, sizeof(ipv4_addr), AF_INET);
				if (host_info == NULL) {
					state = 1;
					ret = sendto(server_socket, (const char*)&state, sizeof(state), 0,
						(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
					if (ret == SOCKET_ERROR) {
						printf("ERROR %d\n", WSAGetLastError());
					}
				}
				//else if found info about the addr
				else {
					state = 2;
					ret = sendto(server_socket, (const char*)&state, sizeof(state), 0,
						(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
					if (ret == SOCKET_ERROR) {
						printf("ERROR %d\n", WSAGetLastError());
					}
					ret = sendto(server_socket, host_info->h_name, strlen(host_info->h_name)+1, 0,
						(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
					if (ret == SOCKET_ERROR) {
						printf("ERROR %d\n", WSAGetLastError());
					}
					for (int i = 0; host_info->h_aliases[i] != NULL; i++) {
						ret = sendto(server_socket, host_info->h_aliases[i], strlen(host_info->h_aliases[i])+1, 0,
							(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
						if (ret == SOCKET_ERROR) {
							printf("ERROR %d\n", WSAGetLastError());
						}
					}
					//send an empty string to client to indicate that 
					//the result has been sent succesfully
					ret = sendto(server_socket, "", 1, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
					if (ret == SOCKET_ERROR) {
						printf("ERROR %d\n", WSAGetLastError());
					}
				}
			}
		}
		//if the client sent a string that is a host name
		else {
			host_info = gethostbyname(buff);
			//if can't find info about the name
			if (host_info == NULL) {
				state = 1;
				ret = sendto(server_socket, (const char*)&state, sizeof(state), 0,
					(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
			}
			//else if found info about the name
			else {
				state = 3;
				ret = sendto(server_socket, (const char*)&state, sizeof(state), 0,
					(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
				for (int i = 0; host_info->h_addr_list[i] != NULL; i++) {
					IN_ADDR addr;
					addr.s_addr = *(unsigned long*)(host_info->h_addr_list[i]);
					ret = sendto(server_socket, inet_ntoa(addr), strlen(inet_ntoa(addr))+1, 0,
						(SOCKADDR*)(&clientAddress), sizeof(clientAddress));
					if (ret == SOCKET_ERROR) {
						printf("ERROR %d\n", WSAGetLastError());
					}
				}
				//send an empty string to client to indicate that 
				//the result has been sent succesfully
				ret = sendto(server_socket, "", 1, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
			}
		}
	}
	//Close socket and cleanup for exiting
	closesocket(server_socket);
	WSACleanup();
	return 0;
}