//
//  main.c
//  @loki
//
//  Created by Juan Minor on 3/19/23.
//

#include "include/chess/chess.h"
#include "include/string/string.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("No arguments found. Supply a valid FEN string.\n");
        return 1;
    }
    if (argv[1] && !trim(argv[1])[0])
    {
        printf("FEN string cannot be empty or null.\n");
        return 2;
    }

    draw(trim(argv[1]));
    
    return 0;
}
