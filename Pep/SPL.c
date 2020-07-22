#define _DEFAULT_SOURCE
#define _GNU_SOURCE
#define _POSIX_SOURCE

#define errno (*__errno_location())

#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

#define SIG_IGN ((__sighandler_t)1)
#define SIGUSR1 10
#define SIGCONT 18
#define SIGSTOP 19
#define SIGUSR2 12

// Several functions use GLOBAL VARIABLES. This has been done to split the code in functions in an easier way

// GLOBAL VARIABLES FOR GENERAL USE
#define BUFFER_SIZE 1024

// GLOBAL VARIABLES OF THE SOCKET CONNECTION
#define MAX_QUEUE_CONNECTIONS 5 //In reality the computer will connect to itself, so with a 1 it would still work

// GLOBAL VARIABLES OF THE FILE READER
int i = 0;
int n;
int numProgs = 0;
char *programs[50];
char line[50];
char *My_IP, *My_PORT, *My_RF, *My_WT; // In this variables we store the read paramaters from the file

// GLOBAL VARIALBES PIPE COMUNICATION
pid_t P, L, G, S;                       //PID's
int fd_SP[2], fd_PL[2], fd_GP[2];       // Pipe names
char received_data[BUFFER_SIZE] = {0};  // Variable where we store the received data through socket connection
char received_char_number[BUFFER_SIZE]; // Decomposed number from the received_data
char received_char_time[BUFFER_SIZE];   // Decomposed date from the received_data
int j = 0;
int ctr = 0;
char newString[256][256];
char *word[1024];
int start_signal = 0; // Variable used to let the process know we have awekaned from a STOP signal

// GLOBAL VARIABLES FOR THE SIGNAL HANDLING
int dump_log = 0;
int startsignal = 0;
int stopsignal = 0;

// GLOBAL VARIABLES USED TO CALCULTE THE SINEWAVE
#define number_of_iterations 100000 // Iterations of the foor loop
float new_token = 0;                // New computed token
float received_token = 0;           // New received token (old token)
float DT = 1;
const float pi = 3.1415;
int RF;                               // Rate frequency
float numbers_array[524288];          // Array where we store the velue of each point of the sine wave
char date_array[524288][BUFFER_SIZE]; // Array where we store the date for each sine value
int counter = 0;                      // How many iterations have we gone through the P loop
int pilot = 0;                        // Variable that tells us whether is the first iteration or is not
int going_up = 1;                     // Variable that tells us which formula do we need to use
char LOG_ARRAY[524288][BUFFER_SIZE];  // Array where we store all the messages
long long int log_counter;

//GLOBAL VARIABLES FOR THE SELECT
fd_set set;
struct timeval timeout;
struct timeval tv;

//GLOBAL VARIALBES FOR THE TIME COMPUTATION
double cpu_time_used;
struct timeval tv1; // tv1 is the timestamp related to the incoming token
struct timeval tv2; // tv1 is the timestamp related to the outgoing token
struct tm *tm;
uint64_t useconds[2] = {0}; // useconds hold the value in usec of the incoming and outgoing time stamps
int useconds_length = sizeof(uint64_t);

//GLOBAL VARIABLES FOR THE STDIN INPUT
#define BUFFERSIZE 10
char buffer[BUFFERSIZE];
char *string;

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

//SIGNAL HANDLER
void SigHandler(int);

void SigHandler(int sig)
{
    printf("handler called\n");

    //we ignore the signal while the function executes
    signal(sig, SIG_IGN);

    switch (sig)
    {
    default:

    case SIGUSR1:
        printf("[CHILD] SIGUSR1 received\n");
        break;

        /*
    case SIGCONT:
        printf("[CHILD] SIGCONT received\n");
        break;

    case SIGSTOP:
        printf("[CHILD] SIGSTOP received\n");
        break;
        */
    }

    //reinstall it
    signal(sig, SigHandler);
}

