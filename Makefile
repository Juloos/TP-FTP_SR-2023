.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

SRCDIR=Sources
OBJDIR=Objects
HEADDIR=Headers
SERVDIR=Server
CLIENTDIR=Client

# create directories if they don't exist
$(shell mkdir -p $(OBJDIR) $(SERVDIR) $(CLIENTDIR))

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror
CPPFLAGS = -IHeaders
# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread

SRCS1 = $(wildcard $(SRCDIR)/*[^CLIENT].c)
OBJS1 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS1))

SRCS2 = $(wildcard $(SRCDIR)/[^FTP]*.c)
OBJS2 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS2))

PROGS = $(SERVDIR)/FTP $(CLIENTDIR)/CLIENT

all: $(PROGS)

$(SERVDIR)/FTP: $(OBJS1)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(CLIENTDIR)/CLIENT: $(OBJS2)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean :
	-@rm -r $(OBJDIR)/*.o $(SRCDIR)/*~ $(SERVDIR)/* $(CLIENTDIR)/* 2>/dev/null || true
	@echo All is removed