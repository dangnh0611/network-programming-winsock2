#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include"main.h"
#include"MSGRegister.h"
#include"communicate.h"
#include"process.h"

//Register for window
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;	//register callback function to handle msg
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "Homework05";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

//Init and show window
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	//create window with window size 0*0 
	hWnd = CreateWindow("Homework05", "Homework05", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 0, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

//Window Procedure receive message
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			disconnect((SOCKET)wParam);
		}

		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_ACCEPT:		//if a client sends a request to connect
		{
			SOCKET connSock;
			SOCKADDR_IN clientAddr;
			int clientAddrLen = sizeof(clientAddr);
			connSock = accept((SOCKET)wParam, (SOCKADDR *)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET)
				break;
			//init client info and status
			CLIENT client;
			client.ipAddr = inet_ntoa(clientAddr.sin_addr);
			client.port = ntohs(clientAddr.sin_port);
			client.step = 0;
			client.username = "";

			printf("Connected to client [%s:%d]\n", client.ipAddr.c_str(), client.port);
			client_list.insert(std::pair<SOCKET, CLIENT>(connSock, client));	//add client to client_list
			//requests Windows message-based notification of network events for listenSock
			WSAAsyncSelect(connSock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);	//register to receive winsock msg
		}
		break;

		case FD_READ:	//if a connected client send a request
			if (processRequest((SOCKET)wParam) != -1) {}
			else break;

		case FD_CLOSE:	//if client exit, or send a disconnect request
			disconnect((SOCKET)wParam);
			break;
		}
	}
	break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}

	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}