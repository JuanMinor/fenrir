#include "include/uci/uci.h"
#include "include/selfplay/selfplay.h"
#include <iostream>
#include <cstring>

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    try
    {
        if (argc > 1 && std::strcmp(argv[1], "--selfplay") == 0)
        {
            int games = 1000;
            int simulations = 800;
            
            for (int i = 2; i < argc; ++i) {
                if (std::strcmp(argv[i], "--games") == 0 && i + 1 < argc) {
                    games = std::stoi(argv[++i]);
                } else if (std::strcmp(argv[i], "--simulations") == 0 && i + 1 < argc) {
                    simulations = std::stoi(argv[++i]);
                }
            }
            fenrir::SelfPlay sp(simulations, games);
            sp.run();
        }
        else
        {
            fenrir::UCI uci_loop;
            uci_loop.loop();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
