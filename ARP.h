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


// FUNCTIONS NEEDED BY MAIN CODE 

/* Reads config file
      input: file name
      output: array of frequency, IP
*/
double * read_config(char * fileName) {

   FILE * infile = fopen(fileName, "r");

    int size=3;
    double * data = malloc(size);

    // Open file 
    if ( !infile ) {
        perror("couldn't open file");
        //return EXIT_FAILURE;
    }

    char buffer[MAX_LEN];
    char * endptr;

    // Read file
    while ( fgets(buffer, MAX_LEN, infile) ) {
	
        // Look for the frequency
        if ( !strncmp(buffer, "Frequency=", FREQ_LEN ) ) {
            char * num_start = buffer + FREQ_LEN;
            double freq = strtod(num_start, &endptr);

            if ( endptr == num_start ) {
            fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[0]=freq;
        }

        // Look for the IP
        else if ( !strncmp(buffer, "IP=", IP_LEN) ) {
            char * num_start = buffer + IP_LEN;
            double ip = strtod(num_start, &endptr);

            if ( endptr == num_start ) {
                fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[1]=ip;
        }

        // Look for the IP
        else if ( !strncmp(buffer, "WaitTime=", WAIT_LEN) ) {
            char * num_start = buffer + WAIT_LEN;
            double waitingTime = strtod(num_start, &endptr);

            if ( endptr == num_start ) {
                fprintf(stderr, "Badly formed input line: %s\n", buffer);
            }
            data[2]=waitingTime;
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

void writeLogFile(char * fileName, double token1, double token2) {
            // Start writing log process
            printf("Log dumping...\n");
            time_t timer;
            char buffer[26];
            struct tm* tm_info;
            
            // Note down current time
            timer = time(NULL);
            tm_info = localtime(&timer);

            // Save current time in a string
            strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm_info);

            // Append a new line to the log file
            FILE * fp = fopen(fileName, "a");
            char * data_1 = "LOG: ";
            char * data_open = "<"; 
            char * data_2 = "> <from G | from S> <";
            char * data_close = ">";
            char * data_enter = "\n";
            
            char * value1=malloc(sizeof(double));
            memcpy(value1,&token1,sizeof(double));
            
            char * value2=malloc(sizeof(double));
            memcpy(value2,&token2,sizeof(double));
            
            fputs(data_1, fp);
            fputs(data_open, fp);
            fputs(buffer, fp);
            fputs(data_2, fp);
            fprintf(fp,"%f",token1);
            fputs(data_close, fp);
            fputs(data_enter, fp);

            fputs(data_open, fp);
            //timestamp
            fputs(data_close, fp);
            fputs(data_open, fp);
            fprintf(fp,"%f",token2);
            fputs(data_close, fp);
            fputs(data_enter, fp);
            fclose(fp);

            printf("Log file written\n");
}


/* Writes on screen the content of the log file
      input: file name
      output: none
*/
void dumpLogFile(char * fileName)  {
      
      char buff[FILE_LINE_LENGH]; // Contains the read line 
               
      // Read the last line of the file
      FILE * fp = fopen(fileName, "r");
      fseek(fp, -FILE_LINE_LENGH, SEEK_END);    // set pointer to the end of file minus the length you need. There can be more than one new line caracter
      fread(buff, FILE_LINE_LENGH-1, 1, fp); 
      fclose(fp);

      // Print the line to screen
      printf("%s",buff);
} 
