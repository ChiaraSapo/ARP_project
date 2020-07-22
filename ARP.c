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

// Variables for signals
bool start=0;
bool stop=0; // If set: stop receiving token
bool dumpLog=0; // If set: dump log
int signalReceived; //start, sto, log 

double * dataFromConfig;

void sig_handler(int sig)
{
      if (sig == SIGCONT) // Restart receving
   {
      printf("SIGCONT received, pid = %d\n", getpid());
      kill(Pn,sig);
      kill(Gn,sig);
      start=1;
   }

   if ( sig == SIGUSR1) // Stop receiving
   {
      printf("Stop signal received, pid = %d\n", getpid());
      kill(Pn,SIGSTOP);
      kill(Gn,SIGSTOP);
      stop=1;
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
   double DT[1]; /////////////////////////////////////////////////
   
   // Start socket
   //start_fileserver_tcp();
   
   // Read config file
   char * fileName="config_file.cfg";
   dataFromConfig=read_config(fileName); // freq,IP
   double dataFromSn[4];
   dataFromSn[0]=dataFromConfig[0];
   dataFromSn[1]=dataFromConfig[1];
   dataFromSn[2]=dataFromConfig[2];

   // Check signals
   if (start==1)
      signalReceived=1;
   else if (stop==1)
      signalReceived=2;
   else if (stop==1)
      signalReceived=3;
   else 
      signalReceived=0;

   dataFromSn[3]=signalReceived;
   
   // Write on pipe1 frequency and IP
   int ctr=write(fd1[1], dataFromSn , sizeof(double)*4);  

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
         

         for(int i=0; i<1; i++){
            
            float tokenFromG;

            // Measure time 
            clock_t start, end;
            start=clock();
            
            // Select for pipe1
            double dataSnToPn[4];
            double dataPnToLn[3];
            dataPnToLn[2]=0;
            double freqFromConfig;
            fd_set rfds;
            struct timeval tv;
            int retval;
            FD_ZERO(&rfds);
            FD_SET(fd1[0], &rfds);
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            
            retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
            
            if (retval == -1)   
               perror("select()");

            else if (retval) {   

               printf("Data available in pipe1\n");

               int ctr2=read(fd1[0],dataSnToPn,sizeof(double)*3);
               
               // Save data (easier to read)
               freqFromConfig=dataSnToPn[0];
               int IPFromConfig=dataSnToPn[1];
               double waitTimeFromConfig=dataSnToPn[2];
               double signalFromSn=dataSnToPn[3];

               if(signalFromSn==0)
                  printf("\nNo signal");
               if(signalFromSn==1)
                  printf("\nStart");
               if(signalFromSn==2)
                  printf("\nStop");
               if(signalFromSn==3)
                  printf("\nLog");
                  dataPnToLn[2]=1;

               DT[0]=waitTimeFromConfig;         

               printf("Dataread from Sn in Pn:\n    Freq: %f, IP: %d, WaitTime: %f, \n", freqFromConfig , IPFromConfig ,waitTimeFromConfig);
            }
            else   
                printf("No data within 5 seconds.\n");
            


            // Select for pipe2
            fd_set rfds1;
            struct timeval tv1;
            int retval1;
            FD_ZERO(&rfds1);
            FD_SET(fd2[0], &rfds1);
            tv1.tv_sec = 2;
            tv1.tv_usec = 0;
            
            retval1 = select(FD_SETSIZE, &rfds1, NULL, NULL, &tv1);
            
            if (retval1 == -1)   
               perror("select()");

            else if (retval1) {   
               
               printf("Data available in pipe1\n");

               // Read pipe2 (Gn->Pn)
               int nb=read(fd2[0], &tokenFromG, sizeof(tokenFromG));
               printf("Token received: %f\n",tokenFromG);
               
               // Compute new token
               double newToken;
               newToken=tokenFromG+DT[i]*(1-(tokenFromG*tokenFromG)/2)*2*M_PI*freqFromConfig;
               printf("New token calculated in Pn: %f\n", newToken);


               // SOCKET TO GN+1!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
               //tm->tm_hour, tm->tm_min, tm->tm_sec, tv2.tv_usec


               // Measure again time after socket and update DT
               end = clock()-start;
               double time_taken = ((double)end)/CLOCKS_PER_SEC; // in seconds 
               DT[i+1]=DT[i]+time_taken;
               printf("DT is equal to: %f\n", DT[i]);

               // Prepare for Ln
               
               dataPnToLn[0]=tokenFromG; 
               dataPnToLn[1]=newToken;
               
               // Write on pipe 3 what to write on log file
               write(fd3[1], dataPnToLn, sizeof(double)*3); // Pn writes to pipe fd2
            }
            else   
                printf("No data within 5 seconds.\n");
         }
         

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
            for(int j=0;j<1;j++){
               // Reads what to write on log file: put after log
               double dataFromPn[3];
               read(fd3[0], dataFromPn, sizeof(double)*2); // Ln reads from pipe fd2
               printf("Tokens in Ln:\n    Old token: %f, new token:%f\n\n\n",dataFromPn[0],dataFromPn[1]);

               char * logFileName = "log.txt";
               writeLogFile(logFileName, dataFromPn[0], dataFromPn[1]);
               if (dataFromPn[2]==1)
                  dumpLogFile(logFileName);
            }

         }
         waitpid(Ln,0,0);
      }
   }

   waitpid(Pn,0,0);
   return 0;
}

