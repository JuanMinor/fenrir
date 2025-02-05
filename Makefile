 #
 #   Copyright (c) 2025 Juan Minor

 #   This program is free software: you can redistribute it and/or modify
 #   it under the terms of the GNU General Public License as published by
 #   the Free Software Foundation, either version 3 of the License, or
 #   (at your option) any later version.

 #   This program is distributed in the hope that it will be useful,
 #   but WITHOUT ANY WARRANTY; without even the implied warranty of
 #   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 #   GNU General Public License for more details.

 #   You should have received a copy of the GNU General Public License
 #   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 #

CC 					= g++
# flags as required by C++ compiler
FLAGS 				= -g -I /workspaces/loki

# binary
BINARY 				= loki.elf

# logs
LOGS 				= logs/loki.log

# PGN (Portable Game Notation)
PGN_FILES			= pgn/loki.pgn \
				  	  pgn/loki.store.txt

# binary directories
BIN 				= bin
BUILD 				= bin/build

# object files
OBJECT_FILE_PATHS 	= ${BUILD}/board.o \
				  	  ${BUILD}/chrono.o \
				  	  ${BUILD}/fen.o \
				  	  ${BUILD}/logger.o \
				  	  ${BUILD}/modifier.o \
				  	  ${BUILD}/moves.o \
				  	  ${BUILD}/pgn.o \
				  	  ${BUILD}/piece.o \

OBJECT_FILES 		= board.o \
				  	  chrono.o \
				  	  fen.o \
				  	  logger.o \
				  	  modifier.o \
				  	  moves.o \
				  	  pgn.o \
				  	  piece.o \

# src
BOARD				= src/chess/board.cpp
CHRONO				= src/chrono/chrono.cpp
FEN 				= src/chess/fen.cpp
LOGGER				= src/logger/logger.cpp
MODIFIER			= src/modifier/modifier.cpp
MOVES				= src/chess/moves.cpp
PGN					= src/pgn/pgn.cpp
PIECE				= src/chess/piece.cpp

# rules
loki.bin: main.cpp ${OBJECT_FILES}
	${CC} ${FLAGS} -o ${BIN}/${BINARY} main.cpp ${OBJECT_FILE_PATHS}

board.o: ${BOARD}
	${CC} -c ${FLAGS} -o ${BUILD}/board.o ${BOARD}

chrono.o: ${CHRONO}
	${CC} -c ${FLAGS} -o ${BUILD}/chrono.o ${CHRONO}

fen.o: ${FEN}
	${CC} -c ${FLAGS} -o ${BUILD}/fen.o ${FEN}

logger.o: ${LOGGER}
	${CC} -c ${FLAGS} -o ${BUILD}/logger.o ${LOGGER}

modifier.o: ${MODIFIER}
	${CC} -c ${FLAGS} -o ${BUILD}/modifier.o ${MODIFIER}

moves.o: ${MOVES}
	${CC} -c ${FLAGS} -o ${BUILD}/moves.o ${MOVES}

pgn.o: ${PGN}
	${CC} -c ${FLAGS} -o ${BUILD}/pgn.o ${PGN}

piece.o: ${PIECE}
	${CC} -c ${FLAGS} -o ${BUILD}/piece.o ${PIECE}

clean:
	rm -rf ${BUILD}/*
	rm -rf ${BIN}/${BINARY}
	rm -rf ${LOGS}
	rm -rf ${PGN_FILES}
