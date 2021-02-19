Name: Isobel Glover
Class: CSC 360 Spring 2021, Operating Systems Concepts
Assignment: Assignment 1

SHELL 360 README


Files:
sh360.c (source code)
.sh360rc (Path/prompt file)

Usage:
to compile sh360.c:
 % gcc -Wall -o sh360 sh360.c
to start the interactive shell:
 % ./sh360


Introduction:

SHELL 360 (sh360) is an interactive shell that supports executable commands contained in directories listed in the file .sh360rc, output redirection of a command into a file, and piping of 2 or 3 commands. sh360 also supports the built-in commands cd and exit.


Path/prompt file:

The file .sh360rc must be contained in the running directory of sh360. The first line of .sh360rc will be used as the command line prompt and all subsequent lines in .sh360rc will be used as directories (paths) in which to search for executable commands. A maximum of 10 directories in .sh360rc is supported, and directories must be at most 100 characters in length.



Features and Functionality (assigment spec requirements):
 a) SHELL 360 repeatedly prompts the user for commands and executes these commands in a child process.
 b) SHELL 360 executes commands with at most 10 tokens (commands, arguments, and keywords such as OR, PP and -> are all considered tokens) and prints an error message to stdout when a command cannot be found in any directory listed in .sh360rc. SHELL 360 terminates when 'exit' is entered at the prompt as the sole command (with no arguments).
 c) SHELL 360 supports output redirection by using the keyword 'OR' and the special symbol '->' with the following syntax:
 % OR [cmd] -> [filename]
where [cmd] is a command and its list of arguments, and [filename] is the file in which to write the output of [cmd]. (% here represents the command prompt and is not part of the command syntax.)
If this syntax is not followed, an error message will be output to stderr.
 d) SHELL 360 supports piping by using the keyword 'PP' and the special symbol '->' with the following syntax:
 % PP [cmd1] -> \[cmd2] (-> [cmd3])
where [cmd1], [cmd2] and [cmd3] are commands and their arguments, and the second pipe and third command (in brackets) are optional. (Piping is supported for a minimum of 2 and a maximum of 3 commands.)
If this syntax is not followed, an error message will be output to stderr.
     
Additional Features:

 - SHELL 360 supports the cd command (expected usage). An error message is printed to stderr if the cd fails (ie, the directory given is invalid), or if too many arguments are given to the command (cd only supports a single argument).
 - 

Errors:

sh360.c defines custom error codes for each possible error. For each error, a custom error message is printed to stderr before prompting for the next command.
The following situations will result in an error:
 - input line too long
 - too many arguments in the input line
 - a command not found in any path in .sh360rc
 - invalid syntax for an OR command
 - invalid syntax for a PP command
 - unable to open output file (given in an OR command) for writing
