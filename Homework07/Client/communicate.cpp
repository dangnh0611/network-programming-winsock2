#pragma once
#include"communicate.h"
#define BUFF_SIZE 1400

/*	Receive message from client, store in buff, maximum length = buff_len
Return: -1 if error occur, else return message's length in bytes
*/
int Receive(SOCKET socket, char* buff, int buff_len) {
	//if buffer is not big enough to contain 2 bytes of msg header
	if (buff_len < 2) {
		printf("ERROR: Buffer is too small!\n");
		return -1;
	}
	//receive the msg header, get the msg's length
	int ret = recv(socket, buff, 2, 0);
	if (ret == SOCKET_ERROR) {
		printf("ERROR: %d\n", WSAGetLastError());
		return -1;
	}
	u_short msgLen = buff[0] * 256 + buff[1];
	if (buff_len < msgLen) {
		printf("Buffer's length is too small!");
		return -1;
	}
	//if msgLen is large
	int nLeft = msgLen;
	int idx = 0;
	while (nLeft > 0) {
		ret = recv(socket, buff + idx, nLeft, 0);
		if (ret == SOCKET_ERROR) {
			printf("ERROR: %d\n", WSAGetLastError());
			return -1;
		}
		idx += ret;
		nLeft -= ret;
	}
	return msgLen;
}


/*	Send message to client, message stored in msg_buff, message's length = msg_len
Return: -1 if error occur, else return message's length in bytes
*/
int Send(SOCKET socket, char* msg_buff, int msg_len) {
	//Construct msg with msg size as header
	static char buff[BUFF_SIZE];
	buff[0] = msg_len / 256;
	buff[1] = msg_len % 256;
	memcpy_s(buff + 2, BUFF_SIZE - 2, msg_buff, msg_len);
	//send msg, handle error if any
	int ret = send(socket, buff, msg_len + 2, 0);
	if (ret == SOCKET_ERROR) {
		printf("ERROR: %d\n", WSAGetLastError());
		return -1;
	}
	//if msg's length is large
	if (ret < msg_len + 2) {
		int nLeft = msg_len + 2 - ret;
		int idx = ret - 2;
		while (nLeft > 0)
		{
			ret = send(socket, msg_buff + idx, nLeft, 0);
			if (ret == SOCKET_ERROR)
			{
				printf("ERROR: %d\n", WSAGetLastError());
				return -1;
			}
			nLeft -= ret;
			idx += ret;
		}
	}
	return msg_len;
}
