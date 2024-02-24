"# OS-Assignment" 
1) C program that emulates the behavior of a terminal. The program prompts the user to enter a few commands (console programs) separated by a comma and then execute those commands concurrently. Each command runs on a separate child. Each command may be followed by an argument (e.g., ls -lh).

2) C program that creates a number of threads equal to the number of CPU cores in the system in order to accelerate the search of the strings “CSCI332” and “OS” in a set of text files in the directory “my_files”. If a thread finds the first string, it cancels all the other threads and print out the file name and the line number where the string was found. All threads print out the file name and the line number where the second string was found until they are canceled or terminated.
