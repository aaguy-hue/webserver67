#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

	int connected_socket = connect(clientfd, client_addr, addrlen);
	if (connected_socket < 0) {
		perror("[-] Failed to connect to server socket!");
		exit(EXIT_FAILURE);
	}
	printf("[+] Successfully connected to server socket!\n");
	return 0;
}
