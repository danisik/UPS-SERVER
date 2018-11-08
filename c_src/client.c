#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
// kvuli iotctl
#include <sys/ioctl.h>
#include "structures.h"
#include "messages.h"



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
	//printf("Client created\n");
}

void add_client(clients **array_clients, char *name, int socket_ID) {
	(*array_clients) -> clients_count++;
	(*array_clients) -> clients = realloc((*array_clients) -> clients, (*array_clients) -> clients_count * sizeof(client));
	client *client = NULL;
	create_client(&client, name);
	(*array_clients) -> clients[((*array_clients) -> clients_count) - 1] = client;
	//printf("Client added into array\n");
}
