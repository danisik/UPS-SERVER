#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
// kvuli iotctl
#include <sys/ioctl.h>
#include "client.h"
#include "structures.h"
#include "messages.h"


//vlánko do javy pro receive a send
int main (void)
{

	int name_length = 20;
	int max_players = 32;

	clients *array_clients;	
	create_clients(&array_clients);

	int server_socket;
	int client_socket, fd;
	int return_value;
	int cbuf_size = 100;
	char cbuf[cbuf_size];
	int len_addr;
	int a2read;
	struct sockaddr_in my_addr, peer_addr;
	fd_set client_socks, tests; // mnozina file deskriptoru (mj. i napr. socketu)

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	int param = 1;
    return_value = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int));
	
	if (return_value == -1)
		printf("setsockopt ERR\n");

	memset(&my_addr, 0, sizeof(struct sockaddr_in));

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(10000);
	my_addr.sin_addr.s_addr = INADDR_ANY;

	return_value = bind(server_socket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in));

	if (return_value == 0) 
		printf("Bind - OK\n");
	else {
		printf("Bind - ERR\n");
		return -1;
	}

	return_value = listen(server_socket, 5);
	if (return_value == 0){
		printf("Listen - OK\n");
	} else {
		printf("Listen - ERR\n");
	}

	// vyprazdnime sadu deskriptoru a vlozime server socket
	FD_ZERO(&client_socks);
	FD_SET(server_socket, &client_socks);

	while(1)
	{
		// zkopirujeme si fd_set do noveho, stary by byl znicen (select ho modifikuje)
		tests = client_socks;

		// sada deskriptoru je po kazdem volani select prepsana sadou deskriptoru kde se neco delo
		return_value = select(FD_SETSIZE, &tests, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)0);

		if (return_value < 0)
		{
			printf("Select ERR\n");
			return -1;
		}

		// vynechavame stdin, stdout, stderr
		for (fd = 3; fd < FD_SETSIZE; fd++)
		{
			// je dany socket v sade fd ze kterych lze cist ?
			if (FD_ISSET(fd, &tests))
			{
				// je to server socket? prijmeme nove spojeni
				if (fd == server_socket)
				{
					client_socket = accept(server_socket, (struct sockaddr *) &peer_addr, &len_addr);
					FD_SET(client_socket, &client_socks);
					printf("Pripojen novy klient a pridan do sady socketu\n");
				}
				else // je to klientsky socket? prijmem data
				{
					// pocet bajtu co je pripraveno ke cteni
					ioctl(fd, FIONREAD, &a2read);
					// mame co cist
					if (a2read > 0)
					{
						recv(fd, &cbuf, cbuf_size*sizeof(char), 0);

						char *tok = strtok(cbuf, ";");
						char *type_message = tok;
						if (strcmp(type_message, "login") == 0) {	
							tok = strtok(NULL, ";");
							char *name = tok;
		
							if (name_exists(array_clients, name) == 0) {								
								if ((array_clients -> clients_count) < (max_players)) {
									add_client(&array_clients, name, fd);
									printf("Jmeno: %s\n", array_clients -> clients[array_clients -> clients_count -1] -> name);
									send_message(fd, "login_ok;\n");					
								}
								else {
									send_message(fd, "login_false;Too much Players online;\n");
								}		
							}
							else {
								send_message(fd, "login_false;This name is already taken;\n");
							} 
						}
						else if (strcmp(type_message, "client_move") == 0) {
						
						}
						else if (strcmp(type_message, "end_move") == 0) {
		
						}
						else if (strcmp(type_message, "new_game_yes") == 0) {
		
						}
						else if (strcmp(type_message, "new_game_no") == 0) {
				
						}
						else {
							//spatny prikaz
						}						
					}
					else // na socketu se stalo neco spatneho
					{
						close(fd);
						FD_CLR(fd, &client_socks);
						client_remove(&array_clients, fd);
						printf("Klient se odpojil a byl odebran ze sady socketu\n");
					}
				}
			}
		}
	}	
	return 0;
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
	printf("\n");
	return 0;
}

void send_message(int client_socket, char *message) {	
	printf("Message: %s\n", message);
	send(client_socket, message, strlen(message) * sizeof(char), 0);
}

