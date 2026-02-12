#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

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

// rfs 9110 is the modern rfc defining the http spec

#define PORT 8067

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
	bool opt = true;
	if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
				&opt, sizeof(opt)) < 0) {
		perror("[-] setting socket options failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	socklen_t addrlen = sizeof(server_addr);
	memset(&server_addr, 0, addrlen);
	server_addr.sin_family = AF_UNIX;
	//server_addr.sin_addr.s_addr = INADDR_ANY; // this is just 0.0.0.0
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(PORT);
	bind(serverfd, (struct sockaddr *)&server_addr, addrlen);
}
