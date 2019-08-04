#include<process.h>

unsigned __stdcall client_threads(void *param) {
	system("C:\\Users\\dangn\\OneDrive\\Documents\\\"visual studio 2015\"\\Projects\\Homework06\\Debug\\Client.exe 127.0.0.1 611");
	return 0;
}

int main() {
	for (int i = 0; i < 100; i++) {
		_beginthreadex(0, 0, client_threads, 0, 0, 0);
	}
	while (1);
}

