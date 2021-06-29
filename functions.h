#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>



const int BufferSize = 50;

double ComputeToken(double receivedToken, float DT, double RF)
{
    if(receivedToken >= 1)
        receivedToken = -1;
    double newToken;
    newToken = receivedToken + DT * (1 - (receivedToken * receivedToken) / 2 )* (2 * M_PI * RF);
    printf("New Token:%f\n", newToken);
    return newToken;
}


// Returns the current time (mu_sec)
long getTime(){
	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}


// Saves the current process PID to an external file
int save_pid_to_file_fnct(const char * pid_fileName, pid_t pid){
    FILE *pid_file = fopen(pid_fileName, "w");
    if (pid_file == NULL)
    {
        perror("File opening");
        exit(-1);
    }
    if (fprintf(pid_file, "%i", pid) == -1)
    {
        perror("File writing");
        exit(-1);
    }
    fclose(pid_file);
}

// Reads the necessary processes IDs from an external file
int read_pid_from_file_fnct(const char * pid_fileName){
    char Mystr[5];
    int pid_;
    FILE *pid_f = fopen(pid_fileName, "r");
    if (pid_f == NULL)
    {
        perror("file opening");
        exit(-1);
    }
    while (fgets(Mystr, 10, pid_f) == NULL)
    {
    }
    pid_ = atoi(Mystr);
    return pid_;
}

// Dumps log file
int dump_log_fnct(char* logfile, FILE * logname, int dump_size, char buffer_LP[50]){
	logname = fopen(logfile, "r");
    if (logname == NULL)
    {
        printf("There is not Log File yet\n");
    }
    else
    {
                                
    fseek(logname, 0, SEEK_END);
    int size = ftell(logname);
    fseek(logname, size - dump_size, SEEK_SET);
    if (size >= dump_size)
    {
        char *buffer_LP = malloc(dump_size + 1);

        // Read  the file and save 
        if (fread(buffer_LP, 1, dump_size, logname) != dump_size)
        {
            perror("Reading the file");
            exit(-1);
        }
        fclose(logname);
        
        // Write log on stdout
        if (write(STDOUT_FILENO, buffer_LP, dump_size) == -1)
        {
            perror("write log on stdout");
            exit(-1);
        }
        fflush(stdout);
        return 1;
    }

    else
    {
        printf("The log file is empty.\n");
        return 0;
    }
}
}

// Initializes a socket connection
int create_socket_fnct(int sockfd, int W, struct sockaddr_in serv_addr, struct sockaddr_in cli_addr, int clilen, int portno, char server_buf[256]){
	int newsockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(-1);
    }
    
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &W, sizeof(W));
    fflush(stdout);
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("ERROR on binding");
        exit(-1);
    }
    
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
    {
        perror("ERROR on accept");
        exit(-1);
    }
    
    bzero(server_buf, 256);
    return newsockfd;
}

