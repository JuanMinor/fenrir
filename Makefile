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
				  ${BUILD}/log.o

# src
BOARD			= src/chess/board.cpp
FEN 			= src/chess/fen.cpp
LOG				= src/logger/log.cpp
PIECE			= src/chess/piece.cpp

# rules
loki: main.cpp board fen log piece
	${CC} ${FLAGS} -o ${BIN}/${BINARY} main.cpp ${OBJECT_FILES}

board: ${BOARD}
	${CC} -c ${FLAGS} -o ${BUILD}/board.o ${BOARD}

fen: ${FEN}
	${CC} -c ${FLAGS} -o ${BUILD}/fen.o ${FEN}

log: ${LOG}
	${CC} -c ${FLAGS} -o ${BUILD}/log.o ${LOG}

piece: ${PIECE}
	${CC} -c ${FLAGS} -o ${BUILD}/piece.o ${PIECE}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
