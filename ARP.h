#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>

#define MAX_LEN 100
#define FREQ_LEN 10
#define IP_LEN 3
#define WAIT_LEN 9

#define M_PI 3.14159265358979323846
#define FILE_LINE_LENGH 69

int loops = 1;


// FUNCTIONS NEEDED BY MAIN CODE

/* Reads config file
      input: file name
      output: array of frequency, IP
*/
double *read_config(char *fileName)
{

    FILE *infile = fopen(fileName, "r");

    int size = 3;
    double *data = malloc(size);

    // Open file
    if (!infile)
    {
        perror("couldn't open file");
        //return EXIT_FAILURE;
    }

    char buffer[MAX_LEN];
    char *endptr;

    // Read file
    while (fgets(buffer, MAX_LEN, infile))
    {

        // Look for the frequency
        if (!strncmp(buffer, "Frequency=", FREQ_LEN))
        {
            char *num_start = buffer + FREQ_LEN;
            double freq = strtod(num_start, &endptr);

            if (endptr == num_start)
            {
                fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[0] = freq;
        }

        // Look for the IP
        else if (!strncmp(buffer, "IP=", IP_LEN))
        {
            char *num_start = buffer + IP_LEN;
            double ip = strtod(num_start, &endptr);

            if (endptr == num_start)
            {
                fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[1] = ip;
        }

        // Look for the wait time
        else if (!strncmp(buffer, "WaitTime=", WAIT_LEN))
        {
            char *num_start = buffer + WAIT_LEN;
            double waitingTime = strtod(num_start, &endptr);

            if (endptr == num_start)
            {
                fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[2] = waitingTime;
        }
    }

    //printf("%f, %f",data[0],data[1]);
    fclose(infile);

    return data;
}

/* Writes time on log file 
      input: file name, token1 and token2
      output: none
*/

/////////////////////////////////second line missing

void writeLogFile(char *fileName, double token1, double token2)
{
    // Start writing log process
    printf("    Log dumping...\n");
    time_t timer;
    char buffer[26];
    struct tm *tm_info;

    // Note down current time
    timer = time(NULL);
    tm_info = localtime(&timer);

    // Save current time in a string
    strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);

    // Append a new line to the log file
    FILE *fp = fopen(fileName, "a");
    char *data_1 = "LOG: ";
    char *data_open = "<";
    char *data_2 = "> <from G | from S> <";
    char *data_close = ">";
    char *data_enter = "\n";

    char *value1 = malloc(sizeof(double));
    memcpy(value1, &token1, sizeof(double));

    char *value2 = malloc(sizeof(double));
    memcpy(value2, &token2, sizeof(double));

    fputs(data_1, fp);
    fputs(data_open, fp);
    fputs(buffer, fp);
    fputs(data_2, fp);
    fprintf(fp, "%f", token1);
    fputs(data_close, fp);
    fputs(data_enter, fp);

    fputs(data_open, fp);
    //timestamp
    fputs(data_close, fp);
    fputs(data_open, fp);
    fprintf(fp, "%f", token2);
    fputs(data_close, fp);
    fputs(data_enter, fp);
    fclose(fp);

    printf("    Log file written\n\n");
}

/* Writes on screen the content of the log file
      input: file name
      output: none
*/
void dumpLogFile(char *fileName)
{

    char buff[FILE_LINE_LENGH]; // Contains the read line

    // Read the last line of the file
    FILE *fp = fopen(fileName, "r");
    fseek(fp, -FILE_LINE_LENGH, SEEK_END); // set pointer to the end of file minus the length you need. There can be more than one new line caracter
    fread(buff, FILE_LINE_LENGH - 1, 1, fp);
    fclose(fp);

    // Print the line to screen
    printf("%s\n\n", buff);
}

/*
// Function to host a socket connection as a server
int start_fileserver_tcp()
{
    host = gethostbyname(My_IP);
    server_port = atoi(My_PORT);

    // socket descriptor
    tcp_host_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_host_socket_fd == -1)
    {
        fprintf(stderr, "[ERROR] Failed to create socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    sockaddr_size = sizeof(struct sockaddr);
    // zeroise the sockaddr structure
    bzero(&server_address, sockaddr_size);

    server_address.sin_family = AF_INET; // AF_INET is the protocol IPv4
    server_address.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    server_address.sin_port = htons(server_port);

    if (bind(tcp_host_socket_fd, (struct sockaddr *)&server_address, sockaddr_size) == -1)
    {
        fprintf(stderr, "[ERROR] Failed to bind socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    if (listen(tcp_host_socket_fd, MAX_QUEUE_CONNECTIONS) == -1)
    {
        fprintf(stderr, "[ERROR] Failed to listen socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    client_sockaddr_size = sizeof(struct sockaddr);

    // Listen for a connection and accept if some
    client_sock_fd = accept(tcp_host_socket_fd, (struct sockaddr *)&client_address, &client_sockaddr_size);
    if (client_sock_fd == -1)
    {
        fprintf(stderr, "[ERROR] Failed to accept socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    printf("Connection stablished :)\n");

    //Send a first random number to verify the connection. This one won't be printed
    x = x + 0.1;
    char buf[BUFFER_SIZE];

    gcvt(x, 6, buf);

    strcpy(output_buffer, buf);

    if (send(client_sock_fd, output_buffer, strlen(output_buffer) + 1, 0) == -1)
    {
        fprintf(stderr, "[ERROR] Failed sending message: %s\n", strerror(errno));
    }
}

// Function to wstablish a connection via socket connection as a client
int connect_fileserver_tcp()
{
    host = gethostbyname("localhost");
    server_port = atoi("1502");

    // socket descriptor
    tcp_client_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_client_socket_fd == -1)
    {
        fprintf(stderr, "[ERROR] Failed to create socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }

    sockaddr_size = sizeof(struct sockaddr);
    // zeroise the sockaddr structure
    bzero(&server_address, sockaddr_size);

    server_address.sin_family = AF_INET;
    server_address.sin_addr = *((struct in_addr *)host->h_addr_list[0]);
    server_address.sin_port = htons(server_port);

    if (connect(tcp_client_socket_fd, (struct sockaddr *)&server_address, sockaddr_size) == -1)
    {
        fprintf(stderr, "[ERROR] Failed to connect socket: %s\n", strerror(errno));

        return EXIT_FAILURE;
    }
}
*/

/*
        while (1) {
            fd_set rfds;
            struct timeval tv;
            struct timespec tv2;
            int retval, m,n;
         
            FD_ZERO(&rfds);
            FD_SET(fd1[0], &rfds);
            FD_SET(fd2[0], &rfds);
            
            if(fd1[0]>fd2[0])
               Nselect=fd1[0];
            else
               Nselect=fd2[0];
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            
            retval = select(Nselect+1, &rfds, NULL, NULL, &tv);
            
            if (retval == -1)   
               perror("select()");
            else if (retval) {   
               printf("Data is available now.\n");
               if (FD_ISSET(fd1[0], &rfds)){ // Sn
                  n = read(fd1[0], &actualToken, sizeof(actualToken));
                  printf("Received from pipe 1: %d\n",n);
               }
               
               if (FD_ISSET(fd2[0], &rfds)){ //Gn
                  n = read(fd2[0], &actualToken, sizeof(actualToken));
                 printf("Received from pipe 2: %d\n",n);
               }
               if (FD_ISSET(fd1[0], &rfds) && FD_ISSET(fd2[0], &rfds)){
                  int randomN = rand() % 2;
                  if(randomN==0){
                     n = read(fd1[0], &actualToken, sizeof(actualToken));
                     printf("Received from pipe 1: %d\n",n);
                  }
                  else if(randomN==1){
                     n = read(fd2[0], &actualToken, sizeof(actualToken));
                     printf("Received from pipe 2: %d\n",n);
                  }
               }
            }
             else   
                printf("No data within 5 seconds.\n");
         }
*/

/*-------------------------------------Socket (client) initialization---------------------------*/
/*         // Create socket
         float line;
         int m;
         float q;
         sleep(2);
         int sockfd, portno;
         struct sockaddr_in serv_addr;
         struct hostent *server;
         portno = atoi(targ2);
         sockfd = socket(AF_INET, SOCK_STREAM, 0);
         if (sockfd < 0)
            error("ERROR opening socket");
         server = gethostbyname("localhost");
         if (server == NULL)
            error("ERROR, no such host\n");
         bzero((char *)&serv_addr, sizeof(serv_addr));
         serv_addr.sin_family = AF_INET;
         bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
         serv_addr.sin_port = htons(portno);
         if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");
         float temp = 1;
         m = write(sockfd, &line, sizeof(line));
         if (m < 0)
            error("ERROR writing to socket");
*/
/*------------------------------------------------------------------------------------------------------*/
