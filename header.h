typedef struct the_client client;
typedef struct the_clients clients;
typedef struct the_field field;
typedef struct the_game game;
typedef struct the_games games;


struct the_client {
	char *name;
	int socket_ID;
};

struct the_clients {
	int clients_count;
	client **clients;
};

struct the_field {
	int row;
	int col;
	char *color;
};

struct the_game {
	int size;
	field **fields;
};


struct the_games {
	int count;
	game **games;
};

//client.c
void create_clients(clients **array_clients);
void create_client(client **cl, char *name, int socket_ID);
void add_client(clients **array_clients, char *name, int socket_ID);
void client_remove(clients **array_clients, int socket_ID);

//game.c
void create_game(games **all_games, int first_player_socket_ID, int second_player_socked_ID);

//server.c
int name_exists (clients *array_clients, char *name);
void send_message(int client_socket, char *message);

