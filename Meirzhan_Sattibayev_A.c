#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024  // maximum length for the input string
#define MAX_NUM_CMDS 15        // maximum number of commands that can be input
#define MAX_ARGS_PER_CMD 10    // maximum number of arguments for each command


//

// function to execute a single command
void execute_single_cmd(char *single_cmd) {
    char *cmd_args[MAX_ARGS_PER_CMD]; // array to hold the command and its arguments
    char *token; // for tokenizing the command string
    int arg_counter = 0; // counter for the number of arguments

    // tookenizing the command string into executable name and arguments
    token = strtok(single_cmd, " ");
    while (token != NULL && arg_counter < MAX_ARGS_PER_CMD) {
        cmd_args[arg_counter++] = token;
        token = strtok(NULL, " ");
    }
    cmd_args[arg_counter] = NULL; // NULL-terminate the arguments list

    // execuing the command using execvp
    execvp(cmd_args[0], cmd_args);

    // if execvp returns then it means there was an error
    perror("Execution failed");
    exit(EXIT_FAILURE);
}

int main() {
    char user_input[MAX_INPUT_LENGTH]; // user input string
    char *individual_cmds[MAX_NUM_CMDS]; // array to hold individual commands
    pid_t child_pids[MAX_NUM_CMDS]; //array to hold child process IDs
    int cmd_index, wait_status; //variables for loop and status checking

    while (1) {
        printf("Enter commands separated by commas: ");
        fflush(stdout); // Ensure prompt is displayed immediately

        //reading input from user
        if(fgets(user_input, MAX_INPUT_LENGTH, stdin) == NULL) {
            printf("Error reading input.\n");
            continue; // Continue to next iteration if input reading fails
        }

        user_input[strcspn(user_input, "\n")] = 0; // // Remove the newline character at the end of the input


        // spliting the input into individual commands
        cmd_index = 0;
        individual_cmds[cmd_index] = strtok(user_input, ",");
        while (individual_cmds[cmd_index] != NULL && cmd_index < MAX_NUM_CMDS) {
            individual_cmds[++cmd_index] = strtok(NULL, ",");
        }

        // executing each command in a separate child process
        for (int j = 0; j < cmd_index; j++) {
            if ((child_pids[j] = fork()) == 0) {
                // in each child process, we need to execute the command
                execute_single_cmd(individual_cmds[j]);
            } else if (child_pids[j] < 0) {
                // if fork fails then we need to display error and exit
                perror("Child process was not created");
                exit(EXIT_FAILURE);
            }
        }

        // here main process waits for all child processes to finish
        int error_detected = 0; // Flag for detecting errors in child processes
        for (int j = 0; j < cmd_index; j++) {
            waitpid(child_pids[j], &wait_status, 0);
            // Check if child exited with an error
            if (!WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0) {
                error_detected = 1;
            }
        }

        // in case if there is an error
        if (error_detected) {
            printf("One or more commands failed to execute correctly.\n");
        }
    }

    return 0;
}
