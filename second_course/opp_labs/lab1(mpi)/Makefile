CC = mpicc
CFLAGS = -Wall -Werror -Wextra -O3 -lm
SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) -o $@ $< $(CFLAGS)

clean:
	rm -f $(TARGETS)