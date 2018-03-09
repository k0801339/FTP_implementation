#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  /* contains a number of basic derived types */
#include <winsock2.h> /* provide functions and data structures of socket */
#include <dirent.h>     /* format of directory entries */
#include <unistd.h>     /* `read()` and `write()` functions */
#include <sys/stat.h>   /* stat information about files attributes */
#include <time.h>
#define MAX_SIZE 1024
#define MAX_CONNECTION 3
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);

void Connection_handler(SOCKET sockfd);
void Welcome_handler(SOCKET sockfd);
void File_listing_handler(SOCKET sockfd);
void File_sending_handler(SOCKET sockfd, char filename[]);
void File_receving_handler(SOCKET sockfd, char filename[]);
void File_Rename(SOCKET sockfd, char oldname[], char newname[]);

int main(int argc, char **argv)
{
	SOCKET serverSocket, clientSocket; // create a socket
	struct sockaddr_in serverAddress, clientAddress;
	int clientAddressLen;
	//int bytesRead;
	//char buf[MAX_SIZE];
	
	WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) { // ( version of winsock )
        MY_ERROR("Winsock Error\n");
	}
	
	serverSocket = socket(PF_INET, SOCK_STREAM, 0); // (address , type , protocal ) 
	if(serverSocket<0){
		MY_ERROR("Create socket failed\n");
	}
	
	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(atoi(argv[1]));//converts a u_short from host to TCP/IP network byte order
    
    if( bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        MY_ERROR("Bind Error\n");
	}

	if( listen(serverSocket, MAX_CONNECTION) < 0) {
		MY_ERROR("Listen Error\n");
	}
	
	printf("FTP server starting\n");
	printf("Waiting for client...\n\n");
	clientAddressLen = sizeof(clientAddress);
	
	while(1){
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
		if(clientSocket<0){
			MY_ERROR("Accept Failed!\n");
		}
		printf("[msg] Connection accepted!\n");
		printf("[msg] Client IP is %s\n", inet_ntoa(clientAddress.sin_addr));
		
		Connection_handler(clientSocket);
		
		closesocket(clientSocket);
		printf("Waiting for client...");
	}
	closesocket(serverSocket);
	return 0;
}
void Connection_handler(SOCKET sockfd)
{
	char command[MAX_SIZE];
	char filename[MAX_SIZE];
	char newname[MAX_SIZE];
	memset(command, '\0', MAX_SIZE);
	//show welcome msg
	Welcome_handler(sockfd);
	
	printf("Waiting For Command...\n");
	while( (recv(sockfd, command, MAX_SIZE, 0) ) >0 ){
		if(strncmp(command,"exit",4)==0){
    		break;
		}else if(strncmp(command,"dir",3)==0){
			File_listing_handler(sockfd);
		}else if(strncmp(command,"get",3)==0){
			memset(filename, '\0', MAX_SIZE);
			recv(sockfd, filename, MAX_SIZE, 0);
			File_sending_handler(sockfd, filename);
		}else if(strncmp(command,"put",3)==0){
			memset(filename, '\0', MAX_SIZE);
			recv(sockfd, filename, MAX_SIZE, 0);
			File_receving_handler(sockfd, filename);
		}else if(strncmp(command,"rename",6)==0){
			memset(filename, '\0', MAX_SIZE);
			recv(sockfd, filename, MAX_SIZE, 0);
			memset(newname, '\0', MAX_SIZE);
			recv(sockfd, newname, MAX_SIZE, 0);
			File_Rename(sockfd, filename, newname);
		}else{
			printf("-----------\nInvalid Command\n-----------\n");
		}
		
		printf("Waiting For Command...\n");
	}
	
	printf("[msg] Connection Closed.\n-----------\n\n");
  	closesocket(sockfd);
  	return;
}
void Welcome_handler(SOCKET sockfd)
{
	char buf[MAX_SIZE];
	printf("\n[msg] Show Welcome Msg To Client\n");
	
	sprintf(buf, "%s", "\n[msg] Successfully Connected To FTP Server.\n-----------\n");
	if (send(sockfd, buf, MAX_SIZE, 0) < 0) {
      MY_ERROR("Write failed!\n");
  	}
}
void File_listing_handler(SOCKET sockfd)
{
	DIR* pDir;                      // directory
	struct dirent* pDirent = NULL;  // directory and children file in this dir
	char buf[MAX_SIZE];
	printf("-----------\n[msg] List File to client\n");
	
	if ((pDir = opendir("./upload")) == NULL) {
      	MY_ERROR("Open directory failed\n");
  	}
  	
	memset(buf, '\0', MAX_SIZE);
	while((pDirent = readdir(pDir)) != NULL){
		//dismiss folder's info
		if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
        	continue;
      	}
      	
      	sprintf(buf, "%s   ", pDirent->d_name);
		if (send(sockfd, buf, MAX_SIZE, 0) < 0) {
	      	MY_ERROR("Listing Write Failed\n");
	  	}
	  	
	  	memset(buf, '\0', MAX_SIZE);
	  	sprintf(buf, "./upload/%s", pDirent->d_name);
	  	//get file's size & last_modified_time
	  	struct stat g_file; 
		stat(buf, &g_file);
		//the time is all in second, we need to convert it into year/month/day
		struct tm *ts;
		char tmp[MAX_SIZE];
		ts = localtime(&g_file.st_mtime);
		strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", ts);
		
	  	memset(buf, '\0', MAX_SIZE);
	  	sprintf(buf, "%15d bytes      %s\n", g_file.st_size, tmp);
		send(sockfd, buf, MAX_SIZE, 0);
	}
	printf("[msg] List File Finish\n-----------\n");
	sprintf(buf, "%s", "o_v_e_r");
	send(sockfd, buf, MAX_SIZE, 0);
	closedir(pDir);
}
void File_sending_handler(SOCKET sockfd, char filename[])
{
	char fail_msg[] = "Download failed(No this file or wrong path)";
  	char buf[MAX_SIZE];   // buffer to store msg
  	char path[MAX_SIZE];  // path to this file
  	FILE *fp;             // file want to send
  	
  	int file_size = 0;    //  size of this file
  	int write_byte = 0;   //  bytes this time sent
  	int write_sum = 0;    //  bytes have been sent
  	
  	printf("-----------\n");
  	printf("[msg] Client send \"get %s\" request.\n\n", filename);
  	
  	sprintf(path, "upload/%s", filename);
  	fp = fopen(path, "rb");
  	
  	if(fp){
  		 /* send start downloading message */
  		printf("[msg] Starting Sending...\n");
    	memset(buf, '\0', MAX_SIZE);
    	sprintf(buf, "-----------\n[-] Downloading `%s` ...\n", filename);
    	if (send(sockfd, buf, MAX_SIZE, 0) < 0) {
        	MY_ERROR("Send downloading message failed");
    	}
    	/* get file size, store in file_size */
    	fseek(fp, 0, SEEK_END);
    	file_size = ftell(fp);
    	rewind(fp);

    	memset(buf, '\0', MAX_SIZE);
    	sprintf(buf, "%d", file_size);
    	if (send(sockfd, buf, MAX_SIZE, 0) < 0) {
      		MY_ERROR("File size write failed!\n");
  		}
		
		/* read file data and send to client */
    	write_sum = 0;
    	while (write_sum < file_size) {
      		/* read local file to buf */
      		memset(buf, '\0', MAX_SIZE);
      		write_byte = fread(&buf, sizeof(char), MAX_SIZE, fp);

			if (send(sockfd, buf, write_byte, 0) < 0) {
      			MY_ERROR("Data write failed!\n");
  			}

      		write_sum += write_byte;
    	}

    	fclose(fp);
    	/* sleep for a while */
    	sleep(2);
    	/* send download successful message */
    	memset(buf, '\0', MAX_SIZE);
    	printf("[msg] Sending Over.\n\n-----------\n");
    	sprintf(buf, "%s", "[กิ] Download successfully!\n\n");
    	send(sockfd, buf, strlen(buf), 0);
  	} else {
    	/* fp is null*/
    	printf("[ERROR] %s\n-----------\n", fail_msg);
    	send(sockfd, "E_R_R_O_R", MAX_SIZE, 0);
  	}
}
void File_receving_handler(SOCKET sockfd, char filename[])
{
	char buf[MAX_SIZE];
	char path[MAX_SIZE];
	int file_size = 0;    //  size of this file
	int read_byte = 0;    //  bytes this time recv
	int read_sum = 0;     //  bytes have been recv
	FILE *fp;
	
	printf("-----------\n[msg] Client send \"put %s\" request.\n\n", filename);
  	
  	/* receive start message from client */
  	memset(buf, '\0', MAX_SIZE);
  	recv(sockfd, buf, MAX_SIZE, 0);
  	if(strncmp(buf,"E_R_R_O_R",9)==0){
  		printf("[msg] Upload failed(No this file or wrong path)\n-----------\n");
  		return;
	}else{
		printf("%s", buf);
	}
  	
  	/* receive file size */
  	memset(buf, '\0', MAX_SIZE);
  	recv(sockfd, buf, MAX_SIZE, 0);
  	file_size = atoi(buf);
  	
  	/* file path */
  	memset(path, '\0', MAX_SIZE);
  	sprintf(path, "./upload/%s", filename);
  	
  	read_sum = 0;
  	fp = fopen(path, "wb");
  	if (fp) {
      	while (read_sum < file_size) {
        	memset(buf, '\0', MAX_SIZE);
			read_byte = recv(sockfd, buf, MAX_SIZE, 0);
        	/* write file to server disk*/
        	fwrite(&buf, sizeof(char), read_byte, fp);
        	read_sum += read_byte;
      	}
      	fclose(fp);
      	/* receive upload complete message */
      	memset(buf, '\0', MAX_SIZE);
      	recv(sockfd, buf, MAX_SIZE, 0);
      	printf("%s", buf);
      	printf("[msg] The '%s' File is %d bytes.\n\n-----------\n", filename, file_size);
  	}else{
    	MY_ERROR("Allocate memory fail\n");
  	}	
}
void File_Rename(SOCKET sockfd, char oldname[], char newname[])
{
	DIR* pDir;                      // directory
	struct dirent* pDirent = NULL;  // directory and children file in this dir
	char buf[MAX_SIZE];
	char newbuf[MAX_SIZE];
	
	printf("-----------\n[msg] Finding The File...\n\n");
	
	if ((pDir = opendir("./upload")) == NULL) {
      	MY_ERROR("Open directory failed\n");
  	}
	memset(buf, '\0', MAX_SIZE);
	sprintf(buf, "%s", "-----------\n[-] Attemping To Find The File...\n");
	send(sockfd, buf, MAX_SIZE, 0);
	
	int find = 0;
	while((pDirent = readdir(pDir)) != NULL){
		//dismiss folders' info
		if (strcmp(pDirent->d_name, ".") == 0 || strcmp(pDirent->d_name, "..") == 0) {
        	continue;
      	}
	  	
	  	if(strcmp(pDirent->d_name, oldname)==0){
	  		printf("[msg] Already Got This File, Renaming...\n");
	  		find = 1;
	  		memset(buf, '\0', MAX_SIZE);
	  		sprintf(buf, "./upload/%s", pDirent->d_name);
	  		memset(newbuf, '\0', MAX_SIZE);
	  		sprintf(newbuf, "./upload/%s", newname);
	  		if(rename(buf, newbuf)==0){		//means renaming action succeed
	  			memset(buf, '\0', MAX_SIZE);
	  			sprintf(buf, "%s %s %s %s %s", "Find The File, now Renaming...\n[กิ] Successfully Rename The File '", oldname, "' To Newname '", newname, "'.\n\n");
	  			send(sockfd, buf, MAX_SIZE, 0);
				printf("[msg] Renaming File Finish\n\n-----------\n");
			}
			break;
		}
	}
	//if can't find the file
	if(!find){
		memset(buf, '\0', MAX_SIZE);
		sprintf(buf, "%s", "[x] Sorry, failed to find this file.\n-----------\n");
		send(sockfd, buf, MAX_SIZE, 0);
		printf("[msg] Failed To Find This File!\n\n");
	}
	
	closedir(pDir);
	
}
