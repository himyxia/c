#!/bin/bash

# cc -std=c99 -Wall hello_world.c -o hello_world
# -std=c99, flag to tell the compiler which version or standard of C we are programming with. 
# This lets the compiler ensure our code is standardised, so that people with different operating systems or compilers will be able to use our code.

name=$1
#fileName="$name.c"
#cc -std=c99 -Wall $fileName -o $name
cc -std=c99 -Wall $name ../mpc/mpc.c -ledit -lm -o polish && ./polish
