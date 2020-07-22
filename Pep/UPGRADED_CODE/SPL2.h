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

