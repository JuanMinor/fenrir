CC 				= g++
# flags as required by C++ compiler
FLAGS 			= -I /workspaces/loki

# binary
BINARY 			= loki.elf

# binary directories
BIN 			= bin
BUILD 			= bin/build

# object files
OBJECT_FILES 	= ${BUILD}/fen.o \
				  ${BUILD}/piece.o \
				  ${BUILD}/board.o \
				  ${BUILD}/logger.o \
				  ${BUILD}/pgn.o

# src
BOARD			= src/chess/board.cpp
FEN 			= src/chess/fen.cpp
LOGGER			= src/logger/logger.cpp
PGN				= src/pgn/pgn.cpp
PIECE			= src/chess/piece.cpp

# rules
loki.bin: main.cpp board.o fen.o logger.o piece.o pgn.o
	${CC} ${FLAGS} -o ${BIN}/${BINARY} main.cpp ${OBJECT_FILES}

board.o: ${BOARD}
	${CC} -c ${FLAGS} -o ${BUILD}/board.o ${BOARD}

fen.o: ${FEN}
	${CC} -c ${FLAGS} -o ${BUILD}/fen.o ${FEN}

logger.o: ${LOGGER}
	${CC} -c ${FLAGS} -o ${BUILD}/logger.o ${LOGGER}

pgn.o: ${PGN}
	${CC} -c ${FLAGS} -o ${BUILD}/pgn.o ${PGN}

piece.o: ${PIECE}
	${CC} -c ${FLAGS} -o ${BUILD}/piece.o ${PIECE}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
