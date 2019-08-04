#include"communicate.h"

/*Check whether a string is in IPv4 form*/
int isIpv4Form(char* buff) {
	int num = 0;
	for (int i = 0; i < strlen(buff); i++) {
		if (isdigit(buff[i])) continue;
		else if (buff[i] == '.') num++;
		else return 0;
	}
	if (num != 3) return 0;
	else return 1;
}

//Service for name system resolution on a TCP connection
int NSLookup(SOCKET socket, char* buff) {
	HOSTENT * host_info = NULL;	//Store info about the host
	unsigned long ipv4_addr = inet_addr(buff);
	int state = 0;
	int ret;

	//if the client sent a string that is an IPv4 address
	if (isIpv4Form(buff)) {
		//if the addr is invalid,send error code to client as result
		if (ipv4_addr == INADDR_NONE) {
			state = 0;
			//send result code to client
			ret = Send(socket, (char*)&state, sizeof(int));
			if (ret == -1) {
				return -1;
			}
		}
		else {
			host_info = gethostbyaddr((const char*)&ipv4_addr, sizeof(ipv4_addr), AF_INET);
			//if can't find info about address
			if (host_info == NULL) {
				state = 1;
				Send(socket, (char*)&state, sizeof(int));
			}
			//else if found info about the addr
			else {
				state = 2;
				ret = Send(socket, (char*)&state, sizeof(int));
				if (ret == -1) {
					return -1;
				}
				ret = Send(socket, host_info->h_name, strlen(host_info->h_name) + 1);
				if (ret == -1) {
					return -1;
				}
				for (int i = 0; host_info->h_aliases[i] != NULL; i++) {
					ret = Send(socket, host_info->h_aliases[i], strlen(host_info->h_aliases[i]) + 1);
					if (ret == -1) {
						return -1;
					}
				}
				//send an empty string to client to indicate that 
				//the result has been sent succesfully
				ret = Send(socket, "", 1);
				if (ret == -1) {
					return -1;
				}
			}
		}
	}
	//if the client sent a string that is a host name
	else {
		host_info = gethostbyname(buff);
		//if can't find info about the name
		if (host_info == NULL) {
			state = 1;
			ret = Send(socket, (char*)&state, sizeof(int));
			if (ret == -1) {
				return -1;
			}
		}
		//else if found info about the name
		else {
			state = 3;
			ret = Send(socket, (char*)&state, sizeof(state));
			if (ret == -1) {
				return -1;
			}
			for (int i = 0; host_info->h_addr_list[i] != NULL; i++) {
				IN_ADDR addr;
				addr.s_addr = *(unsigned long*)(host_info->h_addr_list[i]);
				ret = Send(socket, inet_ntoa(addr), strlen(inet_ntoa(addr)) + 1);
				if (ret == -1) {
					return -1;
				}
			}
			//send an empty string to client to indicate that 
			//the result has been sent succesfully
			ret = Send(socket, "", 1);
			if (ret == -1) {
				return -1;
			}
		}
	}
	return 0;
}
