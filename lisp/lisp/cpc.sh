#!/bin/bash

name=$1
cc -std=c99 -Wall $name mpc/mpc.c -ledit -lm -o lisp && ./lisp
