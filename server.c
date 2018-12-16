#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include "header.h"

void sigint_handler(int sig);
log_info *info; 
char *filename = "log.txt";

struct timeval server_start;
struct timeval server_end;

int main(int argc, char *argv[])
{
	int port = 10000;
	game_info();
	if (argc == 2) {
		if (strcmp(argv[0], "-port") == 0 || strcmp(argv[0], "-p") == 0) port = atoi(argv[1]);
	}
	int max_players = 32;

	remove(filename);

	clients *array_clients;	
	create_clients(&array_clients);

	games *all_games;
	create_games(&all_games);

	wanna_play *wanna_plays;
	create_wanna_play(&wanna_plays);

	struct timeval client_timeout;
	client_timeout.tv_sec = 300; //5 min
	//client_timeout.tv_sec = 10; //10 sec

	info = calloc(1, sizeof(info));

	int count_bytes = 0;
	int count_messages = 0;
	int count_connections = 0;
	int count_bad_transmissions = 0;

	int server_socket;
	int client_socket, fd;
	int return_value;
	int cbuf_size = 1024;
	char cbuf[cbuf_size];
	int len_addr;
	int a2read;
	struct sockaddr_in my_addr, peer_addr;
	fd_set client_socks, tests;
	
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	int param = 1;
        return_value = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int));

	gettimeofday(&server_start, NULL);	

	if (return_value == -1)	printf("setsockopt ERR\n");

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));

	if (return_value == 0) printf("Bind - OK\n");
	else {
		printf("Bind - ERR\n");
		return -1;
	}

	return_value = listen(server_socket, 5);
	if (return_value == 0) {
		printf("Listen - OK\n");
	} else {	
		printf("Listen - ERR\n");
	}
	FD_ZERO(&client_socks);
	FD_SET(server_socket, &client_socks);
	while(1) {
		tests = client_socks;
		return_value = select(FD_SETSIZE, &tests, (fd_set*)NULL, (fd_set*)NULL, &client_timeout);
		signal(SIGINT, sigint_handler);	
		client *cl = get_client_by_socket_ID(array_clients, fd);
		if (return_value == 0) {
			delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, 0, &cl);
		}
		else if (return_value > 0) {
			for (fd = 3; fd < FD_SETSIZE; fd++) {
				if (FD_ISSET(fd, &tests)) {
					if (fd == server_socket) {
						client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
						FD_SET(client_socket, &client_socks);
						info -> count_connections++;
					}
					else {
						int int_ioctl = ioctl(fd, FIONREAD, &a2read);
						if (int_ioctl >= 0) {	
							if (a2read > 0) {
								int int_recv = recv(fd, &cbuf, cbuf_size*sizeof(char), 0);	
								if (recv == 0) {
									disconnect(&array_clients, &info, all_games, fd, &cl);
									continue;
								}
								char *tok = strtok(cbuf, ";");
								char *type_message = tok;
								cl = get_client_by_socket_ID(array_clients, fd);
								if (strcmp(type_message, "login") == 0) {
									login(&array_clients, all_games, &info, tok, max_players, fd, &cl);
								}
								else if (strcmp(type_message, "play") == 0) {
									if (cl != NULL) {
										if (cl -> state == 0) {
											play(&array_clients, &wanna_plays, &all_games, &info, fd, &cl);
										}
									}
								}
								else if (strcmp(type_message, "client_move") == 0) {
									if (cl != NULL) {
										if (cl -> state == 3) {
											client_move(&all_games, array_clients, &info, tok);
										}
									}							
								}
								else if (strcmp(type_message, "new_game_no") == 0) {
									if (cl -> state == 3 || cl -> state == 4) {
										client_remove(&array_clients, &wanna_plays, fd);
									}
								}
								else if (strcmp(type_message, "app_end") == 0) {
									delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, 1, &cl);
								}
								else {
									//spatny prikaz
								}	
													
							}
							else {
								//delete(&array_clients, &wanna_plays, &client_socks, &all_games, &info, fd, "end_game_left;\n", &cl);
							}						
						}
						else {
							printf("Ioctl failed and returned: %s\n",strerror(errno));
						}
					}
				} 
			}
		}	
		else {
			printf("\nBRUTAL FATAL ERROR!\n");
			return -1;
		}
	}
	return 0;
}

void sigint_handler(int sig) {
	gettimeofday(&server_end, NULL);
	server_running(server_start, server_end, &info);
	log_all(filename, info);
	_exit(1);
	return;
}

//return: 0 - not contains name
//	  1 - contains name
int name_exists (clients *array_clients, char *name) {
	int i;
	for (i = 0; i < array_clients -> clients_count; i++) {
		if (strcmp(array_clients -> clients[i] -> name, name) == 0) {	
			return 1;
		}
	}
	return 0;
}

