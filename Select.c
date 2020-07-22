while (1) {
            fd_set rfds;
            struct timeval tv;
            struct timespec tv2;
            int retval, m,n;
         
            FD_ZERO(&rfds);
            FD_SET(fd1[0], &rfds);
            
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            
            retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
            
            if (retval == -1)   
               perror("select()");

            else if (retval) {   
               printf("Data available in pipe1\n");


               read(fd1[0], &actualToken, sizeof(actualToken));
               
            }
             else   
                printf("No data within 5 seconds.\n");
         }