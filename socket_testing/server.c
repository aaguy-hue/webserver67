#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// an explanation of each include:
// stdio is obviously io
//
// stdlib has a bunch of general purpose things like malloc/free and
// exit/EXIT_FAILURE
//
// stdbool has definitions true and false, and sets bool to _Bool
//
// errno is set by certain lib functions and syscalls including setsockopt()
// see man errno, you can run the errno <int> command to see what an error means
// i didn't include errno though since perror will say the meaning of the value
// set by errno
//
// netinet/in.h includes sockaddr_in, if I wanted sockaddr_un I'd use sys/un.h
//
// arpa/inet.h provides inet_addr() function, it would be winsock2.h or
// winsock.h on windows
//
// sys/socket provides the function: int socket(int domain, int type, int
// protocol), run `man socket`
// sys/socket also provides socket address families used for the domain parameter, see
// `man address_families`
// bind() binds the socket to an address
// listen() tells it that new connections will be accepted
// accept() gets a new socket with a new incoming connection
// on the client, connect() connects a socket to a remote address
// ok wait I'm just summarizing the manpage for socket(7) at this point so just
// read that if you forgot
//
// unistd is explained in client.c but it just provides close() and is for
// posix-specific things
//
// time.h provides sleep() on posix-compliant systems
// annoyingly, sleep's param is in seconds on *nix systems but ms on windows



// rfs 9110 is the modern rfc defining the http spec

static const char *ADDRESS = "127.0.0.1";
static const int PORT = 8067;
static const int BACKLOG_LENGTH = 2;

int main() {
	char* msg = "Hello! This is the server.";

	// int socket(int domain, int socket, int protocol)
	// AF_INET -> using ipv4
	// AF_LOCAL -> communicating on same device, for testing rn
	// SOCK_STREAM -> tcp bc it's involving an actual connection
	// SOCK_DGRAM -> udp
	// protocol is the protocol field of the ip header of the packet,
	// usually there's only one option you can use so you set 0
	int serverfd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverfd < 0) {
		perror("[-] failed to create socket :(((");
		exit(EXIT_FAILURE);
	}
	printf("[+] yay our server socket was created!!!\n");

	// int setsockopt(int sockfd, int level, int optname,
	// const void optval[optlen], socklen_t optlen)
	// sockfd -> our socket file descripter returned by socket()
	// level -> options exist at multiple protocol levels, always available at
	// uppermost socket level, SOL_SOCKET is for options at the socket level.
	//
	int enable = true;
	if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&enable, sizeof(enable)) < 0) {
		perror("[-] Setting server socket options failed");
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	printf("[+] set server socket options!\n");

	// set the server's address
	struct sockaddr_in server_addr_in;
	socklen_t addrlen = sizeof(server_addr_in);
	memset(&server_addr_in, 0, addrlen);
	server_addr_in.sin_family = AF_INET;
	//server_addr_in.sin_addr.s_addr = INADDR_ANY; // this is just 0.0.0.0
	// note: i used to use inet_addr, but inet_pton is the modern version which
	// is better bc it's thread-safe, also supports ipv6
	// but it is less versatile in its input than inet_addr()
	int addrok = inet_pton(AF_INET, ADDRESS, &server_addr_in.sin_addr.s_addr);
	if (addrok == 0) {
		printf("[-] Invalid IP address passed into inet_pton!");
		exit(EXIT_FAILURE);
	} else if (addrok < 0) {
		perror("[-] Failed to convert IP address to binary with inet_pton");
		exit(EXIT_FAILURE);
	}
	//server_addr_in.sin_addr.s_addr = inet_addr(ADDRESS);
	server_addr_in.sin_port = htons(PORT);

	struct sockaddr *server_addr = (struct sockaddr *)&server_addr_in;

	// bind server to the port
	if (bind(serverfd, server_addr, addrlen) < 0) {
		char errorstr[100];
		sprintf(errorstr, "[-] failed to bind server socket to %s:%d", ADDRESS, PORT);
		perror(errorstr);
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	char successtr[100];
	sprintf(successtr, "[+] succesfully bound server socket to %s:%d\n", ADDRESS, PORT);
	printf(successtr);

	// listen() marks the socket refered to by sockfd as a passive listening
	// port, which is what you'd want on a server
	// it allows us to use accept() later
	// 2nd param is backlog, which is the max length of the queue of pending
	// connections for sockfd
	if (listen(serverfd, BACKLOG_LENGTH) < 0) {
		perror("[-] Failed to set server socket to listen for connections!");
		if (close(serverfd) < 0) {
			perror("[--] Failed to close server socket!");
		}
		exit(EXIT_FAILURE);
	}
	printf("[+] Server socket is now listening for connections!\n");

	// accept() extracts the first connection in the queue from the socket
	// it creates a new socket and returns that (it doesn't modify the original)
	// the 2nd/3rd params are the client info, but we leave that null
	// accept4() is the same thing but it has a 4th parameter, flags, which is
	// useful in multithreaded code bc it lets you have nonblocking calls and
	// enhanced security
	// if you specify no flags then accept4() is the same as accept()
	// accept4() is a nonstandard extension and isn't POSIX-compliant
	int clientfd = accept(serverfd, NULL, NULL);
	printf("[+] Established connection with client!\n");

	// send(int sockfd, const void buf[size], size_t size, int flags)
	// here I set flags to 0 since none are relevant for this case
	sleep(1);
	char message[] = "Hello from server!";
	int numBytes = send(clientfd, message, sizeof(message), 0);
	if (numBytes < 0) {
		perror("[-] Failed to send message to client!");
		exit(EXIT_FAILURE);
	}
	printf("[+] Successfully sent %d bytes to the client!\n", numBytes);

	sleep(3);

	if (close(serverfd) < 0) {
		perror("[-] Failed to close server socket!");
	}
	printf("[+] Closed server socket!\n");
}
