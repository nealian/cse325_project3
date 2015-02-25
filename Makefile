############################################################
# Project 3 Makefile
# by Ian Neal & Rob Kelly
#
# `make` to build Project 3 normally
# `make clean` to clean build files
# `make test` to run deployment tests
#
############################################################
CC = gcc
CFLAGS = -ggdb -Wall
VAL = valgrind --quiet --leak-check=yes --undef-value-errors=no --error-exitcode=1
OUT = p3
OBJECTS = queue.o
RM = rm -f

ECHO = echo -e
R_ALIGN = awk '{printf "%$(shell tput cols)s\n", $$1}'
FAIL_STRING = "\x1b[31;01m[FAILURE]\x1b[0m"
GOOD_STRING = "\x1b[32;01m[GOOD]\x1b[0m"

all: p3

p3: p3.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $(OUT) $^

queue.o: queue.c
	$(CC) $(CFLAGS) -c $<

test: p3
	$(VAL) ./$(OUT) || ($(ECHO) $(FAIL_STRING) | $(R_ALIGN); exit 1)
	@$(ECHO) $(GOOD_STRING) | $(R_ALIGN)

clean:
	$(RM) $(OUT) *.o *~

