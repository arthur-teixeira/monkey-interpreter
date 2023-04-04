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

TESTFILES := $(shell find ./src -name '*_test.c' -type f)

test:
	@echo "Building tests..."
	@for testfile in $(TESTFILES); do \
		testname=$$(basename $$testfile .c); \
		echo "Building $$testname..."; \
		$(CC) $(CFLAGS) $$testfile -o $(BINDIR)/$$testname; \
		echo "Running $$(testname)..."; \
		./$$testname; \
	done

clean:
	rm -rf $(BINDIR)/*

.PHONY: all test clean
