#!/bin/bash

mpicc gameoflife.c -o gameoflife.out
mpirun -np 2 ./gameoflife.out
