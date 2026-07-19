//server.c --> IPC - Sockets
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <string.h>

int main(){
	int socket_fd;
	socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if(socket_fd == -1){
		perror("socket");
		exit(EXIT_FAILURE);
	}else{
		printf("Socket created successfully\n");
	}
	
	struct sockaddr_un server_addr = {0};
	server_addr.sun_family = AF_UNIX;
	//server_addr.sun_path = "/tmp/server.sock";
	strncpy(server_addr.sun_path,"/tmp/server.sock", sizeof("/tmp/server.sock")-1);

	unlink("/tmp/server.sock");

	if(bind(socket_fd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("bind");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}
	printf("Socket bind successfully\n");
//	printf("Press Enter to exit ......\n");
//	getchar();
	
	if(listen(socket_fd, 5) == -1){
		perror("listen");
		close(socket_fd);
		unlink("/tmp/server.sock");
		exit(EXIT_FAILURE);
	}

	printf("Listening for incoming connections...\n");
	
	int client_fd;
	client_fd = accept(socket_fd,NULL, NULL );

	if(client_fd == -1){
		perror("accept");
		close(socket_fd);
		unlink("/tmp/server.sock");
		exit(EXIT_FAILURE);
	}
	printf("Client FD = %d\n", client_fd);
	printf("accept: Client connected successfully\n");
	
	char buffer[100];
	ssize_t bytes_received;
	bytes_received = recv(client_fd, buffer, sizeof(buffer)-1, 0);
	if(bytes_received == -1)
	{
		perror("recv");
		close(client_fd);
		close(socket_fd);
		unlink("/tmp/server.sock");
		exit(EXIT_FAILURE);
	}else if(bytes_received == 0){
		printf("Client Disconnected\n");
		close(client_fd);
	        close(socket_fd);
        	unlink("/tmp/server.sock");
		return 0;
	}else{
	
	buffer[bytes_received] = '\0';
	printf("Received from client: %s\n",buffer);
	}

	char reply[] = "Hello Client";
	ssize_t bytes_sent = send(client_fd, reply, strlen(reply), 0);
	if(bytes_sent == -1){
		perror("send");
		close(client_fd);
                close(socket_fd);
                unlink("/tmp/server.sock");
                return 0;
	}
	else{
		printf("Sent %zd bytes to client\n", bytes_sent);
	}
	close(client_fd);
	close(socket_fd);
	unlink("/tmp/server.sock");


	return 0;
}
