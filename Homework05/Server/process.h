#pragma once
#include"main.h"

/* process when client send a request */
int processRequest(SOCKET& client);

/*	Disconnect when client exit	*/
int disconnect(SOCKET& client);