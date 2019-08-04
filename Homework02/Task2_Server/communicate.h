#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<stdio.h>

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
int Receive(SOCKET socket, char* buff, int buff_len);

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
int Send(SOCKET socket, char* msg_buff, int msg_len);




