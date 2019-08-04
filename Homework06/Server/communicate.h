#pragma once
#pragma once
#include<WinSock2.h>
#include<stdio.h>

/*	Receive message from client, store in buff, maximum length = buff_len
Return: -1 if error occur, else return message's length in bytes
*/
int Receive(SOCKET socket, char* buff, int buff_len);

/*	Send message to client, message stored in msg_buff, message's length = msg_len
Return: -1 if error occur, else return message's length in bytes
*/
int Send(SOCKET socket, char* msg_buff, int msg_len);