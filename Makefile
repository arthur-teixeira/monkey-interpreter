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
		$(CC) $(CFLAGS) $(filter-out ./bin/main.o, $(OBJS)) $$testfile -o $(BINDIR)/$$testname; \
		echo "Running $$testname..."; \
		if $(BINDIR)/$$testname; then \
			echo "$$testname PASSED"; \
		else \
			echo "$$testname FAILED"; \
			exit 1; \
		fi; \
	done

clean:
	rm -rf $(BINDIR)/*

run: clean all test

.PHONY: all test clean run
