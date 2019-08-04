#pragma once
#include<WinSock2.h>
#include<stdio.h>

extern DWORD  flags;	//flags control WSASend and WSARecv functions

/*	Receive message from client, store in buff
Return: -1 if error occur, else return 0
*/
int Receive(SOCKET socket, LPWSABUF buff, LPDWORD numberOfBytesReceived, LPWSAOVERLAPPED lpOverlapped);

/*	Send message to client, stored in buff
Return: -1 if error occur, else return 0
*/
int Send(SOCKET socket, LPWSABUF buff, LPDWORD numberOfBytesSent, LPWSAOVERLAPPED lpOverlapped);