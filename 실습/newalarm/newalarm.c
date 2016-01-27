#include <unistd.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h>

void myalarm() 
{ 
	printf("ding dong dang\n"); 
} 

void newalarm(int sec)
{
	pid_t pid, ppid;
	if(pid = fork(), pid > 0)
	{}
	else if(pid == 0)
	{
		ppid = getppid();
		sleep(sec);
		kill(ppid,SIGALRM);
		exit(0);
	}
	else
	{
		printf("fork error!\n");
		exit(1);
	}
}

int main() 
{ 	         
	int i=0; 
	pid_t real = getpid();
	printf("alarm setting\n");
 	signal(SIGALRM, myalarm);
  	while(i<5) 
	{ 
		printf("ok\n");
		newalarm(2); 
		pause();
		i++;		
	}
}	  
