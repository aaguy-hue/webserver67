#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// note: explanations of almost all the includes here are in the server.c file
//
// unistd.h is the posix operating system api and it's not part of standard ANSI C
// unfortunately, the close() function can only be found here which makes this
// very nonportable since windows requires winsock2.h and closesocket() instead
// of close()

static const char *SERVER_ADDR = "127.0.0.1";
static const int SERVER_PORT = 8067;

int main() {
	int clientfd = socket(AF_INET, SOCK_STREAM, 0);
	if (clientfd < 0) {
		perror("[-] Failed to create client socket!");
		exit(EXIT_FAILURE);
	}
	printf("[+] Succesfully created client socket!\n");

	struct sockaddr_in client_addr_in;
	socklen_t addrlen = sizeof(client_addr_in);
	memset(&client_addr_in, 0, addrlen);
	client_addr_in.sin_family = AF_INET;
	client_addr_in.sin_port = htons(SERVER_PORT);
	int addr_conv_ok = inet_pton(AF_INET, SERVER_ADDR, &client_addr_in.sin_addr.s_addr);
	if (addr_conv_ok == 0) {
		printf("[-] Invalid IP address passed into inet_pton");
		exit(EXIT_FAILURE);
	} else if (addr_conv_ok < 0) {
		perror("[-] Failed to convert IP address to binary format w/ inet_pton");
		exit(EXIT_FAILURE);
	}

	struct sockaddr *client_addr = (struct sockaddr *)&client_addr_in;

	// connects the socket clientfd to the specified address
	if (connect(clientfd, client_addr, addrlen) < 0) {
		perror("[-] Failed to connect to server socket!");
		if (close(clientfd) < 0) {
			perror("[--] Failed to close client socket!");
		}
		exit(EXIT_FAILURE);
	}
	printf("[+] Successfully connected to server socket!\n");

	char msg[] = "Hello from client!";
	send(clientfd, msg, strlen(msg), 0);

	//char buffer[100];
	// recv(int sockfd, void buf[size], size_t size, int flags)
	// this will wait until it receives a message and store it in buf
	// if the message is too large for buf, parts of the msg will be discarded
	//int bytes_received = recv(clientfd, buffer, sizeof(buffer), 0);
	//if (bytes_received < 0) {
	//	perror("[-] Failed to receive message from server");
	//	exit(EXIT_FAILURE);
	//}
	//printf("[+] Successfully received %d bytes from server!\nServer message: %s\n", bytes_received, buffer);

	sleep(3);

	if (close(clientfd) < 0) {
		perror("[-] Failed to close client socket!");
	}
	printf("[+] Successfully closed client socket!\n");
	return 0;
}
