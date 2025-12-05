TARGET_1 = 3_1
SRCS_1 = 3_1.c

CC=gcc
RM=rm
CFLAGS= -g -Wall -Werror -Wextra
LIBS=-lpthread
INCLUDE_DIR="."

all: ${TARGET_1}

${TARGET_1}: ${SRCS_1}
	${CC} ${CFLAGS} ${SRCS_1} ${LIBS} -o ${TARGET_1}
clean:
	${RM} -f *.o ${TARGET_1}