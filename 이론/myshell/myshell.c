#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>

#define buffer_size 1024
#define cmdblock_size 128
#define cmd_size 32
#define history_size 20
#define argv_max 20
#define fmask 0666

void make_argv(char* cmdblock, char** cmd);
void deleteblank(char *cmdblock);
void show_cmdactive(char *cwd);
void execution_cmd(char *cmd[argv_max], char * cwd, char history[history_size][buffer_size], int history_index);
void pipe_execution(int pipe_count, char * cmdblock, char * cwd, char history[history_size][buffer_size], int history_index);

int main()
{
	char buffer[buffer_size], cmdblock[cmdblock_size], remain_block[cmdblock_size], *redir_cmd;
	char *cwd;
	char history[history_size][buffer_size];
	int  flag = 0, i, j, k, count = 0, history_index = 0, pipe_count=0, fd;

	memset(buffer, 0, buffer_size);
	while(1)
	{
		cwd = getcwd(NULL, 0);//getenv("PWD");
		show_cmdactive(cwd);
		//read(STDIN_FILENO, buffer, buffer_size);
		fgets(buffer, buffer_size, stdin);
		// input history history length is 20
		if(buffer[strlen(buffer)-1] =='\n')
			buffer[strlen(buffer)-1] = '\0';
		strcpy(history[history_index%20], buffer);
		history_index ++;
		if(history_index >= history_size*2)
			history_index -= history_size;
		count = 0;	
		// count ; at the input buffer
		for(i = 0; i < strlen(buffer)+1; i ++)
		{
			if(buffer[i] == ';')
			{
				count ++;
			}
		}

		memset(remain_block, 0, cmdblock_size);
		for(i = 0; i <= count; i ++)
		{
			memset(cmdblock, 0, cmdblock_size);
			/*
			for(j = 0; j < argv_max; j ++)
			{
				memset(cmd[i], 0, cmd_size);
			}
			*/
			if(i == 0 && count > 0)
			{
				strcpy(cmdblock, strtok(buffer, ";"));
				if(buffer[strlen(buffer)+2] == '\0')
				{}
				else
				{
					strcpy(remain_block, strtok(NULL, ";"));
					for(j = 1; j < count; j++)
					{
						strcat(remain_block, ";");
						strcat(remain_block, strtok(NULL, ";"));
					}
				}
			}
			else if(count > 0)
			{
				if(remain_block[0] != '\0')
				{
					if(strchr(remain_block, ';'))
					{
						strcpy(cmdblock, strtok(remain_block, ";"));
						strcpy(remain_block, strtok(NULL, ";"));
						for(j = i+1; j < count; j++)
						{
							strcat(remain_block, ";");
							strcat(remain_block, strtok(NULL, ";"));
						}
					}
					else
					{
						strcpy(cmdblock, strtok(remain_block, ";"));
						memset(remain_block, 0, cmdblock_size);
					}
				}
			}
			else
			{
				strcpy(cmdblock, buffer);
			}

			// delete ' ' before the command
			deleteblank(cmdblock);

			if (fork() == 0)
			{
				pipe_count = 0;
				// cut by ' ' to make **argv
				for(j = 0; j < strlen(cmdblock)+1; j++)
				{
					if(cmdblock[j] == '|')
					{
						pipe_count ++;
					}
				}
				
				//redirection
				if((redir_cmd = strstr(cmdblock, ">>")) != NULL)
				{
					*redir_cmd = '\0';
					redir_cmd += 2;
					
					deleteblank(redir_cmd);
					if((fd = open(redir_cmd, O_WRONLY)) < 0)
					{
						if((fd = creat(redir_cmd, fmask)) < 0)
						{
							perror("file open/creation error");
							exit(1);
						}
					}
					else
						lseek(fd, (off_t)0, SEEK_END);
					close(STDOUT_FILENO);
					dup(fd);
					close(fd);
					pipe_execution(pipe_count, cmdblock, cwd, history, history_index);
				}
				else if((redir_cmd = strstr(cmdblock, ">!")) != NULL)
				{
					*redir_cmd = '\0';
					redir_cmd += 2;

					deleteblank(redir_cmd);
					if((fd = open(redir_cmd, O_WRONLY|O_TRUNC)) < 0)
					{
						perror("file open error");
						exit(1);
					}
					close(STDOUT_FILENO);
					dup(fd);
					close(fd);
					pipe_execution(pipe_count, cmdblock, cwd, history, history_index);
				}
				else if((redir_cmd = strchr(cmdblock, '>')) != NULL)
				{
					*redir_cmd = '\0';
					redir_cmd++;

					deleteblank(redir_cmd);
					if((fd = open(redir_cmd, O_WRONLY)) < 0)
					{
						if((fd = creat(redir_cmd, fmask)) < 0)
						{
							perror("file creation error");
							exit(1);
						}
						close(STDOUT_FILENO);
						dup(fd);
						close(fd);
						pipe_execution(pipe_count, cmdblock, cwd, history, history_index);
					}
					else
					{
						printf("file already exist\n");
					}
				}
				else if((redir_cmd = strchr(cmdblock, '<')))
				{
					*redir_cmd = '\0';
					redir_cmd++;

					deleteblank(redir_cmd);
					if((fd = open(redir_cmd, O_RDONLY)) < 0)
					{
						perror("file open error");
						exit(1);
					}
					close(STDIN_FILENO);
					dup(fd);
					close(fd);
					pipe_execution(pipe_count, cmdblock, cwd, history, history_index);
				}
				else
				{
					pipe_execution(pipe_count, cmdblock, cwd, history, history_index);	
				}
			}
			else
			{
				wait(NULL);
			}
		}
		memset(buffer, 0, buffer_size);
	}
}
void pipe_execution(int pipe_count, char * cmdblock, char * cwd, char history[history_size][buffer_size], int history_index)
{
	int i, pipe_fd[2], flag = 0, pid;
	char *cmd[argv_max], *pipe_cmd;
	
	if(pipe_count == 0)
	{
		for(i = 0; i < strlen(cmdblock); i ++)
		{
			if(cmdblock[i] == '&')
			{
				cmdblock[i] = '\0';
				flag = 1;
				break;
			}
		}
		make_argv(cmdblock, cmd);
		if(flag == 1)
		{
			if((pid = fork())==0)
			{
				execution_cmd(cmd, cwd, history, history_index);
			}
			else if(pid < 0)
			{
				perror("fork error");
				exit(1);
			}
			else
				exit(0);
		}
		else
			execution_cmd(cmd, cwd, history, history_index);	
	}
	else
	{
		pipe_cmd = &cmdblock[strlen(cmdblock)];
		while(1)
		{
			if(*pipe_cmd == '|')
			{
				*pipe_cmd = '\0';
				pipe_cmd++;
				pipe_count --;
				break;
			}
			else if(pipe_cmd == cmdblock)
				break;
			pipe_cmd--;
		}
		pipe(pipe_fd);
		if(fork() == 0)
		{
			close(STDOUT_FILENO);
			dup(pipe_fd[1]);
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			pipe_execution(pipe_count, cmdblock, cwd, history, history_index);
		}
		else
		{
			close(STDIN_FILENO);
			dup(pipe_fd[0]);
			close(pipe_fd[0]);
			close(pipe_fd[1]);
			deleteblank(pipe_cmd);
			make_argv(pipe_cmd, cmd);
			execution_cmd(cmd, cwd, history, history_index);	
		}
	}
}
void execution_cmd(char *cmd[argv_max], char * cwd, char history[history_size][buffer_size], int history_index)
{
	int j, k;
	char filename[cmdblock_size] = {'\0'}, *cd_temp, *quot;
	struct stat stat_for_cd;

	if(strcmp(cmd[0], "history")==0)
	{
		if(cmd[1] == '\0')
		{
			if(history_index >= history_size)
				j = history_index%history_size;
			else
				j = 0;
			k = j;
			for(; j < history_index; j++)
			{
				printf("%d: %s\n", j - k, history[j%history_size]);
			}
		}
		else
		{
			if(history_index >= history_size)
				j = (history_index-atoi(cmd[1]))%history_size;
			else
				j = history_index-atoi(cmd[1]);
			k = j;
			for(; j < history_index%history_size; j++)
			{
				printf("%d: %s\n", j - k, history[j%history_size]);
			}
		}
		exit(0);
	}
	else if(strcmp(cmd[0], "cd") == 0)
	{
		if(strcmp(cmd[1], "..") == 0)
		{
			cd_temp = &cwd[strlen(cwd)];
			while(1)
			{
				if(*cd_temp == '/')
				{
					*cd_temp = '\0';
					break;
				}
				else
				{
					cd_temp -= 1;
				}
			}
			chdir(cwd);
		}
		else
		{
			if(stat(cmd[1], &stat_for_cd) == -1)
			{
				perror("stat error");
				exit(1);
			}

			if(S_ISDIR(stat_for_cd.st_mode))
			{
				strcpy(filename, cwd);
				strcat(filename, "/");
				strcat(filename, cmd[1]);
				chdir(filename);
			}
			else
			{
				perror("is not a directory");
			}
		}
	}
	else if(cmd[0] != '\0')
	{
		while((quot = strchr(cmd[1],'"')) != NULL)
		{
			while(*quot != '\n')
			{
				*quot = *(quot+1);
				quot++;
			}
		}
		while((quot = strchr(cmd[1],'\'')) != NULL)
		{
			while(*quot != '\n')
			{
				*quot = *(quot+1);
				quot++;
			}
		}
	
		if(execvp(cmd[0], cmd)<0)
		{
			perror("execvp not a correct");
		}
	}
	else
	{
		perror("command has an error");
		exit(0);
	}
}

void make_argv(char* cmdblock, char** cmd)
{
	int j = 0, k = 0;
	while(1)
	{
		if(j == 0)
		{
			cmd[k] = cmdblock;
			k ++;
		}
		else if(cmdblock[j] == '\0')
		{
			cmd[k] = '\0';
			break;
		}
		else
		{
			if(cmdblock[j] == ' ')
			{
				cmdblock[j] = '\0';
				if(cmdblock[j+1] == '\0')
					cmd[k] = '\0';
				else if(cmdblock[j+1] == ' ')
				{
					j++;
					continue;
				}
				else
					cmd[k] = &cmdblock[j+1];
				k++;
			}
		}
		j++;	
	}
}

void deleteblank(char *cmdblock)
{
	int j;
	while(1)
	{
		if(cmdblock[0] == ' ')
		{
			for(j = 1; j < strlen(cmdblock); j ++)
			{
				cmdblock[j-1] = cmdblock[j];
			}
			cmdblock[strlen(cmdblock)-1] = '\0';
		}
		else
		{
			break;
		}
	}
}

void show_cmdactive(char *cwd)
{
	printf("[myshell] ");
	printf("%s", cwd);
	printf(" >");
}
