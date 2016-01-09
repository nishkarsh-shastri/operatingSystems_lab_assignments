This the ReadMe file for assignment 8 of operating system 

How to Run:

Compile page.c and run ./a.out
1) Give the order of the matrix i.e the value of n
2) Give the length d of the pages
3) Now the matrix of the assignment will be created automatically
4) Give the Algorithm you want to use as your option
5) Now give the smallest page table size you want to start with
6) Number of iterations which will increase the page table size with 1 and start fresh
7) All the results will be automatically saved in the files created in the same folder with name corresponding to each algorithm
8) Repeat step 4 till you run the same on all the algorithms


How to plot:
Requirements :: gnuplot
File to see -> gnuplotCommand

1) Use the gnuplotCommand file along with the gnuplot command prompt.
2) Copy and paste all the commands given in the gnuplot terminal.
3) 5 files will be created, 4 individually representing each plot and 1 representing the superimposed images of all the plots.



How the algorithms work:

FIFO:
Uses the sliding window of length m over the whole array and updates and replaces page in a queue wise fashion

LFU:
Uses two parameters to decide which page to replace, the first one is time of arrival and second one is frequecy of occurrence. The sliding window implementation is the same.

LRU:
Uses single parameter time of entry as the only parameter to decide which page to replace. It gets updated whenever a new page comes. The sliding window implementation is the same.

SCA:
Uses reference bits as the main parameter for deciding which page to replace.


