// GLOBAL VARIABLES OF THE SOCKET CONNECTION
#define MAX_QUEUE_CONNECTIONS 5 //In reality the computer will connect to itself, so with a 1 it would still work

//GLOBAL VARIABLES FOR THE SOCKET
struct hostent *host;
int server_port;
int tcp_host_socket_fd;
int tcp_client_socket_fd;
socklen_t sockaddr_size;
socklen_t client_sockaddr_size;
struct sockaddr_in client_address;
struct sockaddr_in server_address;
int client_sock_fd;
char output_buffer[BUFFER_SIZE] = {0}; // Array with the outgoing token
char input_buffer[BUFFER_SIZE] = {0};  // Array with the incoming token
float x = 0.2;                         // Variable containing the first trial message

