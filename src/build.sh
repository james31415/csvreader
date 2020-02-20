#!/usr/bin/env sh

SOURCES=posix_reader.c
PROGRAM=reader

mkdir -p ../bin
gcc -Wall -Og -DMAIN -o ../bin/$PROGRAM $SOURCES
