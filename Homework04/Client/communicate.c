#include"communicate.h"
#define BUFF_SIZE 1027

int Send(SOCKET socket, u_char opcode, u_short length, char *payload) {
	static char buff[BUFF_SIZE];
	buff[0] = (u_char)opcode;
	buff[1] = (u_char)(length / 256);
	buff[2] = (u_char)(length % 256);
	memcpy_s(buff + 3, BUFF_SIZE, payload, length);
	int ret;
	ret = send(socket, buff, length + 3, 0);
	if (ret == SOCKET_ERROR) {
		printf("Socket error: %d. ", WSAGetLastError());
		return -1;
	}
	return 0;
}

int Receive(SOCKET socket, u_char* opcode, u_short* length, char* payload) {
	static char buff[3];
	int ret = recv(socket, buff, 3, 0);
	if (ret == SOCKET_ERROR) {
		printf("Socket error: %d. ", WSAGetLastError());
		return -1;
	}
	*opcode = (u_char)buff[0];
	*length = (u_char)buff[1] * 256 + (u_char)buff[2];
	if (*length > BUFF_SIZE) {
		printf("Error: Length field is invalid. ");
		return -1;
	}
	else if (*length == 0) return 0;
	else {
		ret = recv(socket, payload, *length, MSG_WAITALL);
		if (ret == SOCKET_ERROR) {
			printf("Socket error: %d. ", WSAGetLastError());
			return -1;
		}
		return 0;
	}
}
