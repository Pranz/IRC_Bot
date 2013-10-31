CC=gcc
CFLAGS= -Wall -std=gnu99
LDFLAGS= 

LIBS=lolie
BINDIR=bin
OBJDIR=obj
SRCDIR=src
OUT=ircbot

vpath %.c $(SRCDIR)

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))

SOURCES=$(call rwildcard,./$(SRCDIR),*.c)
OBJECTS=$(SOURCES:./$(SRCDIR)/%.c=$(OBJDIR)/%.o)

OUTPREFIX=
OUTPOSTFIX=
OUTPUTFLAG=

ifneq (,$(findstring gcc,$(CC)))
	OUTPUTFLAG=-o
	CFLAGS+= -fno-gnu89-inline
	LDFLAGS+=$(LIBS:%=-l%)
endif

all: CFLAGS+= -O3
all: $(OUT)

debug: CFLAGS+= -g -ftrapv -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings -Wcast-qual -Wswitch-default -Wunreachable-code -Wfloat-equal -Wuninitialized -Wignored-qualifiers
debug: $(OUT)

$(OBJDIR)/%.o: %.c
	$(CC) -I$(SRCDIR) $(CFLAGS) $(OUTPUTFLAG) $@ -c $<

clean:
	rm -f $(OBJECTS) $(BINDIR)/$(OUTPREFIX)$(OUT)$(OUTPOSTFIX)

$(OUT): $(OBJECTS)
	$(CC) $(OUTPUTFLAG) $(BINDIR)/$(OUTPREFIX)$@$(OUTPOSTFIX) $(LDFLAGS) $^

run:
	./$(BINDIR)/$(OUTPREFIX)$(OUT)$(OUTPOSTFIX)
