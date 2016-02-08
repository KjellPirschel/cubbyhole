PROG = server_parallel
COMP = gcc
LIBS = -pthread
CFLAGS = -g $(LIBS) -D_GNU_SOURCE

.PHONY: $(PROG)

all: $(PROG)

$(PROG):
	$(COMP) $(CFLAGS) -o $(PROG) $(PROG).c

clean:
	rm $(PROG)
