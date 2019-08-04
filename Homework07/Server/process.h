#pragma once
#include"main.h"

/* Process when client send a request */
u_char processRequest(LPPER_CLIENT_DATA client_data);

/*	Disconnect when client exit	*/
void disconnect(LPPER_HANDLE_DATA handleData);