#include<stdio.h>
#include<string.h>
#include<stdlib.h>			// exit()
#include<unistd.h>			// fork(), getpid(), exec()
#include<sys/wait.h>		// wait()
#include<signal.h>			// signal()
#include<fcntl.h>			// close(), open()
char** parseInput(char* input_line)
{
	//This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	char* ch;
	char** buffer=malloc(sizeof(char*)*1024);        //memory allocation
	if(!buffer)
	{
		printf("Error!!----Allocation Error\n");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	for(i=0;(ch=strsep(&input_line," "))!=NULL;i++)   
	{
		buffer[i]=ch;
	}
	buffer[i]=NULL;
	return buffer;
}
void executeCommand(char** command)
{
	//This function will fork a new process to execute a command 
	if(command[0]==NULL || strcmp(command[0],"")==0) //for null or empty command
		return;
	else if(strcmp(command[0],"cd")==0)
	{
		chdir(command[1]); // chdir changes directory
		return;
	}
	else
	{
		int x=fork();
  		if(x>0) // for parent process
  		{
			wait(NULL); //parent process waits for child process
  		}
  		else if(x==0) // If Child process is created
  		{
    		if(execvp(command[0],command)<0)
    		{
      			printf("Shell: Incorrect command\n");
				exit(1);
    		}
  		}
  		else  // If error in forking
  		{
    		printf("ERROR!!----forking child error");
    	}
    	return;
	}
}

void executeParallelCommands(char** command)
{
	//This function will run multiple commands in parallel
	int i=0,j=0;
	/*if(command[0]==NULL)
		return;
	if(strcmp(command[0],"")==0)	
		return;*/
	for(i=0;command[i]!=NULL;i++)
	{
		while(command[i]!=NULL && strcmp(command[i],"&&")!=0)
		{
			i++;
		}
		command[i]=NULL;
		int x=fork();
		if(x<0) // If error in forking
		{
			exit(1);
		}
		else if(x==0) // If child process is created
		{
			if(execvp(command[j],&command[j])<0)
			{
				printf("Shell: Incorrect command\n");
				exit(1);
			}
		}
		else{
			wait(NULL); //parent process waits for child processes 
		}
		j=i+1;
	}
	return ;
}

void executeSequentialCommands(char** command)
{
	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
	int i=0,j=0;
	/*if(command[0]==NULL)
		return;
	if(strcmp(command[0],"") == 0)	
		return ;*/
	for(i=0;command[i]!=NULL;i++)
	{
		while(command[i]!=NULL && strcmp(command[i],"##") != 0)
		{
			i++;
		}
		command[i]=NULL;
		executeCommand(&command[j]);// after this executes, another command runs
		j = i+1;
	}

}
void executeCommandRedirection(char** command)
{
	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
	int i=0;
  /*if(command[0] == NULL)
		return;
	if(strcmp(command[0],"") == 0)	
		return ;*/
	while(command[i]!=NULL && strcmp(command[i],">")!=0)
	{
		i++;
	}
	command[i]=NULL;
	int x=fork();
	if(x==0)		// If child process is created
	{
		close(STDOUT_FILENO);
		open(command[i+1], O_CREAT | O_WRONLY | O_APPEND);
		if(execvp(command[0],command)<0) // in case of wrong command
		{
			printf("Shell: Incorrect command\n");
			exit(1);
		}
	}
	else if(x>0)		// parent process wait for child
	{
		wait(NULL);
	}
	else        // error in forking
	{
		exit(1);
	}
}
int main()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	long unsigned int size = 10;
	char* line;
	line = (char *)malloc(size);
	if(!line)
	{
		printf("Allocation error\n");
		exit(EXIT_FAILURE);
	}
	while(1)	// loop exits on exit command
	{
		char path[1024];
		getcwd(path,sizeof(path));
		printf("%s$",path); // printing the PATH
		int i=0;
		getline(&line,&size,stdin);
		while(line[i]!='\0')
		{
			if(line[i]==EOF || line[i]=='\n')
			{
				line[i]='\0';
			}
			i++;
		}
		char** command;
		command = parseInput(line);
		if(strcmp(command[0],"exit")==0)	// on exit command
		{
			printf("Exiting shell...\n");
			break;
		}
		i=0;
		char choice ='\0';
		for(i=0;command[i] != NULL;i++)
		{
			if(strcmp(command[i],"&&") == 0)
			{
				choice='A';
				break;
			}
			else if(strcmp(command[i],"##") == 0)
			{
				choice='B';
				break;
			}
			else if(strcmp(command[i],">") == 0)
			{
				choice='C';
				break;
			}
		}
		if(choice=='A')
		{
			executeParallelCommands(command);
		}	// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(choice=='B')
		{
			executeSequentialCommands(command);
		}	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(choice=='C')
		{
			executeCommandRedirection(command);
		}	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
		{
			executeCommand(command);
		}	// This function is invoked when user wants to run a single commands++

	}
	return 0;
}

