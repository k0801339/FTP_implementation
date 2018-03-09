#include <winsock2.h>
#include <stdio.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 9999

int main(int argc, char **argv) {
	SOCKET serverSocket;
	struct sockaddr_in serverAddress;
	int bytesRead;
	char buf[MAX_SIZE];

	// call WSAStartup first for Winsock
    WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) {
        MY_ERROR("Winsock Error\n");
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]); // transform to 32-bit unsigned integer
    serverAddress.sin_port = htons(atoi(argv[2])); //converts a u_short from host to TCP/IP network byte order

	while(1) {
		scanf("%s", buf);

		serverSocket = socket(PF_INET, SOCK_STREAM, 0);
		if(connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0){
            MY_ERROR("Connect Error\n");
		}
		send(serverSocket, buf, strlen(buf), 0);
		bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
		buf[bytesRead] = '\0';
		if(bytesRead>0) printf("Receive %d byte(s): %s\n", bytesRead, buf);
		closesocket(serverSocket);
	}

	return 0;
}
