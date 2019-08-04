#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"communicate.h"
#define BUFF_SIZE 1024

int main(int argc, char** argv) {

	//Step 1: Parsing arguments from command line
	if (argc != 3) {
		printf("Error: expected 2 command arguments, but %d were provided\n", argc - 1);
		printf("Usage:\n\tConnect to server with specified IP address and port number in argument.\n");
		printf("\tCeasar crypto system service.\n");
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

	int ret;
	//Step 4: Specifying server address
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

	//Step 5: Communicate with server
	char buff[1024];
	u_char opcode;
	u_char key;
	char filename[MAX_PATH + 4];
	FILE* file;
	int selection1, selection2;
	while (1) {
		/*Step 5.1: Get imput from user*/
		printf("Select services:\n");
		printf("\t0. Encrypt\n");
		printf("\t1. Decrypt\n");
		do {
			printf(">>>Select service: ");
			ret = scanf_s("%d", &selection1);
			while (getchar() != '\n');	//clear stdin buffer
		} while (ret < 1 || (selection1 != 0 && selection1 != 1));
		printf("Select key, key must be an integer in range [0,255]\n");
		do {
			printf(">>>Select key: ");
			ret = scanf_s("%d", &selection2);
			while (getchar() != '\n');	//clear stdin buffer
		} while (ret < 1 || selection2 < 0 || selection2>255);
		printf(">>>Select file path: ");
		gets_s(filename, MAX_PATH);
		if (strlen(filename) == 0) {
			printf("Exit client.\n");
			return 0;
		}
		file = fopen(filename, "rb");
		if (file == NULL) {
			printf("Can't open file\n");
			continue;
		}
		else {
			printf("Open file successfully\n");
		}
		opcode = selection1;
		key = selection2;
		/*Step 5.2: Send file to server*/
		ret=Send(client_socket, opcode, 1, &key);
		if (ret == -1) {
			printf("Error, can't connect to server.\n");
			return -1;
		}
		//Get file's size
		fseek(file, 0, SEEK_END);
		int fsize = ftell(file);
		fseek(file, 0, SEEK_SET);
		printf("%s file= '%s', size= %ld, key= %hhd\n", opcode == 0 ? "Encrypt" : "Decrypt", filename, fsize, key);
		printf("Sendding to server...\nProgress: [");
		//Send file to server
		int sum = 0, progress = 0, i = 0, j = 0;
		while ((ret = fread(buff, 1, BUFF_SIZE, file)) != 0) {
			if (Send(client_socket, 2, ret, buff) == -1) {
				printf("Error, can't connect to server.\n");
				return -1;
			}
			//show progress
			sum += ret;
			j = (int)(40.0*sum / fsize);
			for (i = 0; i < j - progress; i++) {
				printf("#");
				progress = j;
			}

		}
		ret=Send(client_socket, 2, 0, NULL);
		if (ret == -1) {
			printf("Error, can't connect to server.\n");
			return -1;
		}
		printf("]\nSuccesfully sent file to server\n");
		fclose(file);
		/*Step 5.3: Receive encypted/decrypted file from server*/
		file = fopen(strcat(filename, ".enc"), "wb");
		u_char opcode;
		u_short length;
		sum = 0;
		progress = 0;
		j = 0;
		printf("\nReceiving result from server...\nProgress: [");
		while (1) {
			ret=Receive(client_socket, &opcode, &length, buff);
			if (ret == -1) {
				printf("Error, can't connect to server.\n");
				return -1;
			}
			if (opcode == 2) {
				if (length == 0) {
					fsize = ftell(file);
					printf("]\nSuccessfully received file= %s, size=%ld\n", filename, fsize);
					fclose(file);
					break;
				}
				else {
					ret = fwrite(buff, 1, length, file);
					sum += ret;
					//show progress
					j = (int)(40.0*sum / fsize);
					for (i = 0; i < j - progress; i++) {
						printf("#");
						progress = j;
					}
				}
			}
			else {		//if server send error msg
				printf("Error occurs at server.\n");
				return -1;
			}
		}
	}
	//step 6: close socket, cleanup
	closesocket(client_socket);
	printf("Socket closed!");
	WSACleanup();
	return 0;
}
