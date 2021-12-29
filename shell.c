#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>

#define BUFF 1024 // defines buffer size

void printDir(); // method prints directory as prompt
int formatArgs(char buf[], char* argv[]); // method formats arguments passed in shell

/**
 * Shell is a Unix Shell class that implements standard Bash shell utilities within a
 * custom class. The user is given a prompt and can use a plethora of commands including,
 * but not limited to, cd, cat, wc, head, tail, (etc.).
 */
int main() {

    char* argv[120]; // argument buffer
    int argc; // number of args
    char buf[BUFF]; // input buffer
    size_t b = sizeof(buf)/sizeof(buf[0]); // size of input buffer

    chdir(getenv("HOME")); // switches to home directory before shell initially prompts

    // Starts shell loop which prompts user for commands
    while (1) {

        printDir(); // prints prompt

        memset(buf, '\0', b); // resets input buffer
        int n;
        if ((n = read(STDIN_FILENO, buf, BUFF)) == -1) { // reads for user input
            perror("Cannot read input\n");
        } // if
        if (n < 2) { // if no args passed
            if (n < 1) { // if ctrl-D then exits
                printf("\n");
                exit(0);
            } // if
            continue;
        } // if

        argc = formatArgs(buf, argv); // formats inputs into arguments that can be executed

        if (strcmp(argv[0], "exit") == 0) { // if argument is exit
            exit(0);
        } // if

        if (strcmp(argv[0], "cd") == 0) { // if argument is cd
            if (argc == 1) {
                if (chdir(getenv("HOME")) == -1) { // cd to parent
                    perror("Error changing directory");
                } // if
            } else { // gets current directory in order to change
                char cwd2[100];
                char* gdir = getcwd(cwd2, sizeof(cwd2));
                char* dir = strcat(gdir, "/");
                char* full = strcat(dir, argv[1]);
                if (chdir(full) == -1) { // changes directory
                    perror("Error changing directory");
                } // if
            } // if
            continue;
        } // if

        pid_t pid;
        int status;

        pid = fork(); // creates child process
        if (pid == 0) { // child process

            for (int i = 0; i < argc - 1; i++) { // searches for "<", ">", or ">>"
                if (strcmp(argv[i], "<") == 0) { // if "<" is passed
                    argv[i] = NULL; // ignores "<" in cmd args
                    int fd0;
                    // opens file descriptor
                    if ((fd0 = open(argv[i+1], O_RDONLY)) < 0) {
                        perror("Couldn't open input file");
                        exit(0);
                    } // if
                    dup2(fd0, STDIN_FILENO); // redirects file descriptor to stdin
                    close(fd0); // closes file descriptor
                } else if (strcmp(argv[i], ">") == 0) { // if ">" is passed
                    argv[i] = NULL; // ignores ">" in cmd args
                    int fd1;
                    // opens file descriptor
                    if ((fd1 = open(argv[i+1], O_CREAT | O_WRONLY | O_TRUNC, 0666)) < 0) {
                        perror("Couldn't open the output file");
                        exit(0);
                    } // if
                    dup2(fd1, STDOUT_FILENO); // redirects stdout and writes to file descriptor
                    close(fd1); // closes file descriptor
                } else if (strcmp(argv[i], ">>") == 0) { // if ">>" is passed
                    argv[i] = NULL; // ignores ">>" in cmd args
                    int fd2;
                    // opens file descriptor
                    if ((fd2 = open(argv[i+1], O_CREAT | O_WRONLY | O_APPEND, 0666)) < 0) {
                        perror("Couldn't open the output file");
                        exit(0);
                    } // if
                    dup2(fd2, STDOUT_FILENO); // redirects stdout and appends to file descriptor
                    close(fd2); // closes file descriptor
                } // if
            } // for

            if (execvp(argv[0], argv) == -1) { // executes command args
                perror("Unknown command");
            } // if
            exit(EXIT_FAILURE); // should never reach here
        } else if (pid > 0) {
            while (!(wait(&status) == pid)); // parent waits for child to execute
        } else {
            perror("Error forking"); // error forking
            exit(EXIT_FAILURE);
        } // if
    } // while
    return 0;
} // main

/**
 * formatArgs takes an input buffer and formats into char*[]
 * that can later be executed.
 *
 * @param char buf[]  buffer array with inputs
 * @param char* argv[]  destination for formatted arguments
 * @return int  number arguments in argv
 */
int formatArgs(char buf[], char* argv[]) {
    int n = strlen(buf);
    if (buf[n - 1] == '\n') { // sets the last char to string terminating char
        buf[n - 1] = '\0';
    } // if

    // separates each input arg into char* to add to argv
    char *token;
    token = strtok(buf, " ");
    int argc = 0;

    while (token != NULL) {
        argv[argc] = token;
        token = strtok(NULL, " ");
        argc++; // increments arg count
    } // while
    argv[argc] = NULL; // sets last item of argv to NULL
    return argc; // returns number of args
} // formatArgs

/**
 * printDir prints the modified current working directory. Instead of
 * printing userid, "~" is printed instead.
 */
void printDir() {
    // gets current working directory
    char cwd[100];
    size_t s = sizeof(cwd)/sizeof(cwd[0]);
    memset(cwd, '\0', s);
    getcwd(cwd, s);
    int slsh = 0;
    int start = 0;
    for (int i = 0; i < s; i++) { // finds location of userid
        if (cwd[i] == '/') {
            slsh++;
            if (slsh == 4) {
                start = i;
            } // if
        } // if
    } // for
    setbuf(stdout, NULL);
    printf("1730sh:");
    if (slsh < 3) { // prints directory prior to userid
        setbuf(stdout, NULL);
        printf("%s$ ", cwd);
    } else if (slsh == 3) { // prints directory at userid with ~ instead
        setbuf(stdout, NULL);
        printf("~$ ");
    } else { // inserts ~ instead of userid along with following directories
        char subcwd[sizeof(cwd) - start];
        memcpy(subcwd, &cwd[start], sizeof(subcwd) - 1);
        subcwd[sizeof(subcwd) - 1] = '\0';
        setbuf(stdout, NULL);
        printf("~%s$ ", subcwd);
    } // if
} // printDir
