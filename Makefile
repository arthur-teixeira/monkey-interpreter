CC = gcc
CFLAGS = -Wall -Werror -g
SRCDIR = ./src
BINDIR = ./bin
SRCEXT = c

SRCS := $(shell find ./src ! -name '*_test.c' -type f -name '*.c')

OBJS := $(patsubst $(SRCDIR)/%.$(SRCEXT),$(BINDIR)/%.o,$(SRCS))

BINNAME = program


all: $(BINDIR)/$(BINNAME)

$(BINDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/$(BINNAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@


clean:
	rm -rf $(BINDIR)/*
