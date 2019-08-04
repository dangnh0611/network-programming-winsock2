#pragma once
#pragma once
#include<WinSock2.h>
#include<process.h>
#include<stdio.h>
#include<string>

/*WORKER THREAD
	each worker thread can process maximum 64 events from clients.
*/
unsigned __stdcall WorkerThread(void *param);