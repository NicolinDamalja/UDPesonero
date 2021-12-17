/*
 * UdpClient.c
 *
 *  Created on: 16 dic 2021
 *      Author: Windows 10
 */

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef WIN32
    #include <winsock.h>
#else
    #define closesocket close
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netdb.h>
#endif

#include "protocol.h"
enum bool{false = 0, true = 1};

char* getPort(char* s);

void setup_server(struct sockaddr_in * sad, char * name, int port);

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

    struct sockaddr_in server;

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    char *port = malloc(sizeof(char)*5);

    if (argc > 1 && argc <= MAX_ARGC) {
    	port = getPort(argv[1]);
        setup_server(&server, strtok(argv[1], ":"), atoi(port));
    } else {
        setup_server(&server, DEFAULT_SERVER, DEFAULT_PORT);
    }

    int server_size = sizeof(struct sockaddr_in);

    char buffer[BUFFER_SIZE];

    while(true) {

        bzero(buffer, BUFFER_SIZE);
        printf("Insert the operation: ");
        scanf("%[^\n]s", buffer);
        fflush(stdin);

        char *symbol = lookup_operator(buffer);
        if(symbol != NULL) {
            if(*symbol == '=') { exit(EXIT_SUCCESS); }
            sendto(sockfd, buffer, strlen(buffer), DEFAULT_BEHAVIOUR, (const struct sockaddr *) &server, server_size);
            bzero(buffer, BUFFER_SIZE);
            if (recvfrom(sockfd, buffer, BUFFER_SIZE, DEFAULT_BEHAVIOUR, (struct sockaddr *) &server, &server_size) > 0 ) {

                printf("Received result from Server %s, ip %s: %s\n", net_get_name(&server), inet_ntoa(server.sin_addr), buffer);
            }
        }

    }

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
	return port;
}

void setup_server(struct sockaddr_in * sad, char * name, int port) {
    bzero(sad, sizeof (struct  sockaddr_in));
    sad->sin_family = AF_INET;
    sad->sin_addr.s_addr = net_get_addr(name);
    sad->sin_port = htons(port);

    printf("Referring to %s at %s:%d\n", name, inet_ntoa(sad->sin_addr), ntohs(sad->sin_port));
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



char *lookup_operator(char *str) {
    for(int i = 0; i < strlen(str); i++) {
        if (member(str[i], allowed_operations)) {
            return &str[i];
        }
    }
    return NULL;
}

int member(char c, char *set) {
    for(int i = 0; i < strlen(set); i++) {
        if (c == set[i]) {
            return true;
        }
    }
    return false;
}
