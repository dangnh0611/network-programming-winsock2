#pragma once
#include<stdio.h>
#include<WinSock2.h>
#include"main.h"

/*
Set fname to store a random file name
[out]: fname
*/
void getRandomFileName(char fname[30]);

/*Encrypt function
[in,out]: buff
[in]: len-length of buffer
[in] key used to encrypt
*/
void Encrypt(char* buff, u_short len, u_char key);

/*Decrypt function
[in,out]: buff
[in]: len-length of buffer
[in] key used to decrypt
*/
void Decrypt(char* buff, u_short len, u_char key);

//Receive activities, when socket is ready to receive from client
int receiveProcess(CLIENT* client);

//Send activities, when socket is ready to send to client
int sendProcess(CLIENT* client);