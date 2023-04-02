.PHONY: all, clean

# Disable implicit rules
.SUFFIXES:

BINDIR=Bin
SRCDIR=Sources
OBJDIR=Objects
HEADDIR=Headers

# create directories if they don't exist
$(shell mkdir -p $(OBJDIR) $(BINDIR) .client .server)

# Keep intermediate files
#.PRECIOUS: %.o

CC = gcc
CFLAGS = -Wall -Werror -g
CPPFLAGS = -IHeaders
# Note: -lnsl does not seem to work on Mac OS but will
# probably be necessary on Solaris for linking network-related functions
#LIBS += -lsocket -lnsl -lrt
LIBS += -lpthread

ifdef DEBUG
	CPPFLAGS+=-DDEBUG
endif

SRCS1 = $(wildcard $(SRCDIR)/*[^CLIENT^1^2^3].c)
OBJS1 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS1))

SRCS2 = $(wildcard $(SRCDIR)/[^FTP]*[^1^2^3].c)
OBJS2 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS2))

SRCS_ESCLAVE1 = $(wildcard $(SRCDIR)/serveurs_esclaves.c $(SRCDIR)/csapp.c $(SRCDIR)/protocoles.c $(SRCDIR)/esclave1.c)
OBJS_ESCLAVE1 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_ESCLAVE1))

SRCS_ESCLAVE2 = $(wildcard $(SRCDIR)/serveurs_esclaves.c $(SRCDIR)/csapp.c $(SRCDIR)/protocoles.c $(SRCDIR)/esclave2.c)
OBJS_ESCLAVE2 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_ESCLAVE2))

SRCS_ESCLAVE3 = $(wildcard $(SRCDIR)/serveurs_esclaves.c $(SRCDIR)/csapp.c $(SRCDIR)/protocoles.c $(SRCDIR)/esclave3.c)
OBJS_ESCLAVE3 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_ESCLAVE3))

PROGS = $(BINDIR)/FTP $(BINDIR)/CLIENT $(BINDIR)/esclave1 $(BINDIR)/esclave2 $(BINDIR)/esclave3

all: $(PROGS)

$(BINDIR)/FTP: $(OBJS1)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(BINDIR)/CLIENT: $(OBJS2)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

ftp:
	@make > /dev/null
	$(BINDIR)/FTP

client:
	@make > /dev/null
	$(BINDIR)/CLIENT localhost

$(BINDIR)/esclave1: $(OBJS_ESCLAVE1)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(BINDIR)/esclave2: $(OBJS_ESCLAVE2)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(BINDIR)/esclave3: $(OBJS_ESCLAVE3)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

clean :
	-@rm -r $(OBJDIR)/*.o $(SRCDIR)/*~ $(BINDIR)/* 2>/dev/null || true
	@echo All is removed