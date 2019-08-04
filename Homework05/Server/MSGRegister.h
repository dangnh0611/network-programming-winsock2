#pragma once
#include"main.h"

//Register a window class
ATOM MyRegisterClass(HINSTANCE hInstance);

//Init window
HWND InitInstance(HINSTANCE hInstance, int nCmdShow);

//define callback function to handle when receive a msg
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);