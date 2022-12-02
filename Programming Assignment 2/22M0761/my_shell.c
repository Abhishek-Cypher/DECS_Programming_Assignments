#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/*Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
    char **tokens = (char **) malloc(MAX_NUM_TOKENS* sizeof(char*));
    char *token = (char*) malloc(MAX_TOKEN_SIZE* sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < strlen(line); i++)
    {
        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t')
        {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0)
            {
                tokens[tokenNo] = (char*) malloc(MAX_TOKEN_SIZE* sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0;
            }
        }
        else
        {
            token[tokenIndex++] = readChar;
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

int fg_pid = 50;

void shell_ctrlc_handler(int signalno)
{
	kill(fg_pid,SIGTERM);
}


int main(int argc, char *argv[])
{
    signal(SIGINT,shell_ctrlc_handler); 
    char line[MAX_INPUT_SIZE];
    char **tokens;
    int i;
 
    while (1)
    {	
        // ``````````````````````````````` "watch ps -au" TO MONITOR `````````````````````````````````````    
        int stat;
        while (1)
        {
            if (waitpid(-1, &stat, WNOHANG) > 0)    // RETURNS PID IF ZOMBIE FOUND, ELSE DOES NOT WAIT
            {
                printf("Shell: Background process finished\n");
                continue;    // TO REAP MULTIPLE ZOMBIES
            }
            break;    // IF NO ZOMBIE FOUND
        }

        /*BEGIN: TAKING INPUT */
        bzero(line, sizeof(line));
        printf("$ ");
        scanf("%[^\n]", line);
        getchar();
        
        //        printf("Command entered: %s (remove this debug output later)\n", line);
        /*END: TAKING INPUT */

        line[strlen(line)] = '\n';    //terminate with new line
        tokens = tokenize(line);

        // ```````````````` MY CODE ````````````````

        if (tokens[0] == NULL)
            continue;
        
        // ``````````````````````````````` exit PART ````````````````````````````````````
        if (strcmp(tokens[0], "exit") == 0)
        {
            kill(0,SIGTERM);        // IF SIGKILL USED, CLEAN UP NOT ALLOWED, AND KILLED ABRUPTLY
            break;
        }
        
        // ```````````````````````````` TO FIND THE LAST TOKEN, FOR CHECKING IF IT IS & ````````````````````````````````
        int k = 0, flag = 0;
        
        while (tokens[k] != NULL)
        {
            k++;
        }
        k--;
        
        // `````````````````````````` cd PART ````````````````````````````````
        if (strcmp(tokens[0], "cd") == 0)
        {
	    if(k>1)
	    {
	    	printf("cd WAS GIVEN MORE THAN 1 DIRECTORY AS ARGUMENT\n");
	    	continue;
	    }	
            int err = chdir(tokens[1]);
            if (err == -1)
            {
                printf("Shell : Incorrect command\n");
            }
        }
        else
        {
            // `````````````````````````` FORK AND OTHER FUNCTIONS PART ````````````````````````````````````
            if (strcmp(tokens[k], "&") == 0)
            {
                flag = 1;        // IF BG PROCESS
                tokens[k] = NULL;
            }
            
            int fr = fork();
            if (fr == 0)
            {	
            	if(flag == 0)		// D PART , PUTTING ALL FOREGROUND PROCESSES INTO GROUPS OF THEIR OWN, SO THAT NOT EFFECTED BY PARENT
            	{
            		setpgid(0,0);
            	}
            	
            	if(flag == 1)
            	{
            		signal(SIGINT, SIG_IGN);
            	}
            		
                execvp(tokens[0], tokens);
                printf("THIS COMMAND DOES NOT EXIST\n");
                exit(-1);
            }
            else if (fr > 0)
            {
                if (flag != 1)		// SO, PARENT WAITS FOR FOREGROUND
                {	
                    fg_pid = fr;
                    waitpid(fr, NULL, 0);
                }
            }
            else
            {
                printf("UNABLE TO FORK");
            }
        }

        // ```````````````` MY CODE ENDS ````````````````

        //do whatever you want with the commands, here we just print them

        //        for(i=0;tokens[i]!=NULL;i++){
        //            printf("found token %s (remove this debug output later)\n", tokens[i]);
        //        }

        // Freeing the allocated memory    
        for (i = 0; tokens[i] != NULL; i++)
        {
            free(tokens[i]);
        }
        free(tokens);
    }
    return 0;
}
