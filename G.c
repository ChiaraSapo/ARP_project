#include "ARP.h"
#include <errno.h> // useful for errno
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //useful for strlen (modify the strings)
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


#define PARAM_CLIENT_PORT 7891
#define PARAM_CLIENT_ADDRESS "127.0.0.1"

/*void error(const char *msg)
{
    perror(msg);
    exit(1);
}*/

float computeOriginalToken(const float min, const float max)
{
    if (max == min)
        return min;
    else if (min < max)
        return (max - min) * ((float)rand() / RAND_MAX) + min;
    else if (max < min)
        return -1;
}

int main(int argc, char *argv[])
{

    float tokenToSend;
    int socket_available = 1;
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    printf("\n\nGn code, pid = %d\n", getpid());

    for (int i = 0; i < loops; i++)
    {
          /*---Create the socket. The three arguments are: ---*/
        // 1)internet dumain 2)stream socket 3) default protocol
        clientSocket= socket(PF_INET, SOCK_STREAM, 0);

        /*----Configure settings of the server adress struct ---*/
        //Address family : Internet
        serverAddr.sin_family = AF_INET;
        //Set port number, using htons funcione to use proper byte order
        serverAddr.sin_port = htons(PARAM_CLIENT_PORT);
        //Set IP address to localhost
        serverAddr.sin_addr.s_addr = inet_addr(PARAM_CLIENT_ADDRESS);
        //Set all bits of the padding field to 0
        memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

        /*---Connect the socket to the server using the address struct ---*/
        addr_size = sizeof(serverAddr);
        connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

        /*---Read the message from the server into the buffer ---*/
        recv(clientSocket, &receivedToken, sizeof(receivedToken), 0);
        // Read socket: CLIENT
        //socket_client();
        //printf("Socket available\n");
        float receivedToken = 0.5; // Read from socket!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // G has received token
        if (socket_available == 0)
        {
            tokenToSend = computeOriginalToken(-1, 1);
        }

        // G has found socket empty
        else
        {
            tokenToSend = receivedToken;
        }

        printf("    Token to send: %f\n", tokenToSend);

        // Write on pipe
        int ctr = write(atoi(argv[2]), &tokenToSend, sizeof(tokenToSend)); //write on the pipe to child2
        if (ctr < 0)
            printf("    No data sent from G to P");
        else
            printf("    Pipe written from G to P\n\n");
    }

    return 0;
}
