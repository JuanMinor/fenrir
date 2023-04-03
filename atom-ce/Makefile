CC 				= gcc
# flags as required by the gcc
C_FLAGS 		= -I /workspaces/atom-ce/atom-ce

# binary
BINARY 			= atom-ce

# binary directories
BIN 			= bin
BUILD 			= bin/build

# object files
OBJECT_FILES 	= ${BUILD}/string.o \
				  ${BUILD}/fen.o \
				  ${BUILD}/piece.o \
				  ${BUILD}/board.o

# src paths
STRING	 		= src/string/string.c
FEN 			= src/fen/fen.c
# chess
PIECE			= src/chess/piece/piece.c
BOARD			= src/chess/board/board.c

# rules
atom-ce: main.c string fen piece board
	${CC} ${C_FLAGS} -o ${BIN}/${BINARY} main.c ${OBJECT_FILES}


# string rule
string: ${STRING}
	${CC} -c ${C_FLAGS} -o ${BUILD}/string.o ${STRING}

fen: ${FEN}
	${CC} -c ${C_FLAGS} -o ${BUILD}/fen.o ${FEN}

piece: ${PIECE}
	${CC} -c ${C_FLAGS} -o ${BUILD}/piece.o ${PIECE}

board: ${BOARD}
	${CC} -c ${C_FLAGS} -o ${BUILD}/board.o ${BOARD}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
