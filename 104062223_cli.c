#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>  /* contains a number of basic derived types */
#include <winsock2.h> /* provide functions and data structures of socket */
#include <dirent.h>     /* format of directory entries */
#include <sys/stat.h>   /* stat information about files attributes */
#include <unistd.h>     /* `read()` and `write()` functions */
#define MAX_SIZE 1024
#define MAX_CONNECTION 3
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);

void Connection_handler(SOCKET sockfd);
void File_download_handler(SOCKET sockfd, char filename[]);
void File_upload_handler(SOCKET sockfd, char filename[]);

int main(int argc, char **argv)
{
	SOCKET clientSocket;
	struct sockaddr_in serverAddress;
	
	WSADATA wsadata;
    if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) {
        MY_ERROR("Winsock Error\n");
	}
	
	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		MY_ERROR("Create Socket Failed!\n");
	}
	
	memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]); // transform to 32-bit unsigned integer
    serverAddress.sin_port = htons(atoi(argv[2])); //converts a u_short from host to TCP/IP network byte order
	if (inet_pton(AF_INET, argv[1], &serverAddress.sin_addr) <= 0) {
     	MY_ERROR("Address converting fail with wrong address argument\n");
  	}
  	
  	if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    	MY_ERROR("Connect Failed!\n");
  	}
  	//Connection
	Connection_handler(clientSocket);

  	closesocket(clientSocket);
  	return 0;
}
void Connection_handler(SOCKET sockfd)
{
	char filename[MAX_SIZE];  // file want to download/upload
	char command[MAX_SIZE];	  // receive command 
    char buf[MAX_SIZE];       // store data from server/ send to server
    
    memset(buf, '\0', MAX_SIZE);
    recv(sockfd, buf, MAX_SIZE,0);
    printf("%s", buf);
    
    printf("Enter The Command: ");
    while(scanf("%s", command)>0){
    	if(strncmp(command,"exit",4)==0){
    		break;
		}else if(strncmp(command,"dir",3)==0){
			sprintf(buf, "%s", "dir");
			send(sockfd, buf, MAX_SIZE, 0);
			printf("-----------\nBelow Are File in ./upload:\n\n");
			memset(buf, '\0', MAX_SIZE);
			recv(sockfd, buf, MAX_SIZE,0);
			while(strcmp(buf,"o_v_e_r")){		//get until no file haven't visited
				//filename
				printf("%25s", buf);
				//filesize & file_last_modified_time
				memset(buf, '\0', MAX_SIZE);
				recv(sockfd, buf, MAX_SIZE, 0);
				printf("%s", buf);
				
				memset(buf, '\0', MAX_SIZE);
				recv(sockfd, buf, MAX_SIZE,0);
			}
			printf("\nFile List End.\n");
			memset(buf, '\0', MAX_SIZE);
			printf("\n-----------\nEnter The Command: ");
		}else if(strncmp(command, "get", 3)==0){
			if(send(sockfd, command, MAX_SIZE, 0)<0){
				MY_ERROR("Request send failed!\n")
			}
			scanf("%s", buf);		//enter filename
			send(sockfd, buf, MAX_SIZE, 0);
			File_download_handler(sockfd,buf);
			printf("-----------\nEnter The Command: ");
		}else if(strncmp(command, "put", 3)==0){
			if(send(sockfd, command, MAX_SIZE, 0)<0){
				MY_ERROR("Request send failed!\n")
			}
			scanf("%s", buf);		//enter filename
			send(sockfd, buf, MAX_SIZE, 0);
			File_upload_handler(sockfd,buf);
			printf("-----------\nEnter The Command: ");
		}else if(strncmp(command, "rename", 6)==0){
			if(send(sockfd, command, MAX_SIZE, 0)<0){
				MY_ERROR("Msg send ERROR\n");
			}
			scanf("%s", buf);	//oldname
			send(sockfd, buf, MAX_SIZE, 0);
			scanf("%s", buf);	//newname
			send(sockfd, buf, MAX_SIZE, 0);
			//get welcome msg
			memset(buf, '\0', MAX_SIZE);
			recv(sockfd, buf, MAX_SIZE, 0);
			printf("%s", buf);
			
			memset(buf, '\0', MAX_SIZE);
			//wait the successful/fail msg
			if(recv(sockfd, buf, MAX_SIZE, 0)<0){
				MY_ERROR("Msg recv ERROR\n");
			}else{
				printf("%s", buf);
			}
			printf("-----------\nEnter The Command: ");
		}else{
			printf("Invalid Command!\n");
			printf("-----------\nEnter The Command: ");
		}
	}
    printf("[x] Socket closed\n");
}
void File_download_handler(SOCKET sockfd, char filename[])
{
	char buf[MAX_SIZE];
	char path[MAX_SIZE];
	int file_size = 0;    //  size of this file
	int read_byte = 0;    //  bytes this time recv
	int read_sum = 0;     //  bytes have been recv
	FILE *fp;
	
	/* receive start message from server */
	memset(buf, '\0', MAX_SIZE);
  	recv(sockfd, buf, MAX_SIZE, 0);
  	if(strncmp(buf,"E_R_R_O_R",9)==0){
  		printf("Download failed(No this file or wrong path)\n");
		return;	
	}else{
  		printf("%s", buf);
  	}
  	
  	/* receive file size */
  	memset(buf, '\0', MAX_SIZE);
  	recv(sockfd, buf, MAX_SIZE, 0);
  	file_size = atoi(buf);
  	int packet_num, now_num = 0;	//split file into pkts
  	if(file_size>1024){
  		packet_num = file_size/1024 +1;
  		printf("\nThe File Is Over 1024 bytes.\n");
		printf("Split It Into %d Packets.\n\n", packet_num);	
	}else{
		packet_num = 1;
	}
  	
  	/* file path */
  	memset(path, '\0', MAX_SIZE);
  	sprintf(path, "./download/%s", filename);
  	
  	read_sum = 0;
  	fp = fopen(path, "wb");
  	if (fp) {
  		sleep(1);
  		printf("[0/%d] Packet Receiving...\n", packet_num);
      	while (read_sum < file_size) {
      		if(read_sum>1024*(now_num)){
      			printf("[%d/%d] Packet Receiving...\n", now_num+1, packet_num);
      			now_num++;
			}
        	memset(buf, '\0', MAX_SIZE);
			read_byte = recv(sockfd, buf, MAX_SIZE, 0);
        	/* write file to local disk*/
        	fwrite(&buf, sizeof(char), read_byte, fp);
        	read_sum += read_byte;
      	}
      	fclose(fp);
      	/* receive download complete message */
      	printf("[%d/%d] Packet receiving...\n", packet_num, packet_num);
      	memset(buf, '\0', MAX_SIZE);
      	recv(sockfd, buf, MAX_SIZE, 0);
      	printf("%s", buf);
  	}else{
    	MY_ERROR("Allocate memory fail\n");
  	}	
}
void File_upload_handler(SOCKET sockfd, char filename[])
{
	char fail_msg[] = "Upload failed(No this file or wrong path)";
  	char buf[MAX_SIZE];   // buffer to store msg
  	char path[MAX_SIZE];  // path to this file
  	FILE *fp;             // file want to upload
  	
  	int file_size = 0;    //  size of this file
  	int write_byte = 0;   //  bytes this time upload
  	int write_sum = 0;    //  bytes have been upload
  	
  	int packet_num, now_num = 0;
  	
  	sprintf(path, "upload/%s", filename);
  	fp = fopen(path, "rb");
  	
  	if(fp){
    	printf("[-] Uploading `%s` ...\n", filename);
    	
    	memset(buf, '\0', MAX_SIZE);
    	sprintf(buf, "%s", "[msg] Starting Receving...\n");
    	send(sockfd, buf, MAX_SIZE, 0);
    	
    	/* get file size, store in file_size */
    	fseek(fp, 0, SEEK_END);
    	file_size = ftell(fp);
    	rewind(fp);
    	
		/*send file size*/
    	memset(buf, '\0', MAX_SIZE);
    	sprintf(buf, "%d", file_size);
    	if (send(sockfd, buf, MAX_SIZE, 0) < 0) {
      		MY_ERROR("File size write failed!\n");
  		}
  		
  		if(file_size>1024){
	  		packet_num = file_size/1024 +1;
	  		printf("\nThe File Is Over 1024 bytes.\n");
			printf("Split It Into %d Packets.\n\n", packet_num);	
		}else{
			packet_num = 1;
		}
		
		/* read file data and upload to server */
    	write_sum = 0;
    	sleep(1);
    	printf("[0/%d] Packet Uploading...\n", packet_num);
    	while (write_sum < file_size) {
    		if(write_sum>1024*(now_num)){
      			printf("[%d/%d] Packet Uploading...\n", now_num+1, packet_num);
      			now_num++;
			}
      		/* read local file to buf */
      		memset(buf, '\0', MAX_SIZE);
      		write_byte = fread(&buf, sizeof(char), MAX_SIZE, fp);

			if (send(sockfd, buf, write_byte, 0) < 0) {
      			MY_ERROR("Data write failed!\n");
  			}

      		write_sum += write_byte;
    	}
		printf("[%d/%d] Packet Uploading...\n", packet_num, packet_num);
    	fclose(fp);
    	/* sleep for a while */
    	sleep(2);
    	printf("[กิ] Upload successfully!\n\n");

    	memset(buf, '\0', MAX_SIZE);
    	sprintf(buf, "%s", "[msg] Receiving Over.\n");
    	send(sockfd, buf, strlen(buf), 0);
  	} else {
    	/* fp is null*/
    	printf("%s\n", fail_msg);
    	send(sockfd, "E_R_R_O_R", MAX_SIZE, 0);
  	}
  	
}
