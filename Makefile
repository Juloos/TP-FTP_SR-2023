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

ifdef MAX_SERVERS
	CPPFLAGS+=-DMAX_SERVERS=$(MAX_SERVERS)
endif

ifdef NB_PROC
	CPPFLAGS+=-DNB_PROC=$(NB_PROC)
endif

SRCS1 = $(wildcard $(SRCDIR)/*[^CLIENT^e].c)
OBJS1 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS1))

SRCS2 = $(wildcard $(SRCDIR)/[^FTP]*[^e].c)
OBJS2 = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS2))

SRCS_ESCLAVE = $(wildcard $(SRCDIR)/serveurs_esclaves.c $(SRCDIR)/csapp.c $(SRCDIR)/protocoles.c $(SRCDIR)/esclave.c)
OBJS_ESCLAVE = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_ESCLAVE))

PROGS = $(BINDIR)/FTP $(BINDIR)/CLIENT $(BINDIR)/esclave

all: $(PROGS)

$(BINDIR)/FTP: $(OBJS1)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(BINDIR)/CLIENT: $(OBJS2)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BINDIR)/esclave: $(OBJS_ESCLAVE)
	$(CC) -o $@ $(LDFLAGS) $^ $(LIBS)

ftp:
	@make > /dev/null
	$(BINDIR)/FTP

client:
	@make > /dev/null
	$(BINDIR)/CLIENT localhost

doc:
	doxygen Documentation/Doxyfile

clean :
	-@rm -r $(OBJDIR)/*.o $(SRCDIR)/*~ $(BINDIR)/* 2>/dev/null || true
	@echo All is removed