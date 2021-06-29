#include "functions.h"


int main(int argc, char *argv[])
{
    char *log_file;
    char *IP_address;
    const char *process_G_path;
    const char *first_token;
    double RF;
    int number_of_ports;
    const char *waiting_time;
    const char *pid_P_fileName; 
    const char *pid_G_fileName; 


    // Take useful data from input
    log_file = argv[1];
    IP_address = argv[2];
    RF = atof(argv[3]);
    process_G_path = argv[4];
    number_of_ports = atoi(argv[5]);
    first_token = argv[6];
    waiting_time = argv[7];
    pid_P_fileName = argv[8]; 
    pid_G_fileName = argv[9];     

    if (argc < 10)
    {
        fprintf(stderr, "Missing arguments\n");
        exit(-1);
    }

    // Variables declaration
    pid_t pid[3];
    pid_t pid_P;
    pid_t pid_G;
    pid_t pid_L;

    long startTime, endTime;
    float DT;
    double new_token;
    const int buffer_size = 50;
    FILE *logname;
    char buffer_LP[buffer_size], buffer_SP[buffer_size];
    ssize_t token_from_G, action, dumplog, comm_value;
    int first = 0; // If set: first loop of the machine

    // Select Variables declaration
    fd_set rfds_P, rfds_S, wfds_S;
    struct timeval tvP, tvL, tvS;
    int retval;
    int nfds;     

    // Socket variables declaration 
    int sockfd, newsockfd, clilen;
    char server_buf[256];
    struct sockaddr_in serv_addr, cli_addr;

    // Signal variables
    const int start = 1; 
    const int stop = 2;  
    const int log = 3;   

    // Pipes initialization
    int pipe_SP[2];
    int pipe_GP[2];
    int pipe_LP[2];

    // Pipes creation
    if (pipe(pipe_SP) == -1)
    {
        perror("pipe_SP creation");
        exit(-1);
    }
    if (pipe(pipe_GP) == -1)
    {
        perror("pipe_GP creation");
        exit(-1);
    }
    if (pipe(pipe_LP) == -1)
    {
        perror("pipe_LP creation");
        exit(-1);
    }

    printf("Select:\n'1' to start\n'2' to stop\n'3' to dump the log file\n\n");

    // First fork
    pid[0] = fork();
    if (pid[0] < 0)
    {
        perror("First fork");
        return -1;
    }

    /********************************************/
    /********************Pn**********************/
    /********************************************/
    if (pid[0] > 0)
    {
        int n;
        const int W = 1;
        printf("Process P starts with pid: %d\n", getpid());
        pid_P = getpid();

        // Save pid_P on an external file
	    save_pid_to_file_fnct(pid_P_fileName, pid_P);

        // loop
        while (1)
        {
            //Define buffer for P and for string to write in log file 
            char bufferP[buffer_size];
            char string_log[buffer_size * 2];

            // ----------------------------- Initialize select G and S ----------------------------------//
            FD_ZERO(&rfds_P);
            FD_SET(pipe_GP[0], &rfds_P);
            FD_SET(pipe_SP[0], &rfds_P);
            close(pipe_SP[1]);
            close(pipe_GP[1]);

            // Choose file descriptor
            if (pipe_GP[0] > pipe_SP[0])
                nfds = pipe_GP[0] + 1;
            else
                nfds = pipe_SP[0] + 1;

            tvP.tv_sec = 0;
            tvP.tv_usec = 1000;

            //Execute ONCE (the first time): send the default token to G
            if (first == 0)
            {
                time_t taketime = time(NULL);
                char temp_new_token[buffer_size];

                // Create string for log file 
                int n = sprintf(string_log, "time: %s| G starts with default Token: %s\n", ctime(&taketime), first_token);
                if (n == -1)
                {
                    perror("String creation");
                    exit(-1);
                }

                //----------------------------------Initialize socket--------------------------------------//
                newsockfd=create_socket_fnct(sockfd, W, serv_addr,  cli_addr, clilen, number_of_ports, server_buf);

                // Write socket: first_token to G
                n = write(newsockfd, first_token, sizeof(first_token));
                if (n < 0)
                {
                    perror("Writing to socket");
                    exit(-1);
                }
                taketime = time(NULL);

                // Create string to send to L with the first_token line
                int m = sprintf(temp_new_token, "%s First Token: %s\n\n", ctime(&taketime), first_token);
                if (m == -1)
                {
                    perror("String creation");
                    exit(-1);
                }
                int o = sprintf(string_log, "%s%s", string_log, temp_new_token);
                if (o == -1)
                {
                    perror("String creation");
                    exit(-1);
                }

                // Set first to 1 so that this piece of code is not executed again
                first = 1; 

            } // Close first=0

            else
            {
                //-------------------------------Select G and S--------------------------------//
                retval = select(nfds, &rfds_P, NULL, NULL, &tvP);
                if (retval == -1)
                {
                    perror("Select creation");
                    exit(-1);
                }

                else if (retval)
                {
                    // P reads pipe_GP  
                    if (FD_ISSET(pipe_GP[0], &rfds_P))
                    {
                        // Read token from G 
                        token_from_G = read(pipe_GP[0], bufferP, buffer_size);

                        if (token_from_G == -1)
                        {
                            perror("No token from G\n");
                            exit(-1);
                        }

                        startTime = getTime();
                        bufferP[token_from_G] = '\0';
                        char temp_new_token[buffer_size];

                        // Calculate timestamp
                        time_t taketime = time(NULL);

                        // Create string for log file 
                        int n = sprintf(string_log, "\ntime: %s G sent the Token: %s\n", ctime(&taketime), bufferP);
                        if (n == -1)
                        {
                            perror("String creation");
                            exit(-1);
                        }

                        // Initialize socket
			            newsockfd = create_socket_fnct(sockfd,  W,  serv_addr,  cli_addr,  clilen, number_of_ports, server_buf);

                        // Calculate time 
                        endTime = getTime();

                        // Calculate DT
                        DT = (double)(endTime-startTime)/1000000;

                        // Calculate new_token
                        char tokenBuf[buffer_size];
                        new_token = ComputeToken(atof(bufferP), DT, RF);
                        sprintf(tokenBuf, "%f", new_token);

                        // Write new_token on socket 
                        n = write(newsockfd, tokenBuf, buffer_size);
                        if (n < 0)
                        {
                            perror("Writing to socket");
                            exit(-1);
                        }

                        // Complete the string to send to L with the new_token line
                        int m = sprintf(temp_new_token, " Updated Token is %f", new_token);
                        if (m == -1)
                        {
                            perror("String creation");
                            exit(-1);
                        }
                        int p = sprintf(string_log, "%s%s\n\n", string_log, temp_new_token);
                        if (p == -1)
                        {
                            perror("String creation");
                            exit(-1);
                        }
                    } // Close Reading of pipe GP

                    // P reads pipe_SP 
                    else if (FD_ISSET(pipe_SP[0], &rfds_P))
                    {
                        // Read action from S
                        action = read(pipe_SP[0], bufferP, buffer_size);
                        bufferP[action] = '\0';
                        time_t taketime = time(NULL);

                        // Create string to be sent to L (log_file)
                        if (sprintf(string_log, "\ntime: %s New action from S: %s\n", ctime(&taketime), bufferP) == -1)
                        {
                            perror("String creation");
                            exit(-1);
                        }

                    } // Close Reading of pipe SP
                } // Close select S and G
            }

            // Send string_log to L 
            write(pipe_LP[1], string_log, buffer_size*2);

        } // Close P while loop
    } // Close PID0 > 0


    else if (pid[0] == 0)
    {
        // Second fork
        pid[1] = fork();
        if (pid[1] < 0)
        {
            perror("Second fork");
            exit(-1);
        }

        /********************************************/
        /********************Ln**********************/
        /********************************************/
        if (pid[1] > 0)
        {

            printf("Process L starts with pid: %d\n", getpid());
            pid_L = getpid();

            //loop
            while (1)
            {
                // Open log_file
                logname = fopen(log_file, "a");
                if (logname == NULL)
                {
                    perror("Opening log file");
                    exit(-1);
                }

                // Read action from the pipe 
                dumplog = read(pipe_LP[0], buffer_LP, buffer_size*2);
                
                if (dumplog == -1)
                {
                    perror("Reading from pipe_LP");
                    exit(-1);
                }

                buffer_LP[dumplog] = '\0';

                // Write on log file 
                if (fputs(buffer_LP, logname) == EOF)
                {
                    perror("Writing log file");
                    exit(-1);
                }
                fclose(logname);

                
            } // Close L while loop 

        } // Close PID1 > 0


        else if (pid[1] == 0)
        {
            // Third fork
            pid[2] = fork();
            if (pid[2] < 0)
            {
                perror("Third fork");
                exit(-1);
            }

            /********************************************/
            /********************Gn**********************/
            /********************************************/
            if (pid[2] > 0)
            {
                printf("Process G starts with pid: %d\n", getpid());	
                pid_G=getpid();

                // Save pid_G on an external file
                save_pid_to_file_fnct(pid_G_fileName, pid_G);

                char *num_port;
                char pipeGP_0[5];
                char pipeGP_1[5];

                // Prepare the variables to be given to G
                if (sprintf(pipeGP_0, "%i", pipe_GP[0]) < 0)
                {
                    perror("String creation");
                    exit(-1);
                }
                if (sprintf(pipeGP_1, "%i", pipe_GP[1]) < 0)
                {
                    perror("String creation");
                    exit(-1);
                }

                num_port = argv[5];

                // exec G by passing the necessary arguments
                if (execl(process_G_path, IP_address, num_port, pipeGP_0, pipeGP_1, waiting_time, NULL) == -1)
                {
                    perror("Exec failed");
                    exit(-1);
                }
            } // Close PID2 > 0

            /********************************************/
            /********************Sn**********************/
            /********************************************/
            else
            {
                printf("Process S starts with pid: %d\n", getpid());

                // Get process P pid from file
                int pid_P_;
                pid_P_ = read_pid_from_file_fnct(pid_P_fileName);

                // Get process G pid from file
                int pid_G_;
                pid_G_ = read_pid_from_file_fnct(pid_G_fileName);

                // Stop processes
                kill(pid_P_, SIGSTOP);
                kill(pid_G_, SIGSTOP);
                printf("The program is stopped.\n");

                //loop
                while (1)
                {
                    // ----------------------------- Initialize select P and S ----------------------------------//
                    FD_ZERO(&rfds_S);
                    FD_ZERO(&wfds_S);
                    FD_SET(STDIN_FILENO, &rfds_S);
                    FD_SET(pipe_SP[1], &wfds_S);
                    close(pipe_SP[0]);

                    tvS.tv_sec = 0;
                    tvS.tv_usec = 1000;

                    int ret = select(pipe_SP[1] + 1, &rfds_S, &wfds_S, NULL, NULL);
                    if (ret == -1)
                    {
                        perror("select creation");
                        exit(-1);
                    }

                    else if (ret)
                    {
                        // S reads command from console 
                        if (FD_ISSET(STDIN_FILENO, &rfds_S))
                        {
                            // Read command from terminal
                            comm_value = read(STDIN_FILENO, buffer_SP, buffer_size);
                            if (comm_value == -1)
                            {
                                perror("Reading from terminal");
                                exit(-1);
                            }
                            buffer_SP[comm_value] = '\0';

                            // 1. If command is START
                            if (atoi(buffer_SP) == start)
                            {
                                kill(pid_P_, SIGCONT);
                                kill(pid_G_, SIGCONT);

                                printf("Starting\n");

                                // Send action to P
                                if (FD_ISSET(pipe_SP[1], &wfds_S))
                                    write(pipe_SP[1], buffer_SP, buffer_size);
                            }

                            // 2. If command is STOP
                            else if (atoi(buffer_SP) == stop)
                            {
                                // Send action to P
                                if (FD_ISSET(pipe_SP[1], &wfds_S))
                                    write(pipe_SP[1], buffer_SP, buffer_size);

                                kill(pid_P_, SIGSTOP);
                                kill(pid_G_, SIGSTOP);

                                printf("Stopping\n");
                            }

                            // 3. If command is dump log 
                            else if (atoi(buffer_SP) == log)
                            {
                                int dump_size = buffer_size * 20;

                                // Send action to P 
                                if (FD_ISSET(pipe_SP[1], &wfds_S))
                                    write(pipe_SP[1], buffer_SP, buffer_size);

				                dump_log_fnct(log_file, logname, dump_size, buffer_LP);

                            }
                            else
                            {
                                printf("Select:\n'1' to start\n'2' to stop\n'3' to dump the log file\n\n");
                            }
                        }
                    } // Close Select P
                } // Close S while loop
            } // Close PID2 > 0
        } // Close PID1 = 0
    }

    return 0;
}




