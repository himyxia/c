#!/bin/bash

name=$1
cc -g -std=c99 -Wall $name mpc/mpc.c -ledit -lm -o lisp && gdb ./lisp
