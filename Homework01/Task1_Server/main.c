#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#pragma comment (lib,"ws2_32.lib")
#define BUFF_SIZE 1000

u_short PORT_NUMBER;
int main(int argc, char** argv) {
	//Step 1: Parsing arguments from command line
	if (argc != 2) {
		printf("Error: expected 1 command argument, but %d were provided\n",argc-1);
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
		PORT_NUMBER=(u_short)temp;
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
	char buff[BUFF_SIZE];
	//buffer for detaching 
	char alpha_buff[BUFF_SIZE];
	char digit_buff[BUFF_SIZE];
	SOCKADDR_IN clientAddress;
	int ret, addrlen = sizeof(clientAddress);
	//Infinitiy loop for listening
	while (1) {
		ret = recvfrom(server_socket, buff, BUFF_SIZE, 0, (SOCKADDR*)(&clientAddress), &addrlen);
		if (ret == SOCKET_ERROR) {
			printf("ERROR %d\n", WSAGetLastError());
		}
		else {
			printf("[%s:%d]:%s\n", inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port, buff);
			//Processing
			//Detach string
				/*
				state variable stores the result code number of detaching the string:

					-1 :the string contains special characters e.g "dang6@11!"
					 0 :the string contains only alpha characters e.g "dangnh"
					 1 :the string contains only digits e.g "123456" 
					 2 :the string contains almost 2 type alpha and digit e.g "dang6111998"
				*/
			int state = 2;
			int alpha_index=0, digit_index = 0;
			for (int i = 0; i < strlen(buff); i++) {
				if (isalpha(buff[i]))
					alpha_buff[alpha_index++] = buff[i];
				else if (isdigit(buff[i]))
					digit_buff[digit_index++] = buff[i];
				else {
					state = -1;
					break;
				} 
			}
			if (state != -1) {
				if (alpha_index == 0) state = 1;
				if (digit_index == 0) state = 0;
			}

			//Send the state number to client
			ret = sendto(server_socket, (const char*)&state, sizeof(state), 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
			if (ret == SOCKET_ERROR) {
				printf("ERROR %d\n", WSAGetLastError());
			}

			//Case on the state number in {-1,0,1,2}
			switch (state) {
				//if the string contains special characters e.g "dang6@11!"
			case -1:
				continue;
				break;
			//if the string contains only alpha characters e.g "dangnh"
			case 0:
				alpha_buff[alpha_index++] = '\0';
				ret = sendto(server_socket, alpha_buff, alpha_index, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
				break;
			//if the string contains only digits e.g "123456" 
			case 1:
				digit_buff[digit_index++] = '\0';
				ret = sendto(server_socket, digit_buff, digit_index, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
				break;
			//if the string contains almost 2 type alpha and digit e.g "dang6111998"
			case 2:
				alpha_buff[alpha_index++] = '\0';
				ret = sendto(server_socket, alpha_buff, alpha_index, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}

				digit_buff[digit_index++] = '\0';
				ret = sendto(server_socket, digit_buff, digit_index, 0, (SOCKADDR*)(&clientAddress), sizeof(clientAddress));
				if (ret == SOCKET_ERROR) {
					printf("ERROR %d\n", WSAGetLastError());
				}
				break;
			}
		}
	}
	
	//Step 5: Close socket and cleanup for exiting
	closesocket(server_socket);
	WSACleanup();
	return 0;
}