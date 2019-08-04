#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#define BUFF_SIZE 1000

u_short SERVER_PORT;
char* SERVER_IP;

int main(int argc, char** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 3) {
		printf("Error: expected 2 command arguments, but %d were provided\n", argc - 1);
		printf("Usage:\n\tConnect to server with specified IP address and port number in argument.\n");
		printf("\tResolving hostname from IP address and vice versa.\n");
		printf("%s <server_ip> <server_port>\n",argv[0]);
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
	client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//Set time-out interval
	int tv = 10000; //Time-out interval: 10000ms
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//Step 4: Specifying server address
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	printf("Connected to server[%s:%d]\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

	//Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(server_addr);

	while (1) {
		printf(">>>");
		gets_s(buff, BUFF_SIZE);
		//if user enters an empty string, exit
		if (strlen(buff) == 0) {
			printf("\nClient stopped!\n");
			break;
		}
		//Send the inputed string to server
		ret = sendto(client_socket, buff, strlen(buff) + 1, 0, (SOCKADDR*)&server_addr, serverAddrLen);
		if (ret == SOCKET_ERROR) {
			printf("Can't send INPUT to server!\n");
			continue;
		}
		//Receive result code number
		ret = recvfrom(client_socket, buff, BUFF_SIZE, 0, (SOCKADDR*)&server_addr, &serverAddrLen);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT) printf("Time-out error!");
			else printf("Can't not receive message!\n");
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
				ret = recvfrom(client_socket, buff, BUFF_SIZE, 0, (SOCKADDR*)&server_addr, &serverAddrLen);
				if (ret == SOCKET_ERROR) {
					if (WSAGetLastError() == WSAETIMEDOUT) printf("Time-out error!");
					else printf("Can't not receive message!\n");
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
					ret = recvfrom(client_socket, buff, BUFF_SIZE, 0, (SOCKADDR*)&server_addr, &serverAddrLen);
					if (ret == SOCKET_ERROR) {
						if (WSAGetLastError() == WSAETIMEDOUT) printf("Time-out error!");
						else printf("Can't not receive message!\n");
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
	closesocket(client_socket);
	WSACleanup();
	return 0;
}