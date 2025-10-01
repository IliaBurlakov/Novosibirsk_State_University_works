CC = gcc
CFLAGS = -Wall -Wextra -Werror

TARGETS = a b c d e f

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)
