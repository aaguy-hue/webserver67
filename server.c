#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// rfs 9110 is the modern rfc defining the http spec

static const char *ADDRESS = "127.0.0.1";
static const int PORT = 8067;
static const int BACKLOG_LENGTH = 2;

int main() {
	int serverfd = -1;
	int clientfd = -1;

	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd < 0) {
		perror("[-] failed to create socket :(((");
		exit(EXIT_FAILURE);
	}
	printf("[+] yay our server socket was created!!!\n");

	int enable = true;
	int opt1ok = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	int opt2ok = setsockopt(serverfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	if (opt1ok < 0 || opt2ok < 0) {
		perror("[-] Setting server socket options failed");
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	printf("[+] set server socket options!\n");

	struct sockaddr_in server_addr_in;
	socklen_t addrlen = sizeof(server_addr_in);
	memset(&server_addr_in, 0, addrlen);
	server_addr_in.sin_family = AF_INET;
	//server_addr_in.sin_addr.s_addr = INADDR_ANY; // this is just 0.0.0.0
	int addrok = inet_pton(AF_INET, ADDRESS, &server_addr_in.sin_addr.s_addr);
	if (addrok == 0) {
		printf("[-] Invalid IP address passed into inet_pton!");
		exit(EXIT_FAILURE);
	} else if (addrok < 0) {
		perror("[-] Failed to convert IP address to binary with inet_pton");
		exit(EXIT_FAILURE);
	}
	server_addr_in.sin_port = htons(PORT);

	struct sockaddr *server_addr = (struct sockaddr *)&server_addr_in;

	// bind server to the port
	if (bind(serverfd, server_addr, addrlen) < 0) {
		char errorstr[100];
		snprintf(errorstr, sizeof(errorstr), "[-] failed to bind server socket to %s:%d", ADDRESS, PORT);
		perror(errorstr);
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	char successtr[100];
	snprintf(successtr, sizeof(successtr), "[+] succesfully bound server socket to %s:%d\n", ADDRESS, PORT);
	printf(successtr);

	if (listen(serverfd, BACKLOG_LENGTH) < 0) {
		perror("[-] Failed to set server socket to listen for connections!");
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	printf("[+] Server socket is now listening for connections!\n");

	// Note: consider using accept4() on supporting platforms
	struct sockaddr_in client_addr;
	socklen_t addrsize = sizeof(client_addr);
	clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &addrsize);
	if (clientfd < 0) {
		perror("[-] Failed to accept client connection attempt");
		close(serverfd);
		exit(EXIT_FAILURE);
	}

	char client_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
	printf("[+] Client connected from %s:%d!\n", client_ip, ntohs(client_addr.sin_port));







cleanup:
	if (clientfd > -1) {
		if (close(clientfd) < 0) {
			perror("[-] Failed to close client socket!");
		}
		printf("[+] Closed client socket!\n");
	}

	if (serverfd > -1) {
		if (close(serverfd) < 0) {
			perror("[-] Failed to close server socket!");
		}
		printf("[+] Closed server socket!\n");
	}
}
