#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "hashmap.h"
#include "fields.h"
#include "startline.h"
#include "request.h"
#include "config.h"
#include "response.h"
#include "headers.h"
#include "util.h"

#define CHECK(x, msg) \
	do { \
		if ((x) < 0) { \
			fprintf(stderr, "[%s:%d] %s failed\n", __FILE__, __LINE__, #x); \
			perror(msg); \
			status = EXIT_FAILURE; \
			goto cleanup; \
		} \
	} while (0)

#define BUFFSIZE 1000

// rfs 9110 is the modern rfc defining the http spec
static const int BACKLOG_LENGTH = 2;

static volatile bool keepRunning = true;

void ctrlCHandler(int) {
	keepRunning = false;
}

int main() {
	int status = EXIT_SUCCESS;
	int serverfd = -1;
	int clientfd = -1;
	
	// allocate memory for the request and response objects
	HttpRequest request;
	memset(&request, 0, sizeof(HttpRequest));

	HttpResponse *response = initializeResponse();
	if (response == NULL || response->statusLine == NULL) {
		status = EXIT_FAILURE;
		goto cleanup;
	}

	//ServerConfig *cfg = malloc(sizeof(ServerConfig));
	//memset(cfg, 0, sizeof(ServerConfig));
	ServerConfig *cfg = readConfig("./config.yml");
	if (cfg == NULL) {
		status = EXIT_FAILURE;
		goto cleanup;
	}
	printf("[+] Successfully loaded YAML configuration!\n");

	serverfd = socket(AF_INET, SOCK_STREAM, 0);
	CHECK(serverfd, "[-] Failed to create socket :((");
	printf("[+] yay our server socket was created!!!\n");

	int enable = true;
	int opt1ok = setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	// int opt2ok = setsockopt(serverfd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	CHECK(opt1ok, "[-] Setting server socket option SO_REUSEADDR failed");
	// CHECK(opt2ok, "[-] Setting server socket option SO_REUSEPORT failed");
	printf("[+] set server socket options!\n");

	struct sockaddr_in server_addr_in;
	socklen_t addrlen = sizeof(server_addr_in);
	memset(&server_addr_in, 0, addrlen);
	server_addr_in.sin_family = AF_INET;
	server_addr_in.sin_port = htons(cfg->port);
	//server_addr_in.sin_addr.s_addr = INADDR_ANY; // this is just 0.0.0.0
	int addrok = inet_pton(AF_INET, cfg->address, &server_addr_in.sin_addr.s_addr);
	if (addrok == 0) {
		printf("[-] Invalid IP address passed into inet_pton!\n");
		status = EXIT_FAILURE;
		goto cleanup;
	}
	CHECK(addrok, "[-] Failed to convert IP address to binary with inet_pton");

	struct sockaddr *server_addr = (struct sockaddr *)&server_addr_in;

	// bind server to the port

	char errorstr[100];
	snprintf(errorstr, sizeof(errorstr), "[-] failed to bind server socket to %s:%d", cfg->address, cfg->port);
	CHECK(bind(serverfd, server_addr, addrlen), errorstr);

	char successtr[100];
	snprintf(successtr, sizeof(successtr), "[+] succesfully bound server socket to %s:%d\n", cfg->address, cfg->port);
	printf(successtr);

	CHECK(listen(serverfd, BACKLOG_LENGTH), "[-] Failed to set server socket to listen for connections");
	printf("[+] Server socket is now listening for connections!\n");

	signal(SIGINT, ctrlCHandler);

	do {
		// Note: consider using accept4() on supporting platforms
		struct sockaddr_in client_addr;
		socklen_t addrsize = sizeof(client_addr);
		clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &addrsize);
		CHECK(clientfd, "[-] Failed to acccept client connection attempt");

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
		printf("[+] Client connected from %s:%d!\n\n", client_ip, ntohs(client_addr.sin_port));

		char buf[BUFFSIZE];
		int n;
		while (keepRunning) {
			memset(buf, 0, sizeof(buf));
			// note: I do sizeof(buf) - 1 since I need to set the last byte
			// to the null byte manually bc recv doesn't automatically add it
			n = recv(clientfd, buf, sizeof(buf)-1, MSG_CMSG_CLOEXEC | MSG_DONTWAIT);
			if (n == -1) {
				// no data received
			} else if (n == 0) {
				// connection has been closed
				printf("Client closed connection! Breaking...\n");
				break;
			} else {
				// n is the number of bytes read
				printf("Read %d bytes!\nMessage: \n%s\n\n\n", n, buf);
				buf[n] = '\0';
				break;
			}
		}
		if (!keepRunning) break;

		char *bufptr = buf;

		RequestLine requestLine = getRequestLine(&bufptr);
		if (requestLine.method == INVALID_METHOD || requestLine.version == INVALID_VERSION) {
			status = EXIT_FAILURE;
			goto cleanup;
		}

		request.requestLine = &requestLine;
		printf("[/] HTTP Method: %d\n", requestLine.method);
		printf("[/] HTTP Target: %s\n", requestLine.target);
		printf("[/] HTTP Version: %d\n", requestLine.version);

		//printf("[/] Buffer first byte: %d\n", buf[0]);
		struct hashmap *headers = readRequest(&bufptr);
		request.headers = headers;

		const char *contentLengthStr = popHeader(headers, "content-length");
		int contentLength = contentLengthStr && strIsNumeric(contentLengthStr) ? atoi(contentLengthStr) : 0;
		strncpy(request.content, bufptr, minInt(CONTENT_MAXLEN-1, contentLength));
		request.content[CONTENT_MAXLEN-1] = '\0';

		printf("\nContent:\n%s\n", request.content);

		generateResponse(response, &request, cfg);
		printf("[+] Response generated!\n");


		sendStatusLine(response, clientfd);
		sendHeaders(response, clientfd);
		sendBody(response, clientfd);
		printf("Sent response!\n");



		memset(&request, 0, sizeof(HttpRequest));
		memset(response->statusLine, 0, sizeof(StatusLine));
		// note: don't memset response to 0 bc then it zeros the struct and statusLine pointer and we need to free those later
		//memset(response, 0, sizeof(HttpResponse));

		if (request.headers != NULL) {
			hashmap_free(request.headers);
		}
		if (response->headers != NULL) {
			hashmap_free(response->headers);
		}

		if (clientfd > -1) {
			if (close(clientfd) < 0) {
				perror("[-] Failed to close client socket!");
			}
			printf("[+] Closed client socket!\n");
		}
	} while (keepRunning);

cleanup:
	free(cfg);
	if (request.headers != NULL) {
		hashmap_free(request.headers);
	}
	freeResponse(response);
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
	return status;
}
