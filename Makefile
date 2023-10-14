C 				= gcc
# flags as required by C compiler
FLAGS 			= -I /workspaces/loki

# binary
BINARY 			= loki.elf

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
loki: main.c string fen piece board chess
	${C} ${FLAGS} -o ${BIN}/${BINARY} main.c ${OBJECT_FILES}

# string rule
string: ${STRING}
	${C} -c ${FLAGS} -o ${BUILD}/string.o ${STRING}

fen: ${FEN}
	${C} -c ${FLAGS} -o ${BUILD}/fen.o ${FEN}

chess: ${CHESS}
	${C} -c ${FLAGS} -o ${BUILD}/chess.o ${CHESS}

piece: ${PIECE}
	${C} -c ${FLAGS} -o ${BUILD}/piece.o ${PIECE}

board: ${BOARD}
	${C} -c ${FLAGS} -o ${BUILD}/board.o ${BOARD}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
