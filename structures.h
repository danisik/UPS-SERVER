typedef struct the_clients clients;
typedef struct the_client client;
typedef struct the_fields fields;
typedef struct the_field field;


struct the_client {
	char *name;
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

struct the_fields {
	int size;
	field **fields;
};

void create_clients(clients **array_clients);
void create_client(client **cl, char *name, int socket_ID);
void add_client(clients **array_clients, char *name, int socket_ID);

int name_exists (clients *array_clients, char *name);
void send_message(int client_socket, char *message);

