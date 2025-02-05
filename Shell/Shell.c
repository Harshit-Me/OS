#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <unistd.h>
#include <sys/wait.h>
#include<sys/types.h>
#include<pwd.h>

#define BUFFSIZE 128// Buffer size. Set as 128. Not expecting too many commands more than 128 in size.
#define DELIM "\t\n\r\a"// Delimiter for the rest of the commands.
#define DELIM1 " \"\t\n\r\a"// Delimiter to be used first as the command will have commas and to be used for cd command
//Declaring global Variables
char* prev1;// Most recent command
char* prev2; // Second most recent command
char* prev3; // Third most recent command
char* prev4; // Fourth most recent command
char* prev5; //Fifth most recent command
char cwd[BUFFSIZE];// The homedirectory
typedef enum {F, T} boolean;// Separate other commands from cd command. Different Delim to be used.
char *global_command;// Command to be passed in case the given command is not cd or history as they are handled differently
// declaring the functions to be used
void loop();// Intializing the shell and keeping process active
char *getCommand(); // Getting users input (Command)
char **Parse(char *cmd);// (Parsing the command into tokens)
int execute(char **args); //( Executing the commands)
int changeDirectory(char **args); //(Change the current dirrectory)
int history(char **args); //(Give the history of last 5 commands input by the user)
int Launch_Forkable_Command(char **args);// (Launching commands like ls, cat,wc etc which can be directly implemented from execvp)
char *getCommand()// Get theinpyt command from the user
{
    char *cmd = malloc(sizeof(char)*BUFFSIZE);// Allocating memory
    int cur_pos = 0;// Current position in which new character is to be filled in cmd
    int c = 0; // Placeholder to get the character
    int cur_size = BUFFSIZE; // Current maximum alloted memory size to cmd
    int global_size = BUFFSIZE;
    int global_pos = 0;
    global_command = malloc(sizeof(char)*BUFFSIZE);
    while (1)
    {
        c = getchar();// Get the characeter from the user
        if (c =='\n'||c == EOF)// Reached end of file or endline character
        {
            cmd[cur_pos] = '\0';// Fill the last position as null. Works like a flag.
            global_command[global_pos] = '\0';
            return cmd;// Return the command
        }
        else if (c=='\"')
        {
            global_command[global_pos++]=c;// Inverted commas pass only to global command and not to cmd
            if(global_size == global_pos)// Memory full
            {
                global_size+=BUFFSIZE;// Increase maximum path size
                global_command = realloc(global_command, global_size);// reallocate
            }
            continue;
        }
        else
        {
            cmd[cur_pos] = c;// Fill the next available position with the character.
            global_command[global_pos++] = c;
        }
        cur_pos++;// Increment the current available position
        if (cur_size == cur_pos)// Memory Full
        {
            cur_size+=BUFFSIZE;// Increase maximum size
            cmd = realloc(cmd,cur_size);// Reallocate memory
        }
        if(global_size == global_pos)// Memory full
        {
            global_size+=BUFFSIZE;// increase  size
            global_command = realloc(global_command, global_size);// reallocate memory
        }
    }
    return cmd;
}
char **Parse(char *cmd)
{
    if (cmd== NULL)// In any odd chance that cmd is NULL
    {
        return NULL;
    }
    
    char **args = malloc(sizeof(char*)*BUFFSIZE);// Place holder for tokens of the parsed command
    char *token = strtok(cmd,DELIM1);// First token
    int cur_pos = 0;// Current available position in tokens
    int cur_size = BUFFSIZE;// Maximum alloted size to tokens.
    // A standard way of tokenizing the string in C
    while (token!=NULL)
    {
        args[cur_pos] = token;// Fill the available position with token got from prev iteration
        token = strtok(NULL, DELIM);// Use of different set of delimmitor in case the command is cd to handle space case
        cur_pos++;// Increment the current available position
        if (cur_size == cur_pos)// Memory Full
        {
            cur_size+=BUFFSIZE;// Increase the maximum size
            args = realloc(args, cur_size); // Reallocating the memory
        }
    }
    args[cur_pos]=NULL;// Fill the last position by NULL ptr.
    return args;
}
char *substring(const char* str, int beg, int n)// Give the substring of str from beg to n. Equivalent to str[beg:n+1] in python
{
   char *ret = malloc(n); // Allocate memory of size n characters
   strncpy(ret, (str + beg), n);// Copy the string from beg to n in ret
   return ret;    // Return the string
}
int my_str_check(char *sup_s,char *sub_s)//Check if the current path has home directory as its subpath
{
    /*
        @ This function return 0 if sub_s == sup_s or sup_s[0:len(sub_s)] == sub_s
    */
    int a = strlen(sub_s);//Size of sub-string
    int b = strlen(sup_s);//size of super-string
    if (a>b)// Sub string is larger in size;
    {
        return -1;
    }
    else if (a==b)// String are of equal size
    {
        return strcmp(sup_s,sub_s);// Return 0 if the strings are equal
    }
    else
    {
        char *xqc = substring(sup_s,0,a);// Path length equal to the length of home dir is extracted.
        return strcmp(xqc,sub_s);// If the returned path length is same as home dir then the given path has home dir as its subpath
    }
}
void loop()
{
    // Set all the ptrs to prev commands to NULL initially
    prev1 = NULL;
    prev2 = NULL;
    prev3 = NULL;
    prev4 = NULL;
    prev5 = NULL;
    char *line;// Command from the user
    char **args;// Tokens of the tokenized string
    int status;
    if (getcwd(cwd,sizeof(cwd))==NULL)// Current working directory. Stores home directory
    {
        perror("getcwd() has failed");// The get cwd command  has failed probably due to insufficient size in cwd 
    }
    uid_t uid = getuid();  // Get user ID
    struct passwd *pw = getpwuid(uid);  // Get user info

    if (!pw) {
        printf("Failed to get user name");
        return;
    }
    printf("%s:~$",pw->pw_name);
    while (1)
    {
        line = getCommand();// Get command from use.
        char *cp;// Copy of the command. Used for storing in prev ptrs. So that parsing does of line does not affect what is stored in those ptrs
        if (line[0]!='\0')
        {
            cp = strdup(line);// Copy the line. Str tok will modify our line. We need to store a copy
        }
        else
        {
            cp = NULL;// Empty line wa inserted.
        }
        args = Parse(line);// parsing and splitting in tokens
        status = execute(args);// Executing the command
        if (!status)// Command has failed. This will never execute as we are never returning 0
        {
            perror("Command Not Valid\n");
        }
        else
        {
            char wd[BUFFSIZE];// get the current working directory
            if (getcwd(wd,sizeof(wd))==NULL)
            {
                perror("getcwd() has failed");
            }
            else if (strcmp(cwd,wd)==0)// Current working directory is home directory
            {
                printf("%s:~$",pw->pw_name);
            }
            else if (my_str_check(wd,cwd)==0)
            {
                printf("%s:~%s$",pw->pw_name,substring(wd,strlen(cwd), strlen(wd)-strlen(cwd)));
            }
            else
            {
                printf("%s:~%s$",pw->pw_name,wd);// Current working directory is not home directory
            }
        }
        if (cp!=NULL)// Command input by user is not empty line
        {
            // Setting the various pointers. Self understood flow.
            if(prev4!=NULL)
            {
                prev5 = strdup(prev4);
            }
            if (prev3!=NULL)
            {
                prev4 = strdup(prev3);
            }

            if (prev2!=NULL)
            {
                prev3 = strdup(prev2);
            }
            if (prev1!=NULL)
            {
                prev2 = strdup(prev1);
            }
            if (cp!=NULL)
            {
                prev1 = strdup(cp);
            }
        }
        free(line);// Free the memory
        free(args);// Free the memory
        free(global_command);//Free the memory
    }
}
int execute(char **args)// Execute the command
{
    if (args[0] == NULL)// Empty Line
    {
        return 1;
    }
    if (strcmp(args[0],"history")==0)// History Command
    {
        history(args);
    }
    else if (strcmp(args[0],"cd")==0)// cd Command
    {
        changeDirectory(args);
    }
    else{
        Launch_Forkable_Command(args);// Any other command that can be execited using execvp and fork
    }
    return 1;

}
int history(char **args)// History command
{
    // Print all those ptrs which are non-null;
    if (prev5!=NULL)
    {
        printf("   %s \n", prev5);
    }
    if (prev4!=NULL)
    {
        printf("   %s\n",prev4);
    }
    if (prev3!=NULL)
    {
        printf("   %s\n",prev3);
    }
    if (prev2!=NULL)
    {
        printf("   %s\n",prev2);
    }
    if (prev1!=NULL)
    {
        printf("   %s\n",prev1);
    }
    return 1;
}
int changeDirectory(char **args)// Cd command
{
    if (args[1]==NULL)// cd is not follwd by path
    {
        perror("cd command must be followed with path \n");
    }
    else if (strcmp(args[1],"~")==0)
    {
        chdir(cwd);// Get to the home directory
    }
    else
    {

        if (chdir(args[1])!=0)// Directory change and returns 0 if no such directory is found.
        {
            perror("No directory found");
        }
    }
    return 1;
    
}
int Launch_Forkable_Command(char **args)//Other command execution
{
    int i=0;
    boolean inside  = F;
    for(i = 0;i<strlen(global_command);i++)
    {// In this forloop we replace spaces outside inverted commas with a delimiter (endline character in our case) and spaces inside the inverted commas are not altered
    // Inverted commas are relpaced by a delimiter(endline character in our case).
    // This ensures proper tokenization for commands like mkdir, cat, echo etc.
        if(inside == T)
        {
            // Inside inverted commas
            if(global_command[i]=='\"')// Replace all inverted commas by endline character.
            {
                inside = F;// Reached the other inverted commas. So we will now get outside the inverted commas.
                global_command[i]='\n';
            }
            continue;
        }
        else
        {
            if(global_command[i]=='\"')// Inverted comma spotted/
            {
                inside = T;
                //In the next iteration we will be inside inverted commas/
                global_command[i] = '\n';
            }
            if(global_command[i]==' ')// Replace space by a delimiter(endline character in our case.) in case we are outside inverted commas.
            {
                global_command[i]='\n';
            }
        }
    }
    free(args);
    args = Parse(global_command);

    pid_t pid;// process id
    int status;
    pid = fork();// fork process
    if (pid == 0)
    {
        //Child process
        if (execvp(args[0],args)==-1)

        {
            perror("Invalid argument");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid<0)
    {
        // Fork has failed
        perror("Failed to fork a new process");
    }
    else{
        //Parent process
        do
        {
            /* code */
          waitpid(pid, &status, WUNTRACED);// Wait for child to complete.
        } while (!WIFEXITED(status)&&!WIFSIGNALED(status));
    
    }
    return 1;
}
int main(int argv, char **argc)
{
    loop();
    return EXIT_SUCCESS;
}