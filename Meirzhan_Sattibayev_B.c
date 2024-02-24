#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_FILES 100
#define STRING1 "CSCI332"
#define STRING2 "OS"

typedef struct {
    char **files;
    int start;
    int end;
    int index; // Index of threads for the sake of debugging
} ThreadArg;

pthread_mutex_t lock; // created for thread syncronization. Makes sure that only one thread is accessing 1 file.
volatile int found_string1 = 0; // used for "CSCI332" string, declared as volatile so that it can be changed by multiple threads
pthread_t *threads; // pointers to threads
int thread_count; 
char *directory = "my_files";

void *search_strings(void *arg) {
    ThreadArg *thread_arg = (ThreadArg *)arg;
    char line[1024];
    FILE *fp;

    for (int i = thread_arg->start; i < thread_arg->end && !found_string1; i++) { // loop iterates over a range of files from start to end specified by start and end && till "CSCI332" is found
        fp = fopen(thread_arg->files[i], "r");
        if (fp == NULL) {
            printf("Cannot open file: %s\n", thread_arg->files[i]);
            continue;
        }

        int line_num = 0;
        while (!found_string1 && fgets(line, sizeof(line), fp) != NULL) { // reading lines from files
            line_num++;
            if (strstr(line, STRING1) != NULL) { // if match is found, then in the next lines below,  mutex 'lock' is locked so that only one thread can change found_string1. 
                pthread_mutex_lock(&lock);
                if (!found_string1) {
                    found_string1 = 1; // found "CSCI332", so the flag is 1
                    printf("Thread %d found string %s in file %s at line %d\n", thread_arg->index, STRING1, thread_arg->files[i], line_num); // printing the thread index and the file name and line
		    
		    for(int i = 0; i < thread_count; i++) {
			if(i != thread_arg->index) {
			   pthread_cancel(threads[i]);  // cancelling threads when CSCI332 is found
			}
 		    }
                }
		// mutex is unlocked, file is closed, and returning from function happens (thread is being terminated)
                pthread_mutex_unlock(&lock); 
                fclose(fp); 
                return NULL;
            } else if (strstr(line, STRING2) != NULL) {	// otherwise search continues by the same principle explained above
                printf("Thread %d found string %s in file %s at line %d\n", thread_arg->index, STRING2, thread_arg->files[i], line_num);
            }
        }
        fclose(fp);
    }
    return NULL;
}

int main() {
    DIR *dir; // pointer to a directory stream
    struct dirent *ent; // entry in the directory stream 
    char *file_names[MAX_FILES]; // array for filenames from directory specified at the very top of this file
    int file_count = 0;

    if ((dir = opendir(directory)) != NULL) { // start of simple directory scanning 
        while ((ent = readdir(dir)) != NULL) {	
            if (ent->d_type == DT_REG) { // checking if each entry in directory is a regular file
                file_names[file_count] = malloc(strlen(directory) + strlen(ent->d_name) + 2); // memory allocation for storing the full path of the file (path length + filename + '\' + null terminator)
                sprintf(file_names[file_count], "%s/%s", directory, ent->d_name);
                file_count++;
            }
        }
        closedir(dir);
    } else { // directory could not be opened
        perror("Error!");
        return EXIT_FAILURE;
    }

    int cpu_cores = sysconf(_SC_NPROCESSORS_ONLN); // getting the number of available CPU cores
    threads = malloc(cpu_cores * sizeof(pthread_t)); // dynamically allocating memory for an array of pthread objects
    ThreadArg args[cpu_cores]; 
    pthread_mutex_init(&lock, NULL);
    thread_count = cpu_cores;

    // The logic of dividing the work evenly among threads
    // Each thread has 'total number of files / total number of CPU cores' files	
  
    int files_per_thread = file_count / cpu_cores;
    for (int i = 0; i < cpu_cores; i++) {
        args[i].files = file_names; // setting up file names, start and end indexes of files for each thread
        args[i].start = i * files_per_thread;
        args[i].end = (i + 1) * files_per_thread;
        args[i].index = i; // 
        if (i == cpu_cores - 1) args[i].end = file_count; // if file count isn't perfectly divisible, all the remaining files will go to the last thread
        pthread_create(&threads[i], NULL, search_strings, (void *)&args[i]); // creating a new thread that will execure the search_strings function
    }

    // Check the found_string1 flag and cancel all threads if the string is found
    for (int i = 0; i < cpu_cores; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);  //  destroying the mutex 
    free(threads); // freeing the dynamically allocated memory for threads

    for (int i = 0; i < file_count; i++) {
        free(file_names[i]); // freeing the allocated memory for the file names
    }

    return 0;
}
