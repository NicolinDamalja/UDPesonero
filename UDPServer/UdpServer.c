/*
 * UdpServer.c
 *
 *  Created on: 16 dic 2021
 *      Author: Windows 10
 */


#ifdef WIN32
#include <winsock.h>
#else
    #define closesocket close
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
#endif

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "protocol.h"

enum bool{false = 0, true = 1};

char* getPort(char* s);

int init_server(struct sockaddr_in * sad, char * ip, int port);
void handle_client(int sockfd);
void close_server(int server);

operation parse_operation(char * str);
result compute_operation(operation op);
void encode_results(operation op, result r, char *out);

int member(char c, char *set);
char *lookup_operator(char * str);


// DNS functions & net functions
char *net_get_name(struct sockaddr_in *sad);
unsigned long net_get_addr(char *name);


int main(int argc, char **argv) {

    #if defined WIN32
        // Initialize Winsock
        WSADATA wsa_data;
        int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
        if (result != NO_ERROR) {
            perror("Error at WSAStartup()\n");
            return 0;
        }
    #endif

    struct sockaddr_in server_andress;
    int sockfd;
    char *port;

    if (argc > 1 && argc <= MAX_ARGC) {
    	port = getPort(argv[1]);
        sockfd = init_server(&server_andress, strtok(argv[1], ":"), atoi(port));
    } else {
        sockfd = init_server(&server_andress, DEFAULT_SERVER, DEFAULT_PORT);
    }

    handle_client(sockfd);
    return EXIT_SUCCESS;
}

char* getPort(char* s){
	char* str = malloc(sizeof(char)*25);
	strcpy(str, s);
	char* token = strtok(str,":");
	char* port = malloc(sizeof(char)*5);
	int i=0;
	while(token!=NULL){
		if (i==1){
			strcpy(port,token);
		}
		i++;
		token=strtok(NULL,":");
	}
	free(str);
	return port;
}

int init_server(struct sockaddr_in * sad, char * name, int port) {
    int server;
    bzero(sad, sizeof (struct  sockaddr_in));
    sad->sin_family = AF_INET;
    sad->sin_addr.s_addr = net_get_addr(name);
    sad->sin_port = htons(port);

    if ( (server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if ( bind(server, (struct sockaddr*) sad, sizeof(struct sockaddr_in)) < 0) {
        perror("Bind failed");
        close_server(server);
        exit(EXIT_FAILURE);
    }

    printf("Server %s started at %s:%d\n", name, inet_ntoa(sad->sin_addr), ntohs(sad->sin_port));
    return server;
}


void close_server(int socket) {
    closesocket(socket);
    #ifdef WIN32
        WSACleanup();
    #endif
}

void handle_client(int sockfd) {

    struct sockaddr_in client;
    int client_size = sizeof(struct sockaddr_in);
    bzero(&client, client_size);

    char buffer[BUFFER_SIZE];


    while (true)
    {
        bzero(buffer, BUFFER_SIZE);
        if (recvfrom(sockfd, buffer, BUFFER_SIZE, DEFAULT_BEHAVIOUR, ( struct sockaddr *) &client, &client_size) > 0 ) {
            operation op = parse_operation(buffer);
            result r = compute_operation(op);

            printf("Requested operation '%c %d %d' from client %s, ip %s\n",
                    op.symbol,
                    op.first,
                    op.second,
                    net_get_name(&client),
                    inet_ntoa(client.sin_addr)
                );

            bzero(buffer, BUFFER_SIZE);
            encode_results(op, r, buffer);

            sendto(sockfd, buffer, strlen(buffer), DEFAULT_BEHAVIOUR, (const struct sockaddr *) &client, client_size);
        }
    }

}

operation parse_operation(char * str) {

    operation op;

    if(str != NULL) {
        char *token = lookup_operator(str);

        if(token != NULL) {
            op.symbol = *token;

            do {
                token = token + 1;
            } while (*token != ' ' && *token != '\0');

            op.first = atoi(strtok(token, " "));
            op.second = atoi(strtok(NULL, " "));

            return op;
        }
    }

    op.symbol = '!';
    op.first = 0;
    op.second = 0;
    return op;

}

result compute_operation(operation op) {
    result r;
    r.type = TYPE_FLOAT;
    r.real = NAN;

    switch (op.symbol) {
        case '+':
            return add(op.first, op.second);
        case '-':
            return sub(op.first, op.second);
        case 'x':
            return mult(op.first, op.second);
        case '/':
            return division(op.first, op.second);
        default:
            return r;
    }
}

result add(int a, int b) {
    result r;
    r.type = TYPE_INT;
    r.integer = a + b;
    return r;
}

result sub(int a, int b) {
    result r;
    r.type = TYPE_INT;
    r.integer = a - b;
    return r;
}

result mult(int a, int b) {
    result r;
    r.type = TYPE_INT;
    r.integer = a * b;
    return r;
}

result division(int a, int b) {
    result r;
    r.type = TYPE_FLOAT;
    r.real = b != 0 ? (float) a / (float) b : INFINITY;

    return r;
}

int member(char c, char *set) {
    for(int i = 0; i < strlen(set); i++) {
        if (c == set[i]) {
            return true;
        }
    }
    return false;
}


char* lookup_operator(char *str) {

    for(int i = 0; i < strlen(str); i++) {
        if (member(str[i], allowed_operations)) {
            return &str[i];
        }
    }

    return NULL;
}

void encode_results(operation op, result r, char *out) {

    snprintf(out, BUFFER_SIZE - 1, "%d %c %d = ", op.first, op.symbol, op.second);
    size_t p = strlen(out);
    out = &out[p];

    if (r.type == TYPE_INT) {
        snprintf(out, BUFFER_SIZE - p - 1, "%d", r.integer);
        return;
    }

    if (isnan(r.real)) {
        snprintf(out, BUFFER_SIZE - p - 1, "%s", "NaN");
        return;
    }

    if (r.real == INFINITY) {
        snprintf(out, BUFFER_SIZE - p - 1, "%s", "Inf");
        return;
    }

    snprintf(out, BUFFER_SIZE - p - 1,  "%.2f", r.real);
}


unsigned long net_get_addr(char *name) {
    struct hostent *hent = gethostbyname(name);
    unsigned long addr = 0;

    if(hent != NULL) {
        addr = ((struct in_addr *) hent->h_addr)->s_addr;
        //freehostent(hent);
    }

    return addr;

}

char *net_get_name(struct sockaddr_in *sad) {

    struct hostent *hent = gethostbyaddr(&(sad->sin_addr.s_addr), sizeof(unsigned long), AF_INET);
    if(hent != NULL) {
        return hent->h_name;
    }
    return NULL;
}

