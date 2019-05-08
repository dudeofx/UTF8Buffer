#!/bin/bash

gcc -Wall  -Werror -o UTF8Buffer.o -c UTF8Buffer.c

gcc -Wall -Werror -o test01.o -c test01.c
gcc -Wall -Werror -o test01 UTF8Buffer.o test01.o

gcc -Wall -Werror -o example01.o -c example01.c
gcc -Wall -Werror -o example01 UTF8Buffer.o example01.o 

gcc -Wall -Werror -o example02.o -c example02.c
gcc -Wall -Werror -o example02 UTF8Buffer.o example02.o 

