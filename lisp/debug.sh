#!/bin/bash

name=$1
cc -g -std=c99 -Wall $name ../mpc/mpc.c -ledit -lm -o polish && gdb ./polish
