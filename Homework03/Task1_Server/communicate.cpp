#pragma once
#include"communicate.h"
#define BUFF_SIZE 1400

/*function to receive msg using TCP connection
using 2 bytes-length header to specify the msg's length in bytes
***Parameter:
socket: the socket which receives msg from
buff: the buffer to receive msg
buff_len: specify buffer's length
flags: flag to control behaviors of recv function
***Return value:
size of the msg sent in bytes
-1 if error occurs
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
	if (buff_len >= msgLen) {
		ret = recv(socket, buff, msgLen, MSG_WAITALL);
		if (ret == SOCKET_ERROR) {
			printf("ERROR: %d\n", WSAGetLastError());
			return -1;
		}
		else return ret;
	}
	else return -1;
}

/*function to send msg using TCP connection
using 2 bytes-length header to specify the msg's length in bytes
***Parameter:
socket: the socket which receives msg from
buff: the buffer to msg
buff_len: msg 's lenght in bytes
flags: flag to control behaviors of send function
***Return value:
size of the msg sent in bytes
-1 if error occurs
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
	return ret;
}
