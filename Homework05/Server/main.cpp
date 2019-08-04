#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include"main.h"
#include"MSGRegister.h"
using namespace std;
	/*############################################################################
	##	CREATE A HIDDEN WINDOW AND REGISTER TO RECEIVE WINSOCK MESSAGE          ##
	##	HANDLE AND OUTPUT TO A CONSOLE WINDOW									##
	############################################################################*/

/*	client: username
	->server: *0-username exits and user is not blocked
			*1-username doesn't exit
			*2-client already login at that time, first, log out before re-login
			*3-user was locked before
	client: password
	->server: *2-client already login at that time, first, log out before re-login
			*3-user was locked before
			*4-password is correct, login successfully
			*5-password is incorrect
			*6-client must first enter username
			*7-user is locked because client enter wrong password over 3 times
	client: logout
	->server: *8-logout successfully
			*9-user hasn't been login
	client: invalid request
	->server: *10-invalid request
*/
SOCKET listenSock;
u_short PORT_NUMBER = 5500;		//server listen at port 5500
unordered_map<string, ACCOUNTINFO> account_info;
unordered_map<SOCKET, CLIENT> client_list;
FILE* file;

int main() {
	MSG msg;
	/*Step 1: Init Window and register to receive message*/
	HWND serverWindow;
	HINSTANCE hInstance = NULL;
	//Registering the Window Class
	MyRegisterClass(hInstance);
	//Create the window
	if ((serverWindow = InitInstance(hInstance, SW_HIDE)) == NULL) {	//hide the created window
		printf("Error, can't create window for receive messages.\n");
		return FALSE;
	}

	/*Step 2:Read account infomations from file	*/
	printf("Account infomation stored in ./account.txt\n");
	file = fopen("account.txt", "r+");
	if (!file) {
		printf("Can't open \"account.txt\" to read account infomations!\n");
		return -1;
	}
	else printf("Open \"account.txt\" successfully!\n");
	printf("Reading accounts infomation.\n");
	ACCOUNTINFO acc;
	string username;
	char temp[50];
	while (1) {
		if (fscanf(file, "%s%s %c", temp, acc.password, &acc.status) == 3) {
			printf("User: %s\n", temp);
			printf("\tpass: %s\n", acc.password);
			printf("\tstatus: %c\n", acc.status);
			username = temp;
			acc.seek_pos = ftell(file) - 1;
			account_info.insert(pair<string, ACCOUNTINFO >(username, acc));
		}
		else break;
	}

	/*Step 3: Start Winsock application*/
	WSADATA wsadata;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsadata)) printf("This version is not supported!\n");

	/*Step 4: Construct Socket*/
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//Bind the socket
	SOCKADDR_IN serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUMBER);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenSock, (SOCKADDR*)(&serverAddress), sizeof(serverAddress))) {
		printf("ERROR in binding socket\n");
		WSACleanup();
		return 0;
	}
	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);
	/*Step 5: Start listening for requests*/
	printf("Listening at Port %d\n", PORT_NUMBER);
	if (listen(listenSock, 1024)) {
		perror("ERROR: ");
		return 0;
	}
	printf("Server started.\n");
	/*Step 6: Main message loop:*/
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}