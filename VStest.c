/******************************************************************************************************************************
 * This is the second part of your shell assignment. At a minimum you should add to your previous shell the following features:
Backgrounding using the ampersand (&)
Input file redirection using <
Output file redirection, both truncating (>) and appending (>>)
*******************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define NOWAIT 1
#define IN_REDIR 2
#define OUT_REDIR_APPEND 4
#define OUT_REDIR_TRUNCATE 8

int main()
{
    char dirname[100], cmd[256], args[128], *infile, *outfile, *argv[32];
    unsigned int flags = 0;
    int i, chpid, fd, termchild;
    mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;

    printf("Ctrl-Z to exit...\n");
    while(1)
    {
        memset(args, 0, 256);
        flags = 0;
        getcwd(dirname, 100); /*find current working directory*/
        printf("\nVS:%s>", dirname); /*prompt the user to enter a command while showing them the cwd*/
        fgets(cmd, 256, stdin);
        cmd[strlen(cmd)-1] = '\0';
        if(cmd == NULL)
            break;
        else
        {
            /*check if there are any control tokens and seperate the argument string*/
            for(i = 0; i < (int)strlen(cmd); i++)
            {
                if(cmd[i] == '>' || cmd[i] == '<' || cmd[i] == '&')
                {
                    /*strncpy(args, cmd, i);*/
                    if(args[i-1] == ' ')
                        args[i-1] = '\0';
                    break;
                }
                else
                {
                    args[i] = cmd[i];
                }
            }
            /*continue through the user command to find all control tokens and apply flags as needed*/
            argv[0] = strtok(args, " \t");
            for(i = 1; i < 32; i++)
            {
                argv[i] = strtok(NULL, " ");
                if(argv[i] == NULL)
                    break;
            }
            for(i = (i-1); i < (int)strlen(cmd); i++)
            {
                if(cmd[i] =='>' && cmd[i+1] == '>')
                {
                    flags = flags | OUT_REDIR_APPEND;
                    outfile = strtok(cmd+i+2, " <>&\t\n");
                }
                if(cmd[i] == '>')
                {
                    flags = flags | OUT_REDIR_TRUNCATE;
                    outfile = strtok(cmd+i+1, " <>&\t\n");
                }
                if(cmd[i] == '<')
                {
                    flags = flags | IN_REDIR;
                    infile = strtok(cmd+i+1, " <>&\t\n");
                }
                if(cmd[i] == '&')
                    flags = flags | NOWAIT;
            }
            if(strcmp(argv[0], "cd") == 0) /*built in command for change directories*/
                chdir(argv[1]);
            else
            {
                chpid = fork();
                if(chpid == 0) /*child process*/
                {
                    /*check flags and redirect input/output as prescribed*/
                    if((flags & IN_REDIR) == IN_REDIR)
                    {
                        fd = open(infile, O_RDONLY);
                        close(0);
                        dup2(fd, 0);
                        close(fd);
                    }
                    if((flags & OUT_REDIR_APPEND) == OUT_REDIR_APPEND)
                    {
                        fd = open(outfile, O_WRONLY | O_APPEND | O_CREAT, mode);
                        close(1);
                        dup2(fd, 1);
                        close(fd);
                    }
                    if((flags & OUT_REDIR_TRUNCATE) == OUT_REDIR_TRUNCATE)
                    {
                        fd = open(outfile, O_WRONLY | O_CREAT, mode);
                        close(1);
                        dup2(fd, 1);
                        close(fd);
                    }
                    execvp(argv[0], argv);
                    perror("exec");
                }
                else if((flags & NOWAIT) != NOWAIT)
                do
                {
                    termchild = wait(NULL);
                } 
                while(termchild != chpid);
                else
                    printf("pid %d\t\t%s", chpid, argv[0]);
            } 
        }
    }
    return 0;
}