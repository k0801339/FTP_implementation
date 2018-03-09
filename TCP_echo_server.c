#include <winsock2.h>
#include <stdio.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 9999

int main(int argc, char **argv) {
	SOCKET serverSocket, clientSocket; // create a socket
	struct sockaddr_in serverAddress, clientAddress; // sockaddr_in：for IPv4 ,  sockaddr_in6：for IPv6 , sockaddr：general format
	int clientAddressLen;
	int bytesRead;
	char buf[MAX_SIZE];

	// call WSAStartup first for Winsock
    WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) { // ( version of winsock )
        MY_ERROR("Winsock Error\n");
	}

	serverSocket = socket(PF_INET, SOCK_STREAM, 0); // (address , type , protocal )  

	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(atoi(argv[1]));//converts a u_short from host to TCP/IP network byte order

    if( bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        MY_ERROR("Bind Error\n");
	}

	if( listen(serverSocket, 3) < 0) {
		MY_ERROR("Listen Error\n");
	}

	while(1) {
		printf("Waiting...\n");
		clientAddressLen = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
		
		bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
		buf[bytesRead] = '\0';
		send(clientSocket, buf, bytesRead, 0);
		printf("Client IP is : %s \n", inet_ntoa(clientAddress.sin_addr));
		printf("Receive %d byte(s): %s\n", bytesRead, buf);
		closesocket(clientSocket);
	}

	return 0;
}