void send_message(int client_socket, char *message, log_info **info) {	
	printf("Message: %s\n", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
	
	(*info) -> count_bytes += strlen(message) + 1;
	(*info) -> count_messages++;
	return;
}


void login(clients **array_clients, games *all_games, log_info **info, char *tok, int max_players, int fd, client **client) {
	tok = strtok(NULL, ";");
	char *name = tok;
	if (name_exists((*array_clients), name) == 0) {								
		if (((*array_clients) -> clients_count) < (max_players)) {			
			if ((*client) == NULL) {
				add_client(array_clients, name, fd);
				(*client) = (*array_clients) -> clients[(*array_clients) -> clients_count - 1];
				printf("Name: %s\n", (*array_clients) -> clients[(*array_clients) -> clients_count -1] -> name);
				char *message = "login_ok;\n";
				send_message(fd, message, info);					
			}
			else {
				//already logged
			}
		}
		else {
			char *message = "login_false;1;\n";
			send_message(fd, message, info);
		}		
	}
	else {
		(*client) = get_client_by_name(*array_clients, name);
		if ((*client) -> state == 2) {
			reconnect(array_clients, all_games, info, name, fd, tok, max_players, client);									
		}
		else {
			char *message = "login_false;2;\n";
			send_message(fd, message, info);
		}								
	} 
	return;
}
	
void reconnect(clients **array_clients, games *all_games, log_info **info, char *name, int fd, char *tok, int max_players, client **client) {
	(*client) -> socket_ID = fd;
								
	char board[1024];
	game *game = find_game_by_name(all_games, name);

	if(game == NULL) {
		(*client) -> state = 0; 
		login(array_clients, all_games, info, tok, max_players, fd, client);
		return;
	}

	char now_playing[10];
	if (strcmp(game -> now_playing, name) == 0) { 
		(*client) -> state = 3;
	}
	else {
		(*client) -> state = 4;
	}
	
	sprintf(board, "board;%s;%s;%d;%d;%s;", name, (*client) -> color, game -> game_ID, game -> fields -> count_pieces, now_playing);					
					
	int i,j;
	for (i = 0; i < game -> fields -> size; i++) {
		for(j = 0; j < game -> fields -> size; j++) {
			if (game -> fields -> all_fields[i][j] -> piece != NULL) {
				char x[3], y[3];

				sprintf(x, "%d;", i);
				sprintf(y, "%d;", j);
											
				strcat(board, x);
				strcat(board, y);
				strcat(board, game -> fields -> all_fields[i][j] -> piece -> color);
				strcat(board, ";");
				strcat(board, game -> fields -> all_fields[i][j] -> piece -> type);
				strcat(board, ";");							
			}
		}
	}
	strcat(board, "\n");
	send_message(fd, board, info);

	if (strcmp(game -> name_1, name) == 0) { 
		send_message(get_client_by_name(*array_clients, game -> name_2) -> socket_ID, "connection_restored;\n", info);
	}
	else {
		send_message(get_client_by_name(*array_clients, game -> name_1) -> socket_ID, "connection_restored;\n", info);	
	}
	return;
}

void play(clients **array_clients, wanna_play **wanna_plays, games **all_games, log_info **info, int fd, client **cl) {
	client *second_client = NULL;
	add_wanna_play(wanna_plays, fd);
	(*cl) -> state = 1;
	if (((*wanna_plays) -> size) >= 2) {
		int socket_ID_1 = fd;
		int socket_ID_2;
		do {
			socket_ID_2 = (*wanna_plays) -> socket_IDs[rand() % ((*wanna_plays) -> size)];
		} 
		while(socket_ID_2 == socket_ID_1);								

		remove_wanna_play(wanna_plays, socket_ID_1);
		remove_wanna_play(wanna_plays, socket_ID_2);
								

		char message_1[100];
		char message_2[100];
		char *now_playing;												

		second_client = get_client_by_socket_ID(*array_clients, socket_ID_2);

		int r = rand() % 2;
		if (r == 0) {
			sprintf(message_1, "start_game;black;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;white;%d;\n", (*all_games) -> games_count);

			set_state(cl, 4);
			set_state(&second_client, 3);

			set_color(cl, "black");
			set_color(&second_client, "white");

			now_playing = second_client -> name; 		
		}
		else {
			sprintf(message_1, "start_game;white;%d;\n", (*all_games) -> games_count);
			sprintf(message_2, "start_game;black;%d;\n", (*all_games) -> games_count);

			set_state(cl, 3);
			set_state(&second_client, 4);

			set_color(cl, "white");
			set_color(&second_client, "black");			

			now_playing = (*cl) -> name;
		}

		add_game(all_games, (*cl) -> name, second_client -> name, now_playing);

		send_message(socket_ID_1, message_1, info);
		send_message(socket_ID_2, message_2, info);
	}
	return;
}

void client_move(games **all_games, clients *array_clients, log_info **info, char *tok) {
	int i = 0;
	char *array[10];
						
	tok = strtok(NULL, ";");

	while(tok != NULL) {
		array[i++] = tok;
		tok = strtok(NULL, ";");
	}

	int game_ID = atoi(array[0]);
	int cp_row = atoi(array[1]);
	int cp_col = atoi(array[2]);
	int dp_row = atoi(array[3]);
	int dp_col = atoi(array[4]);
	char *color = array[5];
	char *piece = array[6];	
			
	printf("Move: GID=%d CP=%d,%d DP=%d,%d COLOR=%s TYPE=%s\n", game_ID, cp_row, cp_col, dp_row, dp_col, color, piece);
							
	process_move(all_games, array_clients, info, game_ID, cp_row, cp_col, dp_row, dp_col, color, piece); 
	return;
}

void delete_connection(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, int fd) {
	close(fd);
	FD_CLR(fd, client_socks);
	client_remove(array_clients, wanna_plays, fd);
	return;
}

void disconnect(clients **array_clients, log_info **info, games *all_games, int fd, client **client) {

	if (client == NULL) return;
	if ((*client) -> state == 2) return;
	(*client) -> state = 2;

	char *second_client_name;
	game *game = find_game_by_name(all_games, (*client) -> name);
	if (game == NULL) return;
	if (strcmp(game -> name_1, (*client) -> name) == 0) {
		second_client_name = game -> name_2;
	} 
	else {
		second_client_name = game -> name_1;
	}
	
	send_message(get_client_by_name(*array_clients, second_client_name) -> socket_ID, "opponent_connection_lost;\n", info);
	return;
}

void delete(clients **array_clients, wanna_play **wanna_plays, fd_set *client_socks, games **all_games, log_info **info, int fd, int err_ID, client **cl) {	
	delete_connection(array_clients, wanna_plays, client_socks, fd);	
	if ((*cl) == NULL) {
		return;
	}
	game *gm = find_game_by_name(*all_games, (*cl) -> name);	
	char message[30];

	switch (err_ID) {
		case 0:
			printf("Client with socket %d timeout\n", fd);
			break;
		case 1:
			printf("Client with socket %d left\n", fd);
			break;
	}
	if (gm == NULL) {
		return;
	}

	char *second_client_name;

	if (strcmp(gm -> name_1, (*cl) -> name) == 0) {
		second_client_name = gm -> name_2;
	} 
	else {
		second_client_name = gm -> name_1;
	}
	client *sec_cl = get_client_by_name(*array_clients, second_client_name);
	set_state(&sec_cl, 0);
	switch (err_ID) {
		case 0:
			sprintf(message, "end_game_timeout;11;\n");
			printf("Game with id %d removed due to client timeout\n", gm -> game_ID);
			break;
		case 1:
			sprintf(message, "end_game_left;10;\n");
			printf("Game with id %d removed due to client left\n", gm -> game_ID);
			break;
	}	
	send_message(get_client_by_name(*array_clients, second_client_name) -> socket_ID, message, info);
	//remove_game(array_clients, all_games, info, gm -> game_ID, cl);

	return;
}

//Každý program bude doplněn o zpracování statistických údajů: 
//	1.přenesený počet bytů
//	2.přenesený počet zpráv
//	3.počet navázaných spojení
//	4.počet přenosů zrušených pro chybu
//	5.doba běhu 
//	6.apod.

//	1.strlen(send_message) 
//	2.počet send
//	3.úspěšný connect
//	4.počet bitů, které se přenesli a nevyužili jsme je
//	5.nazačátku si uložit čas, při ukončení čas, z toho zjistit jak dlouho běžel server

void log_all(char *filename, log_info *info) {

	char message_count_bytes[100];
	char message_count_messages[100];
	char message_count_connections[100];
	char message_count_bad_transmissions[100];	
	char message_server_running[100];
		
	sprintf(message_count_bytes, "Bytes sent: %d\n", info -> count_bytes);
	sprintf(message_count_messages, "Messages sent: %d\n", info -> count_messages);
	sprintf(message_count_connections, "Clients connected: %d\n", info -> count_connections);
	sprintf(message_count_bad_transmissions, "Unused bytes sent: %d\n", info -> count_bad_transmissions);
	sprintf(message_server_running, "Server ran for %d minutes\n", info -> server_running_minutes);

	FILE *fptr;
	fptr = fopen(filename, "r+");
	if(fptr == NULL) 
	{
	    fptr = fopen(filename, "wb");
	}

	fprintf(fptr, message_count_bytes);
	fprintf(fptr, message_count_messages);
	fprintf(fptr, message_count_connections);
	fprintf(fptr, message_count_bad_transmissions);
	fprintf(fptr, message_server_running);

	fclose(fptr);	
	return;
}

void server_running(struct timeval start, struct timeval end, log_info **info) {
	(*info) -> server_running_minutes = (end.tv_sec - start.tv_sec) / 60;
	return;
}

void game_info(){
	printf("Info about draughts\n");
	printf("-port || -p [number] : set listening port (1024-65535), default 10000\n\n");
}
