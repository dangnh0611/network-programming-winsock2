#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"communicate.h"
#define BUFF_SIZE 1024;

int main(int argc, char** argv) {

	/*Step 1: Parsing arguments from command line*/
	if (argc != 3) {
		printf("Error: expected 2 command arguments, but %d were provided\n", argc - 1);
		printf("Usage:\n\tConnect to server with specified IP address and port number in argument.\n");
		printf("\tClient login services.\n");
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

	/*Step 2: Start Winsock application*/
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");
	printf("Client started!\n");

	/*Step 3: Construct socket*/
	SOCKET client_socket;
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Set time-out interval
	int tv = 3000; //Time-out interval: 3000ms
	setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	int ret;
	/*Step 4: Specifying server address*/
	SOCKADDR_IN	server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	ret = connect(client_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (ret == SOCKET_ERROR) {
		printf("Can't connect to server[%s:%d]\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
		return -1;
	}
	printf("Connected to server[%s:%d]\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
	printf("Enter 'help' or 'HELP' to show manual.\n");

	/*Step 5: Communicate with server*/
	char buff[1024];
	while (1) {
		printf(">>>");
		gets_s(buff, 100);
		//show manual when press 'help'
		if (!strcmp(buff, "help") || !strcmp(buff, "HELP")) {
			printf("Login application for client.\n");
			printf("Usage: <command> <argument>\n");
			printf("\t+To enter username: 'user <your_username>' or 'USER <your_username>'\n");
			printf("\t\tE.g, 'user admin123', 'USER admin123'\n");
			printf("\t+To enter password: 'pass <your_pass_word>'\n");
			printf("\t\tE.g, 'pass mypass123', 'PASS mypass123'\n");
			printf("\t+To logout: 'logout' or 'LOGOUT'\n");
			printf("\t+To exit: 'exit' or 'EXIT'\n");
			continue;
		}
		//exit if press 'exit'
		if (!strcmp(buff, "exit") || !strcmp(buff, "EXIT")) break;
		Send(client_socket, buff, strlen(buff) + 1);
		ret = Receive(client_socket, buff, 1024);
		if (ret == -1) {
			printf("An error occur.\n");
			break;
		}
		switch (buff[0]) {
		case 0:
			printf("+OK: valid username (code 0).\n");
			break;
		case 1:
			printf("-ERROR: Username doesn't exit (code 1).\n");
			break;
		case 2:
			printf("-ERROR: Already logged-in, please logout before re-login (code 2).\n");
			break;
		case 3:
			printf("-ERROR: This account has been locked (code 3).\n");
			break;
		case 4:
			printf("+OK: Login succesfully (code 4).\n");
			break;
		case 5:
			printf("-ERROR: Password is incorrect (code 5).\n");
			break;
		case 6:
			printf("-ERROR: You must first enter username (code 6).\n");
			break;
		case 7:
			printf("-ERROR: You have attempted to login more than 3 times illegally. Your account is locked (code 7).\n");
			break;
		case 8:
			printf("+OK: Logout successfully (code 8).\n");
			break;
		case 9:
			printf("-ERROR: You haven't logged-in (code 9).\n");
			break;
		case 10:
			printf("-ERROR: Invalid request! Enter 'help' for more details (code 10).\n");
			break;
		default:
			printf("-ERROR: There was an unexpected error!\n");
			break;
		}
	}
	/*step 6: close socket, cleanup*/
	closesocket(client_socket);
	printf("Socket closed!");
	WSACleanup();
	return 0;
}