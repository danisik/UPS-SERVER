//
//	DRAUGHTS
//	VERSION 1.0.0
//
//	Copyright (c) 2010-2018 Dept. of Computer Science & Engineering,
//	Faculty of Applied Sciences, University of West Bohemia in Plzeň.
//	All rights reserved.
//
//	Code written by:	Vojtěch Danišík
//	Last update on:		21-12-2018
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include "header.h"

/*
 * Create array of clients
 * @param array_clients - array of logged clients
 */
void create_clients(clients **array_clients) {
	(*array_clients) = calloc(1, sizeof(clients));
	(*array_clients) -> clients_count = 0;
	(*array_clients) -> clients = calloc(1, sizeof(client));
	return;
}

/*
 * Create single client
 * @param cl - logging client
 * @param name - selected name of logging client
 * @param socket_ID - socket ID of logging client
 */
void create_client(client **cl, char *name, int socket_ID) {
	(*cl) = calloc(1, sizeof(client));
	(*cl) -> name = calloc(1, strlen(name) * sizeof(char));
	strcpy((*cl) -> name, name);
	(*cl) -> socket_ID = socket_ID;
	(*cl) -> state = 0;
	(*cl) -> connected = 1;
	(*cl) -> disconnected_time = 0;
	return;
}


/*
 * Adding client into array of clients
 * @param array_clients - array of logged clients
 * @param name - selected name of logging client
 * @param socket_ID - socket ID of logging client
 */
void add_client(clients **array_clients, char *name, int socket_ID) {
	(*array_clients) -> clients_count++;

	printf("Clients count: %d\n", (*array_clients) -> clients_count);
	(*array_clients) -> clients = realloc((*array_clients) -> clients, (*array_clients) -> clients_count * sizeof(client));
	client *client = NULL;
	create_client(&client, name, socket_ID);
	(*array_clients) -> clients[((*array_clients) -> clients_count) - 1] = client;
	return;
}

/*
 * Removing client from array of clients
 * @param array_clients - array of logged clients
 * @param wanna_play - array of clients who wants to play a game
 * @param socket_ID - socket ID of logging client
 */	
void client_remove(clients **array_clients, wanna_play **wanna_play, int socket_ID) {
	int i;
	int count = (*array_clients) -> clients_count;
	int socket;	
	for (i = 0; i < count; i++) {
		socket = (*array_clients) -> clients[i] -> socket_ID;
		if (socket == socket_ID) {
			remove_wanna_play(wanna_play, socket_ID);
			(*array_clients) -> clients_count--;			
			if (i < (count - 1)) {
				free((*array_clients) -> clients[i]);
				(*array_clients) -> clients[i] = (*array_clients) -> clients[((*array_clients) -> clients_count)];								
			}
			(*array_clients) -> clients[((*array_clients) -> clients_count)] = NULL;			
			(*array_clients) -> clients = realloc((*array_clients) -> clients, (*array_clients) -> clients_count * sizeof(client));

			printf("Client left, actually logged clients: %d\n", (*array_clients) -> clients_count);	
			return;
		}
	}
	return;
}

/*
 * Return client based on sent socket_ID
 * @param array_clients - array of logged clients
 * @param socket_ID - socket ID of logging client
 * @return client if founded or NULL
 */	
client *get_client_by_socket_ID(clients *array_clients, int socket_ID) {
	int i;
	int count = array_clients -> clients_count;
	int socket;	
	for (i = 0; i < count; i++) {
		socket = array_clients -> clients[i] -> socket_ID;
		if (socket == socket_ID) {
			return array_clients -> clients[i];
		}
	}
	return NULL;
}

/*
 * Return client based on set name
 * @param array_clients - array of logged clients
 * @param name - name of client
 * @return client if founded or NULL
 */	
client *get_client_by_name(clients *array_clients, char *name) {
	int i;
	int count = array_clients -> clients_count;	
	char *nam;
	for (i = 0; i < count; i++) {
		nam = array_clients -> clients[i] -> name;
		if (strcmp(nam, name) == 0) {
			return array_clients -> clients[i];
		}
	}
	return NULL;
}


/*
 * Set state of client
 * @param client - client
 * @param state - state which client must be
 */	
void set_state(client **client, int state) {
	(*client) -> state = state;
	return;
}


/*
 * Set color of client
 * @param client - client
 * @param color - color of clients assigned by server
 */	
void set_color(client **client, char *color) {
	strcpy((*client) -> color, color);
	return;
}

/*
 * Set connected attribute
 * @param client - client
 * @param connected - set number indicates if client is connected
 */	
void set_connected(client **cl, int connected) {
	(*cl) -> connected = connected;
	return;
}

/*
 * Set socket id
 * @param client - client
 * @param socket_ID - socket id of client
 */	
void set_socket_ID(client **cl, int socket_ID) {
	(*cl) -> socket_ID = socket_ID;
	return;
}

/*
 * Set disconnected time
 * @param client - client
 * @param disconnected_time - set time indicates how many seconds are client disconnected
 */	
void set_disconnected_time(client **cl, int disconnected_time) {
	(*cl) -> disconnected_time = disconnected_time;
	return;
}
