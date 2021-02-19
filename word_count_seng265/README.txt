Assignments for SENG 265 (Software Development Methods)

Application that uses dynamic memory and linked lists to count the frequency of words of each length in a provided text document and print the associated words in sorted order.
Optionally, the --sort parameter allows printing the word length lists in descending order of frequency.

Usage:
$ gcc -Wall -std=c99 -o word_count word_count.c
$ ./word_count --sort --infile <input_file>
where the sort parameter is optional and <input_file> is a text document.

