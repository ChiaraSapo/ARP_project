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
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "ARP.h"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

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

    for (int i; i < loops; i++)
    {
        float receivedToken = 0.5; // Read from socket!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

        // G has received token
        if (receivedToken == 0)
        {
            tokenToSend = computeOriginalToken(-1, 1);
        }

        // G has found pipe empty
        else
        {
            tokenToSend = receivedToken;
        }

        printf("\n\nGn code, pid = %d\n", getpid());
        printf("    Token to send: %f\n", tokenToSend);

        // write on pipe
        int ctr = write(atoi(argv[2]), &tokenToSend, sizeof(tokenToSend)); //write on the pipe to child2
        if (ctr < 0)
            printf("No data sent from G");
    }
    //}
    //close(newsockfd);
    //close(sockfd);

    return 0;
}
