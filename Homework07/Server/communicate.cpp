#pragma once
#include"communicate.h"

DWORD  flags = 0;	//flags control WSASend and WSARecv functions

/*	Receive message from client, store in buff
	Return: -1 if error occur, else return 0
*/
int Receive(SOCKET socket, LPWSABUF buff, LPDWORD numberOfBytesReceived, LPWSAOVERLAPPED lpOverlapped) {
	//clear overlapped structure
	ZeroMemory(lpOverlapped, sizeof(WSAOVERLAPPED));

	if (WSARecv(socket, buff, 1, numberOfBytesReceived, &flags, lpOverlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
			return -1;
		}
	}
	return 0;
}


/*	Send message to client, stored in buff
Return: -1 if error occur, else return 0
*/
int Send(SOCKET socket, LPWSABUF buff, LPDWORD numberOfBytesSent, LPWSAOVERLAPPED lpOverlapped) {
	//clear overlapped structure
	ZeroMemory(lpOverlapped, sizeof(OVERLAPPED));

	if (WSASend(socket, buff, 1, numberOfBytesSent, flags, lpOverlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			printf("WSASend() failed with error %d\n", WSAGetLastError());
			return -1;
		}
	}
	return 0;
}


