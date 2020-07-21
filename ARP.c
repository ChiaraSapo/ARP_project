/*  gcc G.c -o G
    gcc ARP.c -o ARP
    '/home/chiara/try/ARP'
*/ 

#include "ARP.h"

#define MAX_LEN 100
#define FREQ_LEN 10
#define IP_LEN 3

#define M_PI 3.14159265358979323846
#define FILE_LINE_LENGH 26

// Processes' IDs
pid_t Pn;          
pid_t Gn;
pid_t Ln;

// Variables for pipes
int fd1[2], fd2[2], fd3[2];

bool stop=0; // If set: stop receiving token
bool dumpLog=0; // If set: dump log
double * dataFromConfig;

void sig_handler(int sig)
{
      if (sig == SIGCONT) // Restart receving
   {
      printf("SIGCONT received, pid = %d\n", getpid());
      kill(Pn,sig);
      kill(Gn,sig);
      //printf("Stop equal to %d\n",stop);
      //signal (SIGCONT, SIG_DFL); //the default response is defined again
   }

   if ( sig == SIGUSR1) // Stop receiving
   {
      printf("Stop signal received, pid = %d\n", getpid());
      kill(Pn,SIGSTOP);
      kill(Gn,SIGSTOP);
   }

   if (sig == SIGUSR2) // Dump log
   {
      printf("Dump log signal received\n");
      dumpLog=1;
   }
}




int main (int argc, char *argv[]) {          // ---> Sn 
/* Sn:
   Handles signals                                   (x)
   Reads config file                                 (v)
   Writes data on pipe to Pn                         (v)
   Creates Pn                                        (v)
*/
   
   // Creation of the pipes
   pipe(fd1); // Sn <---> Pn   freq,ip,waitTime
   pipe(fd2); // Gn <---> Pn   token
   pipe(fd3); // Pn <---> Ln   token

   printf("\nSn code, pid = %d\n", getpid());

   // Link signals to signal handler
   signal(SIGCONT, sig_handler);
   signal(SIGUSR1, sig_handler);
   signal(SIGUSR2, sig_handler); //log signal
   double DT[1]; ////////////////////////////////////////

   // Read config file
   char * fileName="config_file.cfg";
   dataFromConfig=read_config(fileName); // freq,IP
   
   // Write on pipe1 frequency and IP
   int ctr=write(fd1[1], dataFromConfig , sizeof(double)*3);  

   // Creation Sn-->Pn
   Pn = fork(); 

   // Control on the fork   
   if (Pn < 0){
      perror("Fork error");
      return -1;
   }

   else if  (Pn==0){     // ---> Pn
      
      printf("\n\nPn code, pid = %d\n", getpid());
      
      // Prepare to create Gn
      char *arg[4];    //array to pass to the executable  
      char targ1[5];
      char targ2[5];
      sprintf(targ1,"%d",fd2[0]);
      sprintf(targ2,"%d",fd2[1]);
      arg[1]=targ1;//pipe: reading
      arg[2]=targ2;//pipe: writing
      arg[3]=NULL;

      // Creation Pn-->Gn
      Gn=fork();              
      
      // Control on the fork   
      if (Gn < 0){
         perror("Fork error");
         return -1;
      }
      
      /*-----------------------------------Gn----------------------------*/
      else if  (Gn==0){    // ---> Gn
         // Exec to get code of Gn
         char *fname1="./G";   
         arg[0]=fname1; 
         execvp(fname1,arg);  
         perror ("Exec function of G");
         
      }


      /*-----------------------------------Pn----------------------------*/
      else{  
                                    
         /* Pn 
            Reads pipe from Sn                           (v)                   
            Creates pipe with Gn                         (v)
            Creates Gn with exec                         (v)
            Reads pipe from Gn                           (v)
            Computes new token                           (v)
            Writes pipe to Ln                            (v)
            Creates socket to Gn+1                       (x)
            Writes socket to Gn+1                        (x)
         */  

         waitpid(Gn,0,0); // wait for Gn

         printf("\n\nPn code pt2, pid = %d\n", getpid());
         
         //while(1) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11111
         float tokenFromG;

         // Measure time 
         clock_t start, end;
         start=clock();

         //IMPLEMENT SELECT HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
         
         
         
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
         // Read from pipe1 (Sn->Pn)
         double dataSnToPn[2];
         int ctr2=read(fd1[0],dataSnToPn,sizeof(double)*3);
         
         // Save data (easier to read)
         double freqFromConfig=dataSnToPn[0];
         int IPFromConfig=dataSnToPn[1];
         double waitTimeFromConfig=dataSnToPn[2];
         DT[0]=waitTimeFromConfig;         

         printf("Dataread from config file in Pn:\n    Freq: %f, IP: %d, WaitTime: %f\n", freqFromConfig , IPFromConfig ,waitTimeFromConfig);
      
         int nb=read(fd2[0], &tokenFromG, sizeof(tokenFromG));
         printf("Token received: %f\n",tokenFromG);
         int i=0; ///////////////////////////////////////////////////////////////////
         
         // Compute new token
         double newToken;
         newToken=tokenFromG+DT[i]*(1-(tokenFromG*tokenFromG)/2)*2*M_PI*freqFromConfig;
         printf("New token calculated in Pn: %f\n", newToken);

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







         // SOCKET TO GN+1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
         
         // Measure again time after socket and update DT
         end = clock()-start;
         double time_taken = ((double)end)/CLOCKS_PER_SEC; // in seconds 
         DT[i+1]=DT[i]+time_taken;
         printf("DT is equal to: %f\n", DT[i]);

         // Prepare for Ln
         double dataPnToLn[2];
         dataPnToLn[0]=tokenFromG; 
         dataPnToLn[1]=newToken;
         
         // Write on pipe 3 what to write on log file
         write(fd3[1], dataPnToLn, sizeof(double)*2); // Pn writes to pipe fd2
         

         /*-----------------------------------Ln----------------------------*/
         // Create Ln
         Ln=fork();   

         // Control on the fork           
         if (Ln < 0){
            perror("Fork error");
            return -1;
         }
      
         else if  (Ln==0){    // ---> Ln
            /* Ln
               Reads pipe from Pn                     (v)
               Writes on log file                     (-)
               Dumps log file on request              (v)
            */
         
            printf("\n\nLn code, pid = %d\n", getpid());
            
            // Reads what to write on log file: put after log
            double dataFromPn[2];
            read(fd3[0], dataFromPn, sizeof(double)*2); // Ln reads from pipe fd2
            printf("Tokens in Ln:\n    Old token: %f, new token:%f\n",dataFromPn[0],dataFromPn[1]);

            char * logFileName = "log.txt";
            writeLogFile(logFileName, dataFromPn[0], dataFromPn[1]);

            // Dump log
            //if(dumpLog)
               dumpLogFile(logFileName);

         }
         waitpid(Ln,0,0);
      }
   }
   //} //end for loop

   waitpid(Pn,0,0);
   return 0;
}
