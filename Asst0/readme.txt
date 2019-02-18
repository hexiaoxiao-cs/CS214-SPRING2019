Readme:
Usage: ./pointersorter "<inputstring>"
1.	This program utilizes the qsort alogrithm in the C standard library with customized comparison function to meet the requirement of the problem.
2.	This program utilizes a dynamic memory allocation with exponent memory growth in order to reduce memory allocation and copy process and reduce the running time.
3.	This program will check if a dynamic memory allocation failed for 100 times, which can be infered that either there are something wrong with the memory or the target computer is running out of memory. Then, the program will show a wrong message and exit automatically so that nothing terrible bad will happen.