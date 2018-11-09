#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
// kvuli iotctl
#include <sys/ioctl.h>
#include "header.h"



void create_clients(clients **array_clients) {
	(*array_clients) = calloc(1, sizeof(clients));
	(*array_clients) -> clients_count = 0;
	(*array_clients) -> clients = calloc(1, sizeof(client));
}

void create_client(client **cl, char *name, int socket_ID) {
	(*cl) = calloc(1, sizeof(client));
	(*cl) -> name = calloc(1, strlen(name) * sizeof(char));
	strcpy((*cl) -> name, name);
	(*cl) -> socket_ID = socket_ID;
}

void add_client(clients **array_clients, char *name, int socket_ID) {
	(*array_clients) -> clients_count++;
	printf("Client count: %d\n", (*array_clients) -> clients_count);
	(*array_clients) -> clients = realloc((*array_clients) -> clients, (*array_clients) -> clients_count * sizeof(client));
	client *client = NULL;
	create_client(&client, name, socket_ID);
	(*array_clients) -> clients[((*array_clients) -> clients_count) - 1] = client;
}
	
void client_remove(clients **array_clients, int socket_ID) {
	int i;
	int count = (*array_clients) -> clients_count;
	int socket;	
	for (i = 0; i < count; i++) {
		socket = (*array_clients) -> clients[i] -> socket_ID;
		if (socket == socket_ID) {
			(*array_clients) -> clients_count--;			
			if (i < (count - 1)) {
				free((*array_clients) -> clients[i]);
				(*array_clients) -> clients[i] = (*array_clients) -> clients[((*array_clients) -> clients_count)];									
			}
			(*array_clients) -> clients[((*array_clients) -> clients_count)] = NULL;			
			(*array_clients) -> clients = realloc((*array_clients) -> clients, (*array_clients) -> clients_count * sizeof(client));
		}
		return;
	}
}
