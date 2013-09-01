#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<dirent.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include <wait.h>
struct proc_list{
	int pid;
	int is_running;
	char name[100];
};
int proc_count=0;
int background_process;
char *home;
struct proc_list process[100];

//-----------------------------------------------------------------------------------------------------------------------

void print_promt()
{
	char *pwd,*host,*user;
	pwd=(char *)malloc(sizeof(char)*100);
	host=(char *)malloc(sizeof(char)*100);
	user=(char *)malloc(sizeof(char)*100);
	if(getlogin_r(user,100))
	{
		printf("Get login method failed. Exiting\n");
		exit(1);
	}
	if(gethostname(host,100))
	{
		printf("Get host name method failed. Exiting\n");
		exit(1);
	}
	if(getcwd(pwd,100)==NULL)
	{
		printf("Get current working directory method failed. Exiting\n");
		exit(1);
	}
	int min_len;
	char*temp;
	int len_home=strlen(home);
	temp=strdup(pwd);
	if(strncmp(pwd,home,len_home)==0)
	{
		pwd=strcpy(pwd,"~");
		pwd=strcat(pwd,&temp[len_home]);
	}
	fflush(stdout);
	printf("<%s@%s:%s>",user,host,pwd);
	fflush(stdout);
	return;

}

//-------------------------------------------------------------------------------------------------------------------------------

void sigchld_handler_background(int sig){
	pid_t pid;
	int status;
	pid = waitpid(-1,&status,WNOHANG);
	int i;
	for(i=0;process[i].pid!=pid && i<proc_count;i++);
	if(i<proc_count){
		process[i].is_running=0;
		fflush(stdout);
		if(WIFEXITED(status)!=0)
			printf("\n%s %d exited normally\n",process[i].name,process[i].pid);
		fflush(stdout);
		print_promt();
		fflush(stdout);
	}
	return;
}

//-----------------------------------------------------------------------------------------------------------------

void  execute(char **argv,int x)
{
	pid_t  pid;
	int  status;
	//	signal(SIGCHLD,sigchld_handler_background);
	pid=fork();
	if (pid < 0)  
	{    
		printf("ERROR: forking child process failed\n");
		exit(1);
	}   
	else if (pid == 0)  
	{    
		if (execvp(*argv, argv) < 0)  
		{    
			printf("%s: command not found\n",argv[0]);
			fflush(stdout);
			_exit(0);
		}   
	}   
	else 
	{    
		process[proc_count].pid=pid;
		process[proc_count].is_running=1;
		strcpy(process[proc_count].name,argv[0]);
		proc_count++;
		if(background_process==0){
			signal(SIGCHLD,NULL);
			wait(NULL);
			signal(SIGCHLD,sigchld_handler_background);
			process[proc_count-1].is_running=0;
		}
		else
			printf("command %s pid %d\n",process[proc_count-1].name,process[proc_count-1].pid);
	}   
	return;
}

//-------------------------------------------------------------------------------------------------------------

void get_Path(char *pwd,char *home)
{
	int min_len;
	char*temp;
	int len_home=strlen(home);
	temp=strdup(pwd);
	if(strncmp(pwd,home,len_home)==0)
	{
		pwd=strcpy(pwd,"~");
		pwd=strcat(pwd,&temp[len_home]);
	}
}

//--------------------------------------------------------------------------------------------------------------

void get_history(char **str,int n,char *str_temp)
{
	int i=0,len,arg,temp;
	len=strlen(str_temp);
	if(strncmp("hist",str_temp,4)==0)
	{
		if(len==4)
		{
			for(i=1;i<=n;i++)
				printf("%d. %s\n",i,str[i]);
		}
		else
		{
			arg=0;
			for(i=4;i<len;i++)
			{
				temp=str_temp[i]-'0';
				fflush(stdout);
				if(temp>=0&&temp<=9)
					arg=arg*10+temp;
				else
				{
					printf("Invalid argument to hist\n");
					return;
				}
			}
			fflush(stdout);
			if(arg>n)
			{
				for(i=1;i<=n;i++)
				{
					printf("%d. %s\n",i,str[i]);
					fflush(stdout);
				}
			}
			else
			{
				int count=1;
				for(i=n-arg+1;i<=n;i++)
				{
					printf("%d. %s\n",count,str[i]);
					fflush(stdout);
					count++;
				}
			}
		}
	}
	return;
}

