CC = gcc
CFLAGS = -Wall -Wextra -Werror

TARGETS = a

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread

clean:
	rm -f $(TARGETS)
