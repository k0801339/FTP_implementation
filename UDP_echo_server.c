#include <winsock2.h>
#include <stdio.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 8888

int main()
{
	SOCKET serverSocket;
	struct sockaddr_in serverAddress, clientAddress;
	int clientAddressLen;
	int bytesRead;
	char buf[MAX_SIZE];

	// call WSAStartup first for Winsock
    WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) {
        MY_ERROR("Winsock Error\n");
	}

	serverSocket = socket(PF_INET, SOCK_DGRAM, 0);

	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(SERVER_PORT);

    if( bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        MY_ERROR("Bind Error\n");
	}

	while(1) {
		printf("Waiting...\n");

		clientAddressLen = sizeof(clientAddress);
		bytesRead = recvfrom(serverSocket, buf, MAX_SIZE, 0, (struct sockaddr *)&clientAddress, &clientAddressLen);
		buf[bytesRead] = '\0';
		sendto(serverSocket, buf, bytesRead, 0, (struct sockaddr *)&clientAddress, clientAddressLen);
		printf("Receive %d byte(s): %s\n", bytesRead, buf);
	}

	return 0;
}










