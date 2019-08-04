#pragma once
#include<stdio.h>
#include<WinSock2.h>
/*
Send a msg with specified socket,opcode,length and payload
Return:
-1 if an error occur
0 if succesfully
*/
int Send(SOCKET socket, u_char opcode, u_short length, char *payload);

/*
Receive a msg with specified socket,opcode,length and payload
Return:
-1 if an error occur
0 if succesfully
*/
int Receive(SOCKET socket, u_char* opcode, u_short* length, char* payload);
