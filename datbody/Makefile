CC=clang
#CC=musl-gcc
CFLAGS=-static -g -Wall
CCS=clang
CSFLAGS=-g

all: datbody listv 

datbody: datbody.c datbody.conf.h arg.h
	$(CC) $(CFLAGS) datbody.c -o datbody

listv: listv.c listv.conf.h
	$(CCS) $(CSFLAGS) listv.c -o listv -lncurses

