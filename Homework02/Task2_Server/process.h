#pragma once
#include<WinSock2.h>
#include<stdio.h>

//Service for name system resolution on a TCP connection
int NSLookup(SOCKET socket, char* buff);

//Check whether a string is in IPv4 form
int isIpv4Form(char* buff);