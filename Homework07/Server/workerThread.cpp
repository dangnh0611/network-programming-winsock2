#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"workerThread.h"
#include"communicate.h"
#include"main.h"
#include"process.h"

using namespace std;

/*   WORKER THREAD   */
unsigned __stdcall WorkerThread(void *completionPortID) {
	HANDLE completionPort = (HANDLE)completionPortID;
	DWORD transferredBytes;	//number of bytes tranferred in the last IO operation
	LPPER_HANDLE_DATA lpPerHandleData;
	LPPER_CLIENT_DATA lpPerClientData;
	int ret = 0;
	u_char ret_code = 0;	//reply to send to client after processing request
	while (1) {
		if (GetQueuedCompletionStatus(completionPort, &transferredBytes,
			(LPDWORD)&lpPerHandleData, (LPOVERLAPPED *)&lpPerClientData, INFINITE) == 0) {
			//if pointer to overlapped structure is not NULL,
			//it mean that the function dequeues a completion packet for a failed I/O operation from the completion port,
			//the function still stores information about the failed operation
			//it can occur when client teminated abnormally by ctrl+C , etc.
			if (lpPerClientData) {
				disconnect(lpPerHandleData);
				continue;
			}
			else {
				printf("GetQueuedCompletionStatus() failed with error %d\n", GetLastError());
				break;
			}
		}
		// Check to see if an error has occurred on the socket and if so
		// then close the socket
		if (transferredBytes == 0) {
			disconnect(lpPerHandleData);
			continue;
		}
		//if last IO receive the request msg's length
		if (lpPerClientData->operation == GET_MSG_LEN) {
			lpPerClientData->dataBuff.len = lpPerClientData->buffer[0] * 256 + lpPerClientData->buffer[1];
			lpPerClientData->dataBuff.buf = lpPerClientData->buffer;
			lpPerClientData->operation = RECEIVE_MSG;
			ret = Receive(lpPerHandleData->socket, &lpPerClientData->dataBuff, NULL, &lpPerClientData->overlapped);
			if (ret == -1) {
				disconnect(lpPerHandleData);
			}
			continue;
		}
		// if the last IO completion is RECEIVE
		if (lpPerClientData->operation == RECEIVE_MSG) {
			lpPerClientData->dataBuff.len -= transferredBytes;
			//if last IO completely receive a request
			//process the request and put the reply to buffer
			if (lpPerClientData->dataBuff.len == 0) {
				lpPerClientData->operation = SEND_MSG;
				lpPerClientData->dataBuff.len = 3;
				lpPerClientData->dataBuff.buf = lpPerClientData->buffer;
				//put reply msg to buffer
				ret_code = processRequest(lpPerClientData);
				lpPerClientData->buffer[0] = 0;
				lpPerClientData->buffer[1] = 1;
				lpPerClientData->buffer[2] = ret_code;
			}
			//if last IO doesn't completely receive request
			else {
				lpPerClientData->dataBuff.buf += transferredBytes;
				ret = Receive(lpPerHandleData->socket, &lpPerClientData->dataBuff, NULL, &lpPerClientData->overlapped);
				if (ret == -1) {
					disconnect(lpPerHandleData);
				}
				continue;
			}
		}
		//if the last IO is SEND
		else if (lpPerClientData->operation == SEND_MSG) {
			lpPerClientData->dataBuff.len -= transferredBytes;
			lpPerClientData->dataBuff.buf += transferredBytes;
		}

		// If no more bytes to send, post another WSARecv() request
		if (lpPerClientData->dataBuff.len == 0) {
			lpPerClientData->operation = GET_MSG_LEN;
			lpPerClientData->dataBuff.len = 2;
			lpPerClientData->dataBuff.buf = lpPerClientData->buffer;
			ret = Receive(lpPerHandleData->socket, &(lpPerClientData->dataBuff), NULL, &(lpPerClientData->overlapped));
			if (ret == -1) {
				disconnect(lpPerHandleData);
			}
		}
		else {
			// Post another WSASend() request.
			// Since WSASend() is not guaranteed to send all of the bytes requested,
			// continue posting WSASend() calls until send reply msg completely
			ret = Send(lpPerHandleData->socket, &lpPerClientData->dataBuff, NULL, &lpPerClientData->overlapped);
			if (ret == -1) {
				disconnect(lpPerHandleData);
			}
		}
	}

	//if error occur, safely close completion port
	PostQueuedCompletionStatus(completionPort, 0, NULL, NULL);
	printf("CompletionPort closed!\n");
	CloseHandle(completionPort);
	return 0;
}
