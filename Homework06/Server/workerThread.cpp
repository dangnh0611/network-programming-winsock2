#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"workerThread.h"
#include"communicate.h"
#include"main.h"
#include"process.h"
using namespace std;

/*WORKER THREAD
each worker thread can process maximum 64 events from clients.
*/
unsigned __stdcall WorkerThread(void *param) {
	int thread_index = *(int*)param;	//index to identify thread
	int base_index = thread_index* WSA_MAXIMUM_WAIT_EVENTS;
	int index = 0, ret = 0;
	WSANETWORKEVENTS sockEvent;
	while (1) {
		//wait for network events on maximum 64 sockets
		index = WSAWaitForMultipleEvents(WSA_MAXIMUM_WAIT_EVENTS, events + base_index, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
			break;
		}


		index = base_index + index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(client_list[index].socket, events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				printf("FD_READ failed with error %d\n", sockEvent.iErrorCode[FD_READ_BIT]);
				disconnect(index);
				continue;
			}
			ret = processRequest(index);
			//if there is an error, disconnect
			if (ret == -1) {
				disconnect(index);
			}
			WSAResetEvent(events[index]);
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				printf("FD_CLOSE failed with error %d\n", sockEvent.iErrorCode[FD_CLOSE_BIT]);
				disconnect(index);
				continue;
			}
			//Release socket and event
			disconnect(index);
		}
	}
	return 0;
}
