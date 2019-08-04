#include"communicate.h"

int Receive(SOCKET s, char *buff, int size) {
	int n;

	n = recv(s, buff, size,0);
	if (n == SOCKET_ERROR)
		printf("Error: %d", WSAGetLastError());

	return n;
}

/* The send() wrapper function*/
int Send(SOCKET s, char *buff, int size) {
	int n;

	n = send(s, buff, size,0);
	if (n == SOCKET_ERROR)
		printf("Error: %d", WSAGetLastError());

	return n;
}