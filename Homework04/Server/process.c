#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"communicate.h"
#include"process.h"
#include"main.h"

void getRandomFileName(char fname[30]) {
	static int index = 0;
	sprintf_s(fname, 30, "temp\\file_%d", index++);
	return;
}

void Encrypt(char* buff, u_short len, u_char key) {
	for (int i = 0; i < len; i++) {
		buff[i] = ((u_char)buff[i] + key) % 256;
	}
	return;
}

void Decrypt(char* buff, u_short len, u_char key) {
	for (int i = 0; i < len; i++) {
		buff[i] = ((u_char)buff[i] - key) % 256;
	}
	return;
}

int receiveProcess(CLIENT* client) {
	static u_char opcode;
	static u_short length;
	static char payload[BUFF_SIZE];
	int ret = Receive(client->socket, &opcode, &length, payload);
	if (ret == -1) return -1;
	if (opcode == 2) {
		if (length == 0) {
			printf("\nReceived file with size: %d bytes from client [%s:%d]\n", ftell(client->file), inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
			fseek(client->file, 0, SEEK_SET);
			return 1;
		}
		else {
			ret = fwrite(payload, 1, length, client->file);
			//if error occurs
			if (ret != length) {
				Send(client->socket, 3, 0, NULL);	//sent error msg to client
				return -1;
			}
			return 0;
		}
	}
	else if (opcode == 0) client->control = 0;
	else if (opcode == 1) client->control = 1;
	client->key = payload[0];
	getRandomFileName(client->filename);
	client->file = fopen(client->filename, "wb+");
	return 0;
}
int sendProcess(CLIENT* client) {
	int ret;
	static char payload[BUFF_SIZE];
	int nBytes = fread(payload, 1, BUFF_SIZE, client->file);
	//if error occurs
	if (ferror(client->file)) {
		Send(client->socket, 3, 0, NULL);	//sent error msg to client
		return -1;
	}
	if (nBytes == 0) {
		ret = Send(client->socket, 2, 0, NULL);
		if (ret == -1) return -1;
		printf("\nSent file with size: %d bytes to client [%s:%d]\n", ftell(client->file), inet_ntoa(client->addr.sin_addr), ntohs(client->addr.sin_port));
		fclose(client->file);
		remove(client->filename);
		return 1;
	}
	else {
		if (client->control == 0)	//if encrypt file
			Encrypt(payload, nBytes, client->key);
		else	//if decrypt file
			Decrypt(payload, nBytes, client->key);
		ret = Send(client->socket, 2, nBytes, payload);
		if (ret == -1) return -1;
		return 0;
	}
}