// MAIN LOOP
int main(void)
{

    //Read the values from the "config.txt" file
    read_file();

    //Declare the pipes used for inter process comunication
    pipe(fd_SP);
    pipe(fd_PL);
    pipe(fd_GP);

    // FATHER (S)
    P = fork();
    if (P < 0)
    {
        perror("[ERROR] Forking P");
        return EXIT_FAILURE;
    }
    // P
    else if (P == 0)
    {
        printf("P - AWAKE!\n");

        // Give some time for the user to get ready
        sleep(5);

        // Close unnecessary pipes
        close(fd_SP[1]); //write to S
        close(fd_GP[1]); //write to G
        close(fd_PL[0]); //read from L

        // Execute the function that hosts the socket connection
        start_fileserver_tcp();

        // Transform the read variable of frequency from the config file "config.txt" from char to integer
        RF = atoi(My_RF);

        // I execute the loop a finite number of times, but the code is meant to run forever with a "while(1)"
        int k;
        for (k = 0; k < number_of_iterations; ++k)
        {
            // READ S FROM P

            //Initialize the timeout data structure for the select
            FD_ZERO(&set);
            FD_SET(fd_SP[0], &set);
            timeout.tv_sec = 0;
            timeout.tv_usec = atoi(My_WT);

            int ret1 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

            if (ret1 == 0)
            {
                //printf("No data from pipe (S-P)\n");
            }
            else if (ret1 < 0)
            {
                //error occurred
                //printf("ERROR from pipe (S-P)\n");
            }
            else
            {
                //printf("Data available from pipe (S-P)\n");
                char buff[sizeof(LOG_ARRAY[0])];
                read(fd_SP[0], &buff, sizeof(buff));
                // We store the received message from S into the LOG_ARRAY
                sprintf(LOG_ARRAY[log_counter], "%s", buff);
                log_counter++;

                // To gather the correct time, when a command is written in the console, S packs the time
                // and sends it to P when it can.
                // We decompose the received message looking for keywords in order to check whether the
                // received message orders us to change our behabiour
                for (i = 0; i <= (strlen(buff)); i++)
                {
                    if (buff[i] == ' ' || buff[i] == '\0')
                    {
                        newString[ctr][j] = '\0';
                        ctr++;
                        j = 0;
                    }
                    else
                    {
                        newString[ctr][j] = buff[i];
                        j++;
                    }
                }
                int size = ctr;
                i = 0;
                j = 0, ctr = 0;

                for (int i = 0; i < size; i++)
                {
                    word[i] = newString[i];
                }

                // Check if any word is "Dump" to dump the log, or "Start" to reset the sine wave
                for (int k = 0; k < size; k++)
                {
                    if (strncmp(word[k], "Dump", 3) == 0 || strncmp(word[k], "Dump", 2) == 0)
                    {
                        dump_log = 1;
                        //printf("DUMP LOG!!!\n");
                    }
                    else if (strncmp(word[k], "Start", 4) == 0 || strncmp(word[k], "Start", 3) == 0)
                    {
                        start_signal = 1;
                        //printf("RESUME!!!\n");
                    }
                }
            }

            // If we received a dump log from S we will send our log to P

            // We will print two logs: one with the values to plot the sine wave, and one with all the data sent
            // through socket and pipes
            if (dump_log == 1)
            {
                //printf("Sending the log from P to L\n");

                // We start with the sine wave values and dates
                // Firs we send the amount of values the sine wave got so far
                char buff2[sizeof(counter)];
                sprintf(buff2, "%d", counter);
                write(fd_PL[1], buff2, sizeof(buff2));
                dump_log = 0;
                long long int temp_counter = 0;

                // Then we send the dates in the date_array
                while (temp_counter != counter)
                {
                    //printf("P - Sending date %s\n", date_array[temp_counter]);
                    write(fd_PL[1], &date_array[temp_counter], sizeof(date_array[0]));
                    temp_counter++;
                }
                temp_counter = 0;

                // Then we send the values of the sine wave in the numbers_array.
                while (temp_counter != counter)
                {
                    char buff3[sizeof(date_array[temp_counter])];
                    // We multiply the numbers by 100000000000 so we can get rid of the floating point
                    sprintf(buff3, "%f", numbers_array[temp_counter] * 100000000000);
                    //printf("P - Sending number %s\n", buff3);
                    write(fd_PL[1], &buff3, sizeof(date_array[temp_counter]));
                    temp_counter++;
                }
                temp_counter = 0;

                // We follow with all the data send through socket and pipes
                // Firs we send the amount of values inside the LOG_ARRAY
                char buff6[sizeof(log_counter)];
                sprintf(buff6, "%d", log_counter);
                write(fd_PL[1], buff6, sizeof(buff6));

                // Then we sent the actual data inside the LOG_ARRAY
                while (temp_counter != log_counter)
                {
                    //printf("P - Sending LOG_ARRAY %s\n", LOG_ARRAY[temp_counter]);
                    write(fd_PL[1], &LOG_ARRAY[temp_counter], sizeof(LOG_ARRAY[0]));
                    temp_counter++;
                }
                temp_counter = 0;
            }

            //Initialize the timeout data structure for the select
            FD_ZERO(&set);
            FD_SET(fd_GP[0], &set);
            timeout.tv_sec = 0;
            timeout.tv_usec = atoi(My_WT);

            int ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
            if (ret == 0)
            {
                //printf("No data from pipe (G-P)\n");
            }
            else if (ret < 0)
            {
                // error occurred
                //printf("ERROR from pipe (G-P)\n");
            }
            else
            {
                //printf("Data available from pipe (G-P)\n");
                read(fd_GP[0], &received_data, sizeof(received_data));

                // The data we receive is a char array which combines the  the value of the sine wave and the date.
                // The deserialise function decomposes the incoming message into the value of the sine wave and the
                // date which are coming together
                deserialize();

                // We reconvert the date we received into time stamp
                tv1.tv_sec = ((strtoull(received_char_time, NULL, 10)) / 1000000);
                tv1.tv_usec = ((strtoull(received_char_time, NULL, 10)) % 1000000);
                tm = localtime(&tv1.tv_sec);

                // And the received number to float
                received_token = atof(received_char_number) / (1000000);

                // And we store both into the LOG_ARRAY as a single string
                sprintf(LOG_ARRAY[log_counter], "%d:%02d:%02d.%d From G: %f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv1.tv_usec, received_token);
                log_counter++;

                // Now we get the time we will use to compute DT
                gettimeofday(&tv2, NULL);
                tm = localtime(&tv2.tv_sec);

                // useconds[0] has the current time in usec, useconds[1] has the previous time also in usec
                useconds[0] = tv2.tv_sec * 1000000 + tv2.tv_usec;
                useconds_length = sizeof(useconds[0]);
                useconds[1] = strtoull(received_char_time, NULL, 10);

                // Compute DT
                if (pilot == 0)
                {
                    // If its the first loop, consider DT to be 0
                    DT = 0;
                    // Also the first token = 0
                    new_token = 0;

                    // By setting this variable to 1, we make sure we will fall into this case only once
                    pilot = 1;
                    //start_signal = 0;
                }
                else
                {
                    // We compute DT as we should commonly do
                    DT = (useconds[0] - useconds[1]);
                    DT = DT / 1000000;
                }

                // We will use two formulas depending on wether we reach +1 or we reach -1
                if (going_up == 1)
                {
                    new_token = received_token + DT * (1 - (received_token) * (received_token) / 2) * (2 * pi * RF);

                    if (new_token >= 1.0)
                    {
                        going_up = 0;
                    }
                }
                else if (going_up == 0)
                {
                    if (received_token >= 0.0)
                    {
                        new_token = received_token - DT * (1 - (received_token) * (received_token) / 2) * 2 * pi * RF;
                    }
                    else if (received_token < 0.0)
                    {
                        new_token = received_token - DT * (1 - (-received_token) * (-received_token) / 2) * 2 * pi * RF;
                    }

                    if (new_token <= -1.0)
                    {
                        going_up = 1;
                    }
                }

                // If we receive a "start" in our console, we will reset our sine wave
                if (start_signal == 1)
                {
                    new_token = 0.0;
                    // The first value we compute ater a STOP, is always wrong. By doing this, we can erase it
                    // from our logs
                    log_counter = log_counter - 3;
                    counter = counter - 1;

                    start_signal = 0;
                }

                // We store the date and the value we generated inside the date_array, and inside the numbers_array
                // remember that this 2 arrays will be used for plotting purposes
                sprintf(date_array[counter], "%d:%02d:%02d.%d\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec);
                numbers_array[counter] = new_token;

                // And we store both in the LOG_ARRAY
                // Remember that this array won't have plotting purposes, it will only give us information.
                sprintf(LOG_ARRAY[log_counter], "%d:%02d:%02d.%d %f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec, new_token);

                // We increase this number every time we add something inside our LOG_ARRAY
                log_counter++;
                //printf("%i\n",log_counter);
                if (log_counter >= 524288)
                {
                    dump_log = 1;
                }

                // We create a delay, as we were required to do
                usleep((atoi(My_WT)));

                // And we send our new token from P to G via socket connection
                send_data();

                // We increase this number every time we compute a new value for our sine wave
                counter++;
            }
            //printf("Iteration S %d\n", k);
        }

        // Finally we close the pipes and the socket connection
        close(fd_GP);
        close(fd_SP);
        close(fd_PL);

        if (close(tcp_host_socket_fd) == -1)
        {
            fprintf(stderr, "[ERROR] Failed to close socket server: %s\n", strerror(errno));

            return EXIT_FAILURE;
        }

        if (close(client_sock_fd) == -1)
        {
            fprintf(stderr, "[ERROR] Failed to close socket client: %s\n", strerror(errno));

            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
    else
    {

        // FATHER (S)
        L = fork();
        if (L < 0)
        {
            perror("Fork");
            return -1;
        }

        // L
        else if (L == 0)
        {
            printf("L - AWAKE!\n");

            // Give some time for the user to get ready
            sleep(5);

            // Close unnecessary pipes
            close(fd_PL[1]); // write to P
            close(fd_GP[0]); // unrelated pipe
            close(fd_GP[1]); // unrelated pipe
            close(fd_SP[0]); // unrelated pipe
            close(fd_SP[1]); // unrelated pipe

            // Each pilot variable is used to be sure no reading process start untill the previous one is over
            int pilot1 = 0;
            int pilot2 = 0;
            int pilot3 = 0;
            int pilot4 = 0;
            int pilot5 = 0;

            FILE *fp;
            FILE *fp2;

            int temp_counter = 0;

            while (1)
            {
                // We receive the amount of variables inside the date_array
                if ((pilot1 == 0) && (pilot2 == 0) && (pilot3 == 0) && (pilot4 == 0) && (pilot5 == 0))
                {
                    //Initialize the timeout data structure for the select
                    FD_ZERO(&set);
                    FD_SET(fd_PL[0], &set);
                    timeout.tv_sec = 0;
                    timeout.tv_usec = atoi(My_WT);

                    int rett1 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                    if (rett1 == 0)
                    {
                        //printf("No data from pipe (L-P)\n");
                    }
                    else if (rett1 < 0)
                    {
                        // error occurred
                    }
                    else
                    {
                        char buffer3[sizeof(counter)];
                        read(fd_PL[0], &buffer3, sizeof(buffer3));
                        pilot1 = 1;
                        counter = atoi(buffer3);
                    }
                }

                // We receive the date_array
                else if ((pilot1 == 1) && (pilot2 == 0) && (pilot3 == 0) && (pilot4 == 0) && (pilot5 == 0))
                {
                    //printf("L - Receiving the data array\n");
                    while (temp_counter != counter)
                    {
                        //Initialize the timeout data structure for the select
                        FD_ZERO(&set);
                        FD_SET(fd_PL[0], &set);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = atoi(My_WT);

                        int ret2 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                        if (ret2 == 0)
                        {
                            //printf("No data from pipe (L-P)\n");
                        }
                        else if (ret2 < 0)
                        {
                            // error occurred
                        }
                        else
                        {
                            read(fd_PL[0], &date_array[temp_counter], sizeof(date_array[0]));
                            temp_counter++;
                        }
                    }
                    pilot2 = 1;
                    temp_counter = 0;
                }

                // We receive the numbers_array
                else if ((pilot1 == 1) && (pilot2 == 1) && (pilot3 == 0) && (pilot4 == 0) && (pilot5 == 0))
                {
                    //printf("L - Receiving the number array\n");
                    while (temp_counter != counter)
                    {
                        //Initialize the timeout data structure for the select
                        FD_ZERO(&set);
                        FD_SET(fd_PL[0], &set);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = atoi(My_WT);

                        int ret3 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                        if (ret3 == 0)
                        {
                            //printf("No data from pipe (L-P)\n");
                        }
                        else if (ret3 < 0)
                        {
                            // error occurred
                        }
                        else
                        {
                            unsigned char buffer4[sizeof(date_array[0])];
                            read(fd_PL[0], &buffer4, sizeof(buffer4));
                            sprintf(buffer4, "%s", buffer4);
                            numbers_array[temp_counter] = atof(buffer4) / 100000000000;
                            temp_counter++;
                        }
                    }
                    pilot3 = 1;
                    temp_counter = 0;
                }

                // We receive the amount of variables inside the LOG_ARRAY
                else if ((pilot1 == 1) && (pilot2 == 1) && (pilot3 == 1) && (pilot4 == 0) && (pilot5 == 0))
                {
                    //Initialize the timeout data structure for the select
                    FD_ZERO(&set);
                    FD_SET(fd_PL[0], &set);
                    timeout.tv_sec = 0;
                    timeout.tv_usec = atoi(My_WT);

                    int rett4 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                    if (rett4 == 0)
                    {
                        //printf("No data from pipe (L-P)\n");
                    }
                    else if (rett4 < 0)
                    {
                        // error occurred
                    }
                    else
                    {
                        char buffer5[sizeof(log_counter)];
                        read(fd_PL[0], &buffer5, sizeof(buffer5));
                        pilot4 = 1;
                        log_counter = atoi(buffer5);
                    }
                }

                // We receive the LOG_ARRAY
                else if ((pilot1 == 1) && (pilot2 == 1) && (pilot3 == 1) && (pilot4 == 1) && (pilot5 == 0))
                {
                    //printf("L - Receiving the LOG_ARRAY\n");
                    while (temp_counter != log_counter)
                    {
                        //Initialize the timeout data structure for the select
                        FD_ZERO(&set);
                        FD_SET(fd_PL[0], &set);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = atoi(My_WT);

                        int ret5 = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                        if (ret5 == 0)
                        {
                            //printf("No data from pipe (L-P)\n");
                        }
                        else if (ret5 < 0)
                        {
                            // error occurred
                        }
                        else
                        {
                            read(fd_PL[0], &LOG_ARRAY[temp_counter], sizeof(LOG_ARRAY[0]));
                            //printf("%s\n",LOG_ARRAY[temp_counter]);
                            temp_counter++;
                        }
                    }
                    pilot5 = 1;
                    temp_counter = 0;
                }

                // Finally we print everything into 2 files
                else if ((pilot1 == 1) && (pilot2 == 1) && (pilot3 == 1) && (pilot4 == 1) && (pilot5 == 1))
                {
                    i = 0;
                    fp = fopen("PLOT.txt", "w"); // "w" means that we are going to write on this file
                    while (i != (counter))
                    {
                        fprintf(fp, "%f %s", numbers_array[i], date_array[i]);
                        //printf("%f %s", numbers_array[i], date_array[i]);
                        i++;
                    }
                    fclose(fp); //Don't forget to close the file when finished

                    i = 0;
                    fp2 = fopen("LOG.txt", "w"); // "w" means that we are going to write on this file
                    while (i != (log_counter))
                    {
                        fprintf(fp2, "%s\n", LOG_ARRAY[i]);
                        //printf("%s\n", LOG_ARRAY[i]);
                        i++;
                    }
                    fclose(fp2); //Don't forget to close the file when finished

                    // Reset all the variables
                    i = 0;
                    pilot1 = 0;
                    pilot2 = 0;
                    pilot3 = 0;
                    pilot4 = 0;
                    pilot5 = 0;
                }
            }

            return 0;
        }
        else
        {
            // FATHER (S)
            G = fork();
            if (G < 0)
            {
                perror("Fork");
                return -1;
            }
            // G
            else if (G == 0)
            {
                printf("G - AWAKE!\n");

                // Give some time for the user to get ready
                sleep(5);

                // Close unnecessary pipes
                close(fd_GP[0]); // read from P
                close(fd_SP[0]); // unrelated pipe
                close(fd_SP[1]); // unrelated pipe
                close(fd_PL[0]); // unrelated pipe
                close(fd_PL[1]); // unrelated pipe

                // Stablish a connection to P via socket
                connect_fileserver_tcp();

                // I execute the loop a finite number of times, but the code is meant to run forever with a "while(1)"
                int k;
                for (k = 0; k < number_of_iterations; ++k)
                {

                    //Initialize the timeout data structure for the select
                    FD_ZERO(&set);
                    FD_SET(tcp_client_socket_fd, &set);
                    timeout.tv_sec = 0;
                    timeout.tv_usec = atoi(My_WT);

                    int ret = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
                    if (ret == 0)
                    {
                        //printf("No data from socket (P-G)\n");
                    }
                    else if (ret < 0)
                    {
                        // error occurred
                    }
                    else
                    {
                        // We call the function to read from the socket
                        receive_data();
                        // And we send the received message to P using the pipe
                        write(fd_GP[1], input_buffer, sizeof(input_buffer));
                    }
                }

                // Finally we close the pipes and the socket connection
                close(fd_GP);
                close(tcp_client_socket_fd);

                return 0;
            }
            // TRUE FATHER (S)
            else
            {
                printf("S - AWAKE!\n");
                // Close unnecessary pipes
                close(fd_GP[0]); // unrelated pipe
                close(fd_GP[1]); // unrelated pipe
                close(fd_SP[0]); // read from P
                close(fd_PL[0]); // unrelated pipe
                close(fd_PL[1]); // unrelated pipe

                char S_message[BUFFER_SIZE];

                // This process is always looking for an input text, and checks wheter is one of the following
                // if is not, it does nothing
                while (1)
                {
                    char *text = calloc(1, 1), buffer[BUFFERSIZE];
                    printf("Enter a message: \n");
                    while (fgets(buffer, BUFFERSIZE, stdin)) /* break with ^D or ^Z */
                    {
                        text = realloc(text, strlen(text) + 1 + strlen(buffer));
                        strcat(text, buffer); /* note a '\n' is appended here everytime */
                        if (strcmp(text, "start") == 0 || strcmp(text, "start\n") == 0)
                        {
                            printf("START RECEIVED!\n");
                            // We sent a SIGCONT to all the children in order to resume them
                            kill(P, SIGCONT);
                            kill(G, SIGCONT);
                            kill(L, SIGCONT);
                            startsignal = 1;
                        }
                        else if (strcmp(text, "stop") == 0 || strcmp(text, "stop\n") == 0)
                        {
                            printf("STOP RECEIVED!\n");
                            // We sent a SIGSTOP to all the children in order to stop them
                            kill(P, SIGSTOP);
                            kill(G, SIGSTOP);
                            kill(L, SIGSTOP);
                            stopsignal = 1;
                        }
                        else if (strcmp(text, "dump log") == 0 || strcmp(text, "dump log\n") == 0)
                        {
                            printf("DUMP LOG RECEIVED!\n");

                            dump_log = 1;
                        }
                        break;
                    }

                    // We will create and send to P an antry for the log regarding the start command
                    if (startsignal == 1)
                    {
                        gettimeofday(&tv2, NULL);
                        tm = localtime(&tv2.tv_sec);
                        sprintf(S_message, "%d:%02d:%02d.%d From S: Start Signal\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec);
                        char buff[sizeof(S_message)];
                        write(fd_SP[1], S_message, sizeof(S_message));
                        startsignal = 0;
                    }

                    // We will create and send to P an antry for the log regarding the stop command
                    if (stopsignal == 1)
                    {
                        gettimeofday(&tv2, NULL);
                        tm = localtime(&tv2.tv_sec);
                        sprintf(S_message, "%d:%02d:%02d.%d From S: Stop Signal\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec);
                        char buff[sizeof(S_message)];
                        write(fd_SP[1], S_message, sizeof(S_message));
                        stopsignal = 0;
                    }

                    // We will create and send to P an antry for the log regarding the dump log command
                    if (dump_log == 1)
                    {
                        gettimeofday(&tv2, NULL);
                        tm = localtime(&tv2.tv_sec);
                        sprintf(S_message, "%d:%02d:%02d.%d From S: Dump Log\n", tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec);
                        char buff[sizeof(S_message)];
                        write(fd_SP[1], S_message, sizeof(S_message));
                        stopsignal = 0;
                    }
                }
                // Finally we close the pipes and the socket connection
                close(fd_SP);

                return 0;
            }
        }
    }
}

// FUNCTIONS
// Function to get the parameters from the Config.txt file
void read_file()
{
    FILE *file;
    file = fopen("configuration.txt", "r"); //r means read

    // Store each line from the file into the array programs
    while (fgets(line, sizeof line, file) != NULL)
    {
        //store each line into variable programs
        programs[i] = strdup(line);
        i++;
        //count number of programs in file
        numProgs++;
    }
    fclose(file);

    i = 0;
    // We check if each of this read lines, correspond to the parameters we want to change
    // The next line after the found parameter, corresponds to the value of the actual parameter
    while (i != numProgs)
    {
        if (strcmp(programs[i], "RF:\n") == 0)
        {
            // When we find the parameter RF, we jump to the next line, and aquire the true value of RF
            i = i + 1;
            My_RF = programs[i];
        }
        else if (strcmp(programs[i], "IP Adress:\n") == 0)
        {
            // When we find the parameter IP Adress, we jump to the next line, and aquire the true value of IP Adress
            i = i + 1;
            My_IP = programs[i];
        }
        else if (strcmp(programs[i], "PORT:\n") == 0)
        {
            // When we find the parameter PORT, we jump to the next line, and aquire the true value of PORT
            i = i + 1;
            My_PORT = programs[i];
        }
        else if (strcmp(programs[i], "Waiting Time:\n") == 0)
        {
            // When we find the parameter Waiting Time, we jump to the next line, and aquire the true value of Waiting Time
            i = i + 1;
            My_WT = programs[i];
        }
        i++;
    }

    //The function replace_and_remove, removes the final "\n" that accidentally we copy when reading the file
    My_IP = replace_and_remove(My_IP);
    My_PORT = replace_and_remove(My_PORT);
    My_RF = replace_and_remove(My_RF);
    My_WT = replace_and_remove(My_WT);
}

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

    printf("Connection established :)\n");

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

// We could not delete "/n" from the end of an array of chars in a straigthforward way so we needed this 2 functions;
// Function to remove " " from an array of chars
void remove_char_from_string(char c, char *str)
{
    int i = 0;
    int len = strlen(str) + 1;

    for (i = 0; i < len; i++)
    {
        if (str[i] == c)
        {
            strncpy(&str[i], &str[i + 1], len - i);
        }
    }
}

// Function to replace "\n" with " " in a array of chars
void replace_char_from_string(char from, char to, char *str)
{
    int i = 0;
    int len = strlen(str) + 1;

    for (i = 0; i < len; i++)
    {
        if (str[i] == from)
        {
            str[i] = to;
        }
    }
}

// So the combination of the previous two, brings the following:
// Function to remove "/n" from an array of chars
int replace_and_remove(char *incoming_string)
{
    char *original = incoming_string;
    int original_len = strlen(original) + 1;
    char *string = (char *)malloc(original_len);
    memset(string, 0, original_len);
    strncpy(string, original, original_len);
    replace_char_from_string('\n', ' ', string);
    remove_char_from_string(' ', string);
    return string;
}

// This function is used to send data from P to G through the socket connection
void send_data()
{
    //printf("Value to send through socket: %f\n", new_token);

    // We multiply the token by 1000000 toget rid of the floating point
    new_token = new_token * 1000000;

    // We serialise the token, meaning we combine the numerical value and the date into a single string
    serialize();

    if (send(client_sock_fd, output_buffer, strlen(output_buffer) + 1, 0) == -1)
    {
        fprintf(stderr, "[ERROR] Failed sending message: %s\n", strerror(errno));
    }
}

// This function is used to receive data from P to G through the socket connection
void receive_data()
{

    bzero(input_buffer, BUFFER_SIZE);

    int read_bytes = recv(tcp_client_socket_fd, input_buffer, BUFFER_SIZE, 0);
    if (read_bytes < 0)
    {
        fprintf(stderr, "[ERROR] Failed receiving message: %s\n", strerror(errno));
    }

    //fprintf(stdout, "[INFO #1] Received a message: %s\n", input_buffer);
}

// This function combines the value we want to send and the date into a single string, by transforming
// both into char arrays and combining both with a "_" between them
int serialize()
{
    int number_length = sizeof(new_token);
    char buf1[number_length];
    char buf2[useconds_length];

    int temporal_token;
    temporal_token = new_token;

    sprintf(buf1, "%d", temporal_token);

    strcpy(output_buffer, "");
    strcat(output_buffer, buf1);
    strcat(output_buffer, "_");

    sprintf(buf2, "%zu", useconds[0]);

    strcat(output_buffer, buf2);
}

// This function separetes the value we we have received and the date into 2 separate char variables
int deserialize()
{
    char *token = strtok(received_data, "_");     // split the received data through "_"
    sprintf(received_char_number, "%s\n", token); //printing each token
    //printf("received_char_number %s\n", received_char_number);

    token = strtok(NULL, "_");
    sprintf(received_char_time, "%s\n", token);
    //printf("received_char_time %s\n", received_char_time);

    return 0;
}
