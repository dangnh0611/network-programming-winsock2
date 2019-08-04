#pragma once
#include<WinSock2.h>
#include<stdio.h>

int Receive(SOCKET socket, char* buff, int buff_len);


int Send(SOCKET socket, char* msg_buff, int msg_len);
