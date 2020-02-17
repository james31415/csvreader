#!/usr/bin/env sh

SOURCES=main.c
PROGRAM=reader

mkdir -p ../bin
gcc -Wall -Og -DMAIN -o ../bin/$PROGRAM $SOURCES
