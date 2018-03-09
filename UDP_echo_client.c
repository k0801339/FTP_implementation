#include <winsock2.h>
#include <stdio.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 8888

int main()
{
	SOCKET serverSocket;
	struct sockaddr_in serverAddress;
	int bytesRead, serverAddressLen;
	char buf[MAX_SIZE];

	// call WSAStartup first for Winsock
    WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) {
        MY_ERROR("Winsock Error\n");
	}

	serverSocket = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(SERVER_PORT);

	while(1) {
		scanf("%s", buf);

        serverAddressLen = sizeof(serverAddress);
		sendto(serverSocket, buf, strlen(buf), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
		bytesRead = recvfrom(serverSocket, buf, MAX_SIZE, 0, (struct sockaddr *)&serverAddress, &serverAddressLen);
		buf[bytesRead] = '\0';
		if(bytesRead>0) printf("Receive %d byte(s): %s\n", bytesRead, buf);
	}

	return 0;
}
