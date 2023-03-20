CC = gcc
# flags as required by the gcc
C_FLAGS = -I /workspaces/atom-ce/atom-ce

# binary
BINARY = atom-ce

# object files
BIN = bin
BUILD = bin/build

# src paths
STRING_C = src/string/string.c

# rules
atom-ce: main.c string
	${CC} ${C_FLAGS} -o ${BIN}/${BINARY} main.c ${BUILD}/string.o

# string rule
string: ${STRING_C}
	${CC} -c ${C_FLAGS} -o ${BUILD}/string.o ${STRING_C}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
