#! /bin/sh

<<Banner
    Synopsis:
        Loki automated executor
Banner

set -e

_make() {
    make clean
    make
}

run() {
    ./bin/loki.elf "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq e3 0 1"
}

_make
run
