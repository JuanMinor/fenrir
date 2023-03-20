//
//  main.c
//  @atom-ce
//
//  Created by Juan Minor on 3/19/23.
//

#include "include/string/string.h"
#include "include/fen/fen.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No arguments found. Please supply a valid FEN string.\n");
        return 1;
    }
    if (argv[1] && !trim(argv[1])[0])
    { // check argument passed is not empty or null
        printf("FEN string cannot be empty or null.\n");
        return 2;
    }

    print_fen(argv[1]);

    return 0;
}
