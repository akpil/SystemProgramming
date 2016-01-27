#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>

void simple_ftw(DIR* dirptr);
int main(int argc, char** argv)
{
	DIR * dirptr;
	char* cwd, * start_wd;
	int start_wd_length = 0;
	
	if(argc == 1)
		dirptr = opendir(".");
	else
	{
		cwd = getcwd(NULL, 0);
		start_wd_length = strlen(cwd) + strlen(argv[1]) + 2;
		start_wd = (char*) malloc(sizeof(char[start_wd_length]));
		memset(start_wd, 0, start_wd_length);
		strcpy(start_wd, cwd);
		strcat(start_wd, "/");
		strcat(start_wd, argv[1]);
		dirptr = opendir(argv[1]);
		chdir(start_wd);
	}
	if(dirptr == NULL)
	{
		perror("directory error\n");
		exit(1);
	}
	simple_ftw(dirptr);
	return 0;
}
void simple_ftw(DIR* dirptr)
{
        struct dirent* dirtable;
        struct stat cur_stat;
        DIR* next_dirptr;
        char* cwd,* buf;
        int stat_checker = 0, temp_length = 0;

        cwd = getcwd(NULL,0);
	
	printf("===%s===(start)\n",cwd);
        while(dirtable = readdir(dirptr))
        {
                temp_length = strlen(cwd) + 1;
                if(dirtable->d_ino != 0)
                {


			temp_length += strlen(dirtable->d_name);
			buf = (char*) malloc(sizeof(char[temp_length]));
			strcpy(buf, cwd);
			strcpy(buf, "/");
			strcpy(buf, dirtable->d_name);

			if((stat_checker = stat(buf, &cur_stat))==-1)
			{
				perror("stat error");
				exit(1);
			}


			if(S_ISDIR(cur_stat.st_mode))
			{
				printf("%-30s*\t0\%3o\n", dirtable->d_name, (unsigned int) cur_stat.st_ino);
				if(strcmp(dirtable->d_name,".") == 0 || strcmp(dirtable->d_name, "..")==0){}//skip . and ..
				else
				{
					next_dirptr = opendir(dirtable->d_name);
					chdir(buf);

					simple_ftw(next_dirptr);

					chdir(cwd);
					closedir(next_dirptr);
				}

			}
			else
			{
				printf("%-30s\t0\%3o\n", dirtable->d_name, (unsigned int) cur_stat.st_ino);
			}
		}
	}
	printf("===%s===(end)\n",cwd);
}
