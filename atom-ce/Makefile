C 				= gcc
# flags as required by C compiler
C_FLAGS 		= -I /workspaces/atom-ce/atom-ce

# binary
BINARY 			= atom-ce.elf

# binary directories
BIN 			= bin
BUILD 			= bin/build

# object files
OBJECT_FILES 	= ${BUILD}/string.o \
				  ${BUILD}/fen.o \
				  ${BUILD}/piece.o \
				  ${BUILD}/board.o \
				  ${BUILD}/chess.o

# src paths
STRING	 		= src/string/string.c
FEN 			= src/fen/fen.c
# chess
PIECE			= src/chess/piece/piece.c
BOARD			= src/chess/board/board.c
CHESS			= src/chess/chess.c

# rules
atom-ce: main.c string fen piece board chess
	${C} ${C_FLAGS} -o ${BIN}/${BINARY} main.c ${OBJECT_FILES}


# string rule
string: ${STRING}
	${C} -c ${C_FLAGS} -o ${BUILD}/string.o ${STRING}

fen: ${FEN}
	${C} -c ${C_FLAGS} -o ${BUILD}/fen.o ${FEN}

chess: ${CHESS}
	${C} -c ${C_FLAGS} -o ${BUILD}/chess.o ${CHESS}

piece: ${PIECE}
	${C} -c ${C_FLAGS} -o ${BUILD}/piece.o ${PIECE}

board: ${BOARD}
	${C} -c ${C_FLAGS} -o ${BUILD}/board.o ${BOARD}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
