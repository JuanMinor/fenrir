CC 			= gcc
# flags as required by the gcc
C_FLAGS 	= -I /workspaces/atom-ce/atom-ce

# binary
BINARY 		= atom-ce

# object files
BIN 		= bin
BUILD 		= bin/build

# src paths
STRING_C 	= src/string/string.c
FEN_C 		= src/fen/fen.c

# rules
atom-ce: main.c string fen
	${CC} ${C_FLAGS} -o ${BIN}/${BINARY} main.c \
		${BUILD}/string.o \
		${BUILD}/fen.o


# string rule
string: ${STRING_C}
	${CC} -c ${C_FLAGS} -o ${BUILD}/string.o ${STRING_C}

fen: ${FEN_C}
	${CC} -c ${C_FLAGS} -o ${BUILD}/fen.o ${FEN_C}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
