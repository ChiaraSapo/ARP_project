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
  else if (min<max)
      return (max - min) * ((float)rand() / RAND_MAX) + min;
   else if (max<min)
      return -1;
}

int main (int argc, char *argv[]) {
    
    float tokenToSend;

    ////////////
/*
    // Read socket
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    float buffer;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2)
    {
        fprintf(stderr, "ERROR, not enougth parameters\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]); // port number in the first position of argv

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr,
            sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd,
                    (struct sockaddr *)&cli_addr,
                    &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    bzero(&buffer,sizeof(buffer));

    while (1){

        n = read(newsockfd, &buffer, sizeof(buffer)); //leggo dato socket
        if (n < 0)
            error("ERROR reading from socket");

        ///////
*/      for(int i; i<loops; i++){
            float receivedToken=0.5; // Read from socket!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

            // G has received token
            if(receivedToken==0){
                tokenToSend=computeOriginalToken(-1,1);
            }

            // G has found pipe empty
            else{
                tokenToSend=receivedToken;
            }
            
            printf("\n\nGn code, pid = %d\n", getpid());
            printf("Token to send: %f\n", tokenToSend);

            // write on pipe
            write(atoi(argv[2]), &tokenToSend,sizeof(tokenToSend));  //write on the pipe to child2
            
            if (ctr < 0)
                error("ERROR writing to socket");
        }
    //}
    //close(newsockfd);
    //close(sockfd);
    
    return 0;


}
