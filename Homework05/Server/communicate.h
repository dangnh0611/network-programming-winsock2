#pragma once
#pragma once
#include<WinSock2.h>
#include<stdio.h>

/*function to receive msg using TCP connection
using 2 bytes-length header to specify the msg's length in bytes
	buff: the buffer to receive msg
	buff_len: specify buffer's length
*Return: -1 if error occur, else return msg's length in succesfully
*/
int Receive(SOCKET socket, char* buff, int buff_len);

/*function to send msg using TCP connection
using 2 bytes-length header to specify the msg's length in bytes
	buff: the buffer to msg
	buff_len: msg 's lenght in bytes
*Return: -1 if error occur, else return msg's length in succesfully
*/
int Send(SOCKET socket, char* msg_buff, int msg_len);