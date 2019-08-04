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
