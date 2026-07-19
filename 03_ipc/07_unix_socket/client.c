//Client.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

int main(){

	int sockfd;
	sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

	if(sockfd == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("Socket created successfully\n");

	struct sockaddr_un server_addr = {0};
	server_addr.sun_family = AF_UNIX;
	strncpy(server_addr.sun_path, "/tmp/server.sock", sizeof(server_addr.sun_path)-1);

	if(connect(sockfd,
		  (struct sockaddr *)&server_addr,
		  sizeof(server_addr)) == -1)
	{
		perror("connect");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	printf("Connected to server successfully\n");
	
	char msg[] = "Hello Server";
	ssize_t bytes_sent;
	bytes_sent = send(sockfd, msg, strlen(msg), 0);
	if(bytes_sent == -1){
		perror("send");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	printf("Sent %zd bytes \n", bytes_sent);
	
	char buffer[100];
	ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer) -1, 0);
	if(bytes_received == -1)
	{
    		perror("recv");
		close(sockfd);
	 	exit(EXIT_FAILURE);
	}
	else if(bytes_received == 0)
	{
   		printf("Server disconnected\n");
	}
	else
	{
    		buffer[bytes_received] = '\0';
    		printf("Received from server : %s\n", buffer);
	}
	
	close(sockfd);
	return 0;
}
