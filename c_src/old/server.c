#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "structures.h"
#include "messages.h"

int main (void)
{

	int name_length = 20;
	int max_players = 32;
	int count_players = 0;
	char **names[max_players][name_length];
	
	int server_sock;
	int client_sock;
	int return_value;
	int cbufSize = 100;	
	char cbuf[cbufSize];
	struct sockaddr_in local_addr;
	struct sockaddr_in remote_addr;
	socklen_t remote_addr_len;	
	
	server_sock = socket(AF_INET, SOCK_STREAM, 0);

	int flag = 1;
	/* zpusobi ze nenastane error bind v pripade padu serveru */
	//setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

	if (server_sock <= 0)
	{
		printf("Socket ERR\n");
		return -1;
	}
	
	memset(&local_addr, 0, sizeof(struct sockaddr_in));

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(10000);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	return_value = bind(server_sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr_in));

	if (return_value == 0)
		printf("Bind OK\n");
	else
	{
		printf("Bind ERR\n");
		return -1;
	}

	return_value = listen(server_sock, 5);

	if (return_value == 0)
		printf("Listen OK\n");
	else
		printf("Listen ERR\n");

	while(1)
	{
		client_sock = accept(server_sock, (struct sockaddr *)&remote_addr, &remote_addr_len);

		if (client_sock > 0 )
		{	
			//clientCount++;
			return_value = fork();
			if (return_value==0)
			{	
				recv(client_sock, &cbuf, cbufSize*sizeof(char), 0);
				char *tok = strtok(cbuf, ";");
				char *type_message = tok;
				if (strcmp(type_message, "login") == 0) {	
					tok = strtok(NULL, ";");
					char *name = tok;

					if (name_exists(**names, count_players, name) == 0) {
						*names[count_players] = &name;
						count_players++;
						printf("Jmeno: %s\n", name);
						send_message(client_sock, "login_ok");
					}
					else {
						send_message(client_sock, "login_false");
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
				//printf("Nove spojeni se jmenem: %s\n", cbuf);
				//recv(client_sock, &cbuf, cbufSize*sizeof(char), 0);
				//printf("Pocet spojeni: %d\n", clientCount);
				//char *message = "message";
				//send(client_sock, message, strlen(message)*sizeof(char), 0);
				//printf("%s sended\n", message);
				//close(client_sock);
				//return 0;
			}
			//close(client_sock);
		}
		else
		{
			printf("Brutal Fatal ERROR\n");
			return -1;
		}
	}

return 0;
}

//return: 0 - not contains name
//	  1 - contains name
int name_exists (char **names, int players_count, char *name) {
	int i;
	for (i = 0; i < players_count; i++) {
		if (strcmp(names[i], name) == 0) {
			return 1;
		}
	}
	return 0;
}

void send_message(int client_sock, char *message) {	
	printf("Message: %s\n", message);
	send(client_sock, message, strlen(message) * sizeof(char), 0);
}
