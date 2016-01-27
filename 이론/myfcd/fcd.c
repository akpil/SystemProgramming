#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>

/*
 * 	get a resut array at path_result 
 *	list_size is maximum array size
 *	list_ptr is already taken list index
 * 	dirptr must be same with get_length's dirptr
 */
void find_dir(DIR* dirptr, const char* dir_name, char** path_result, int list_size, int* list_ptr);

/*
 *	get a maximum length of list and file path
 *	return value>0 means there is same name of normal file else is there is no nomal file that name is match with dir_name
 *	max_length means maximum length of file path character
 * 	list_size is total length of file path list
 */
int get_length(DIR* dirptr, const char* dir_name, int* max_length, int* list_size);
int main(void)
{
	FILE* fp;
	DIR* dirptr;
	struct dirent* dirtable;
	char** total_path;
	char* buff;
	char dir_name[255];
	int i, selecter = 0;
	int max_length = 0, list_size = 0, flag, ptr = 0;
	
	printf("Enter the name of a directory : ");
	
	memset(dir_name, 0, 255);
	fscanf(stdin, "%[^\n]s\n", dir_name);
	
	dirptr = opendir(".");
	flag = get_length(dirptr,(const char*) dir_name, &max_length, &list_size);
	if(list_size)
	{
		total_path = (char**) malloc(sizeof(char*[list_size]));
		buff = (char*) malloc(sizeof(char[max_length+3]));
		memset(buff, 0, max_length+3);
		strcpy(buff, "cd ");
		for(i = 0; i < list_size; i++)
		{
			total_path[i] = (char*) malloc(sizeof(char[max_length]));
			memset(total_path[i], 0, max_length);
		}
		rewinddir(dirptr);
		find_dir(dirptr, (const char*) dir_name, total_path, list_size, &ptr);
		if(list_size == 1)
		{
			strcat(buff, total_path[0]);
			strcat(total_path[0], ">.");
			printf("%s\n", total_path[0]);
		}
		else
		{
			for(i = 0; i < list_size; i++)
			{
				printf("[%d]\t%s\n",i+1, total_path[i]);
			}
			printf("which one do you want?");
			fscanf(stdin, "%d", &selecter);
			strcat(buff, total_path[selecter-1]);
			strcat(total_path[selecter-1], ">.");
			printf("%s\n", total_path[selecter-1]);
		}
		fp = popen("bash", "w");
		fputs(buff, fp);
//		pclose(fp);
		free(buff);
		free(total_path);	
		closedir(dirptr);
	}
	else
	{
		if(flag)
		{
			printf("Not a directory\n");
		}
		else
		{
			printf("No such file or directory\n");
		}
	}	
	return 0;
}

int get_length(DIR* dirptr, const char* dir_name,int* max_length, int* list_size)
{
	struct dirent* dirtable;
	struct stat cur_stat;
	DIR* next_dirptr;
	char* cwd,* buf;
	int stat_checker = 0, temp_length = 0, flag = 0;

	cwd = getcwd(NULL,0);
	while(dirtable = readdir(dirptr))
	{
		temp_length = strlen(cwd) + 1;
		if(dirtable->d_ino != 0)
		{
			if(strcmp(dirtable->d_name,".") == 0 || strcmp(dirtable->d_name, "..")==0){}//skip . and ..
			else
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
					if(strcmp(dir_name, dirtable->d_name)==0)
					{
						if(*max_length < temp_length)
						{
							*max_length = temp_length+3;
						}
						*list_size += 1 ;
					}

					next_dirptr = opendir(dirtable->d_name);
					chdir(buf);

					flag += get_length(next_dirptr, dir_name, max_length, list_size);

					chdir(cwd);
					closedir(next_dirptr);
				}
				else
				{
					if(strcmp(dir_name, dirtable->d_name)==0)
					{
						flag += 1;
					}
				}
				free(buf);
			}
		}
	}
	return flag;
}

void find_dir(DIR* dirptr, const char* dir_name, char** path_result, int list_size, int* list_ptr)
{
	struct dirent* dirtable;
	struct stat cur_stat;
	DIR* next_dirptr;
	char* cwd, * buf;
	int i, stat_checker, temp_length;

	cwd = getcwd(NULL, 0);

	//check current path until find dir
	while(dirtable = readdir(dirptr))
	{
		temp_length = strlen(cwd) + 1;
		if(dirtable->d_ino != 0)
		{
			if(strcmp(dirtable->d_name,".") == 0 || strcmp(dirtable->d_name, "..")==0){}//skip . and ..
			else
			{
				temp_length += strlen(dirtable->d_name);
				buf = (char*) malloc(sizeof(char[temp_length]));
				memset(buf, 0, temp_length);
				strcpy(buf,cwd);
				strcat(buf,"/");
				strcat(buf, dirtable->d_name);
				
				if((stat_checker = stat(buf, &cur_stat))==-1)
				{
					perror("stat error");
					exit(1);
				}
				
				
				if(S_ISDIR(cur_stat.st_mode))
				{
					if(dir_name, strcmp(dirtable->d_name, dir_name)==0)
					{
						strcpy(path_result[*list_ptr], buf);
						*list_ptr += 1;
					}
					next_dirptr = opendir(dirtable -> d_name);
					chdir(buf);
					find_dir(next_dirptr,(const char*) dir_name, path_result, list_size, list_ptr);
					chdir(cwd);
				//	closedir(next_dirptr);
				}
				//free(buf);
			}
		}
	}
}		
