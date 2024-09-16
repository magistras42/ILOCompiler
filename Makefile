# Makefile - check compiler for dif targets
# make some functions defs, inlining, etc

BINARY          = 412fe
OBJS     		= scanparse.o 412fe.o
NAME			= mns10
DIST            = lab1

CC              = gcc
CFLAGS			= -std=gnu11 -O2
LIBS            =
LDFLAGS         =
INCLUDES		=

build:      $(OBJS)
		$(CC) $(LDFLAGS) -o $(BINARY) $(OBJS) $(LIBS)

.c.o:
		$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
		rm -rf core* $(DIST)*.tar $(OBJS) $(BINARY) *~ 

dist:		clean
		cd ..; \
		tar cfvJ $(DIST)/$(NAME).tar \
			$(DIST)/*