//------------------------------------------------------------------------------------------------------------------------------

void sig_handler(int signum){
	if(signum == 2 || signum == 20){
		fflush(stdout);
		printf("\n");
		print_promt();
	}   
	signal(SIGINT, sig_handler);
	signal(SIGTSTP, sig_handler);
	return;
}

//-----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
	signal(SIGINT,  sig_handler);
	signal(SIGTSTP, sig_handler);

	home=(char *)malloc(sizeof(char)*100);
	if(getcwd(home,100)==NULL)
	{
		printf("Get current working directory method failed. Exiting\n");
		exit(1);
	}
	signal(SIGCHLD,sigchld_handler_background);
	char **str;
	str=(char **)malloc(sizeof(char*)*100);
	str[0]="\0";
	int n=0;
	int flag=0;
	while(1)
	{
		background_process=0;
		char *str_tmp,**cmd,*token,*path,*host,*user,*temp_c;
		str_tmp=(char *)malloc(sizeof(char)*1024);
		cmd=(char **)malloc(sizeof(char*)*100);
		int x=0,i=0;
		print_promt();
		fflush(stdout);
		if (flag==0)
			gets(str_tmp);
		else
		{
			flag=0;
			printf("\n");
			strcpy(str_tmp,temp_c);
		}

		fflush(stdin);
		if(strlen(str_tmp)==0)
			continue;
		if(strcmp(str[n],str_tmp)!=0)
		{
			n++;
			str[n]=strdup(str_tmp);
		}

		token = strtok(str_tmp," \t");
		while(token != NULL)
		{
			cmd[x]=token;
			token = strtok(NULL," \t");
			x++;
		}
		if(strcmp(cmd[x-1],"&")==0)
		{
			background_process=1;
			cmd[x-1]=NULL;

		}
		if(strcmp(cmd[0]," ")==0)
			continue;
		if(strcmp(cmd[0],"quit")==0)
			exit(0);
		cmd[x]=NULL;
		if(strcmp("cd",cmd[0])==0)
		{
			if(x==1)
			{
				if(chdir((const char*)home))
					printf("cd failed to execute\n");
			}
			else
			{
				if(chdir((const char*)cmd[1]))
					printf("Invaid path or argument to cd\n");

			}
			fflush(stdout);

		}
		else if(strncmp("hist",cmd[0],4)==0)
		{
			if(x==1)
				get_history(str,n,cmd[0]);
			else
				printf("Invalid arguments to hist\n");
		}
		else if (strncmp("!hist",cmd[0],5)==0)
		{
			int arg=0;
			int temp,len=strlen(cmd[0]);
			for(i=5;i<len;i++)
			{   
				temp=cmd[0][i]-'0';
				if(temp>=0&&temp<=9)
					arg=arg*10+temp;
				else
				{   
					printf("Invalid arguments to hist\n");
					fflush(stdout);
					return;
				}   
			}   
			if(arg>n)
				printf("argument exceded no. of commands in history\n");
			else
			{
				flag=1;
				temp_c=strdup(str[arg]);
			}
			fflush(stdout);
		}
		else if(strcmp(cmd[0],"pid")==0)
		{
			if(x==1)
			{
				printf("command name: %s process id: %d\n",cmd[0],getpid());
			}	
			else{
				if(strcmp(cmd[1],"all")==0)
				{
					printf("List of all process spawned by the shell\n");
					for(i=0;i<proc_count;i++){
						printf("command name: %s process id: %d\n",process[i].name,process[i].pid);
					}
				}
				else if(strcmp(cmd[1],"current")==0)
				{
					printf("List of currently executing processes spawned by the shell\n");
					for(i=0;i<proc_count;i++){
						if(process[i].is_running)
						{
							printf("command name: %s process id: %d\n",process[i].name,process[i].pid);
						}
					}
				}
				else
				{
					printf("Invaid argument to pid\n");
				}
			}
			fflush(stdout);
		}
		else
		{
			execute(cmd,x);
			fflush(stdout);
		}
		free(cmd);
		free(str_tmp);
	}
	return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------
