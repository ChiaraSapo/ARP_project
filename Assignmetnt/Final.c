#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

double newToken;
double *recivedMsg;
char *timeString;
int flag;
pid_t pid_S, pid_G, pid_L, pid_P;

char *signame[] = {"INVALID", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP", "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1", "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM", "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP", "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ", "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR", "SIGSYS", NULL};

char *myfifo1 = "myfifo1"; //path
char *myfifo2 = "myfifo2"; //path
char *myfifo3 = "myfifo3"; //path

//void logFile(pid_t process_id, double message);
//void error(const char *msg);

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

void logFile(pid_t process_id, double message)
{
	FILE *f;
	f = fopen("logFile.log", "a");
	time_t currentTime;
	currentTime = time(NULL);
	timeString = ctime(&currentTime);
	fprintf(f, "-%sPID: %d value:%.3f.\n", timeString, process_id, message);
	fprintf(f, "-%s%.3f.\n\n", timeString, newToken);

	fclose(f);
}

void sig_handler(int signo)
{
	if (signo == SIGUSR1) //sarebbe il segnale per caricare il log.
	{
		printf("Received SIGUSR1\n");
		printf("%d\n", pid_S);
		logFile(pid_S, (double)signo);
		printf("-%sPID: %d value:%s.\n", timeString, pid_S, signame[(int)signo]);
		printf("-%s%.3f.\n\n", timeString, newToken);
	}
	else if (signo == SIGCONT)
	{
		printf("Received SIGCONT\n");
		kill(pid_P, signo);
		kill(pid_G, signo);
	}
	else if (signo == SIGUSR2)
	{
		printf("Received SIGUSR2\n");
		kill(pid_P, SIGSTOP);
		kill(pid_G, SIGSTOP);
	}
}

int main(int argc, char *argv[])
{
	int res;
	char *argdata[4];  //process G data
	char *cmd = "./G"; //process G path
	char *port = argv[1];

	struct timeval tv;

	float msg;
	int o;

	argdata[0] = cmd;
	argdata[1] = port;
	argdata[2] = myfifo2;
	argdata[3] = NULL;

	/*-----------------------------------------Pipes Creation---------------------------------------*/

	if (mkfifo(myfifo1, S_IRUSR | S_IWUSR) != 0) //creo file pipe P|S
		perror("Cannot create fifo 1. Already existing?");

	if (mkfifo(myfifo2, S_IRUSR | S_IWUSR) != 0) //creo file pipe P|G
		perror("Cannot create fifo 2. Already existing?");

	if (mkfifo(myfifo3, S_IRUSR | S_IWUSR) != 0) //creo file pipe P|L
		perror("Cannot create fifo 3. Already existing?");

	int fd1 = open(myfifo1, O_RDWR); //apro la pipe1

	if (fd1 == 0)
	{
		error("Cannot open fifo 1");
		unlink(myfifo1);
		exit(1);
	}

	int fd2 = open(myfifo2, O_RDWR); //apro la pipe2

	if (fd2 == 0)
	{
		perror("Cannot open fifo 2");
		unlink(myfifo2);
		exit(1);
	}

	int fd3 = open(myfifo3, O_RDWR); //apro la pipe3

	if (fd3 == 0)
	{
		perror("Cannot open fifo 3");
		unlink(myfifo3);
		exit(1);
	}

	printf("\n");

	/*-------------------------------------------Process P--------------------------------------*/

	pid_P = fork();
	if (pid_P < 0)
	{
		perror("Fork P");
		return -1;
	}

	if (pid_P == 0)
	{
		printf("Hey I'm P and my PID is : %d.\n", getpid());

		float line;
		int m;
		float q;

		sleep(2);

		/*-------------------------------------Socket (client) initialization---------------------------*/
		int sockfd, portno;
		struct sockaddr_in serv_addr;
		struct hostent *server;
		portno = atoi(port);

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

		while (1)
		{
			//SELECT 1 : pipe with S
			int retval1, retval2, n;
			fd_set rfds1, rfds2; //come mi riferisco a un fd
			int fd;

			//SELECT 1
			FD_ZERO(&rfds1);
			FD_SET(fd1, &rfds1);

			tv.tv_sec = 2;
			tv.tv_usec = 0;
			retval1 = select(fd1 + 1, &rfds1, NULL, NULL, &tv);

			if (retval1 == -1)
				perror("select()");

			//SELECT 2
			FD_ZERO(&rfds2);
			FD_SET(fd2, &rfds2);

			tv.tv_sec = 2;
			tv.tv_usec = 0;
			retval2 = select(fd2 + 1, &rfds2, NULL, NULL, &tv);

			if (retval2 == -1)
				perror("select()");

			/////SWITCH
			switch (retval1 + retval2)
			{

			case 0:
				printf("no avaiable data\n");
				break;

			case 1:
				if (retval1 == 1)
				{
					n = read(fd1, &q, sizeof(q));
					printf("From S recivedMsg = %.3f \n", q);
					sleep((int)q);
				}
				else if (retval2 == 1)
				{
					n = read(fd2, &line, sizeof(line));
					//if (line < -1 || line > 1)
					//break;
					printf("From G recivedMsg = %.3f \n", line);

					m = write(fd3, &line, sizeof(line));
					if (m < 0)
						error("ERROR writing to L");

					line += 1;

					m = write(sockfd, &line, sizeof(line));
					if (m < 0)
						error("ERROR writing to socket");
				}
				break;

			case 2:
				n = read(fd1, &line, sizeof(line));
				printf("From S recivedMsg = %.3f \n", line);
				break;

			default:
				printf("Select error!!!!\n");
				break;
			}
		}

		wait(&res);

		close(fd1);
		unlink(myfifo1);

		close(fd2);
		unlink(myfifo2);

		close(fd3);
		unlink(myfifo3);
	}

	else
	{
		pid_L = fork();

		if (pid_L < 0)
		{
			perror("Fork L");
			return -1;
		}

		if (pid_L == 0)
		{
			printf("Hey I'm L and my PID is : %d.\n", getpid());
			while (1)
			{
				o = read(fd3, &msg, sizeof(msg));
				if (o < 0)
					error("ERROR reciving file to L");
				logFile(getpid(), msg);
			}
			close(fd3);
			unlink(myfifo3);
		}

		else
		{

			/*----------------------------------------------Process G---------------------------------------*/

			pid_G = fork(); // FORK 2
			if (pid_G < 0)
			{
				perror("Fork G");
				return -1;
			}

			if (pid_G == 0)
			{
				printf("Hey I'm G and my PID is : %d.\n", getpid());
				execvp(argdata[0], argdata);
				error("Exec fallita");
				return 0;
			}

			/*----------------------------------------------Process S---------------------------------------*/

			pid_S = getpid();

			printf("Hey I'm S and my PID is : %d.\n", getpid());

			if (signal(SIGUSR1, sig_handler) == SIG_ERR)
				printf("Can't catch SIGUSR1\n");

			if (signal(SIGCONT, sig_handler) == SIG_ERR)
				printf("Can't catch SIGCONT\n");

			if (signal(SIGUSR2, sig_handler) == SIG_ERR)
				printf("Can't catch SIGUSER2\n");

			float t = 1;
			int g;
			sleep(20);

			g = write(fd1, &t, sizeof(t));
			if (g < 0)
				error("ciao");
			sleep(10);

			while (1)
			{
			};

			return 0;
		}
	}
}
