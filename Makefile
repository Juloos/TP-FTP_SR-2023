.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror
CPPFLAGS = -IHeaders
# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread

INCLUDE = csapp.h
OBJS = csapp.o

PROGS = FTP CLIENT

all: $(PROGS)

#FTP: FTP.o $(OBJS)
#	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

#CLIENT: CLIENT.o $(OBJS)
#	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%: %.o $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)
#	$(CC) -o $@ $(LDFLAGS) $(LIBS) $^

clean:
	rm -f $(PROGS) *.o