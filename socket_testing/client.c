#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
	client_addr_in.sin_addr.s_addr = inet_addr(SERVER_ADDR);

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

	if (close(clientfd) < 0) {
		perror("[-] Failed to close client socket!");
	}
	printf("[+] Successfully closed client socket!\n");
	return 0;
}
