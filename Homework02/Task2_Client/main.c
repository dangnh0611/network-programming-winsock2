#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#include"communicate.h"
#pragma comment(lib,"ws2_32.lib")

u_short SERVER_PORT;
char* SERVER_IP;
int main(int argc, char** argv) {

	//Step 1: Parsing arguments from command line
	if (argc != 3) {
		printf("Error: expected 2 command arguments, but %d were provided\n", argc - 1);
		printf("Usage:\n\tConnect to server with specified IP address and port number in argument.\n");
		printf("\tDetach string into alpha-only string and digit-only string.\n");
		printf("%s <server_ip> <server_port>\n", argv[0]);
		return -1;
	}
	else {
		char* ptr_end;
		long temp = strtol(argv[2], &ptr_end, 0);
		if (temp<0 || temp>USHRT_MAX || ptr_end - argv[2] < strlen(argv[2])) {
			printf("Error: invalid command argument: <server_port>\n");
			return -1;
		}

		if (inet_addr(argv[1]) == INADDR_NONE) {
			printf("Error: invalid command argument: <server_ip>\n");
			return -1;
		}
		SERVER_PORT = (u_short)temp;
		SERVER_IP = argv[1];
	}

	//Step 2: Start Winsock application
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");
	printf("Client started!\n");

	//Step 3: Construct socket
	SOCKET client_socket;
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Set time-out interval
	int tv = 10000; //Time-out interval: 10000ms
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//Step 4: Specifying server address
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	printf("Connected to server[%s:%d]\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	//Step 5: Communicate with server
	char buff[NI_MAXHOST + 2];
	int ret, serverAddrLen = sizeof(server_addr);
	while (1) {
		printf(">>>");
		gets_s(buff, NI_MAXHOST);
		//if user enters an empty string, exit
		if (strlen(buff) == 0) {
			ret = Send(client_socket, "", 1);
			if (ret == -1) {
				WSACleanup();
				return -1;
			}
			printf("\nClient stopped!\n");
			break;
		}
		ret = Send(client_socket, buff, strlen(buff) + 1);
		if (ret == -1) {
			WSACleanup();
			return -1;
		}
		//Receive result code number
		ret = Receive(client_socket, buff, NI_MAXHOST);
		if (ret == -1) {
			WSACleanup();
			return -1;
		}
		else {
			int state = *(int*)buff;
			//Process base on the result code in {0,1,2,3}
			if (state == 0) printf("IP Address is invalid\n");
			else if (state == 1) {
				printf("Not found information\n");
			}
			//if found any info
			else {
				ret = Receive(client_socket, buff, NI_MAXHOST);
				if (ret == -1) {
					WSACleanup();
					return -1;
				}
				if (state == 2) {
					printf("Offical name: %s\n", buff);
					printf("Alias name:\n");
				}
				else {
					printf("Offical IP: %s\n", buff);
					printf("Alias IP:\n");
				}
				while (1) {
					ret = Receive(client_socket, buff, NI_MAXHOST);
					if (ret == -1) {
						WSACleanup();
						return -1;
					}
					//receive from server until an empty string is sent
					//indicate that info was totally sent
					if (buff[0] == '\0') break;
					else printf("\t%s\n", buff);
				}
			}
		}
	}

	//Step 6: Close socket and clean up for exiting
	shutdown(client_socket, SD_SEND);
	closesocket(client_socket);
	WSACleanup();
	return 0;
}
