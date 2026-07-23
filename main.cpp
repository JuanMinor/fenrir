/*
 *   Copyright (c) 2026 Juan Minor
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "include/uci/uci.h"
#include "include/selfplay/selfplay.h"
#ifdef ENABLE_AUTOTUNE_FLAG
#include "include/tuner/auto_tuner.h"
#endif
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
            int gpu_id = 0;
            
            for (int i = 2; i < argc; ++i) {
                if (std::strcmp(argv[i], "--games") == 0 && i + 1 < argc) {
                    games = std::stoi(argv[++i]);
                } else if (std::strcmp(argv[i], "--simulations") == 0 && i + 1 < argc) {
                    simulations = std::stoi(argv[++i]);
                } else if (std::strcmp(argv[i], "--gpu-id") == 0 && i + 1 < argc) {
                    gpu_id = std::stoi(argv[++i]);
                }
            }
            chess::SelfPlay sp(gpu_id, simulations, games);
            sp.run();
        }
#ifdef ENABLE_AUTOTUNE_FLAG
        else if (argc > 1 && std::strcmp(argv[1], "--auto-tune") == 0)
        {
            double gpu_usage = 100.0;
            double cpu_usage = 100.0;

            for (int i = 2; i < argc; ++i) {
                if (std::strcmp(argv[i], "--gpu-usage") == 0 && i + 1 < argc) {
                    gpu_usage = std::stod(argv[++i]);
                } else if (std::strcmp(argv[i], "--cpu-usage") == 0 && i + 1 < argc) {
                    cpu_usage = std::stod(argv[++i]);
                }
            }

            tuner::TuningParameters baseline(false);
            tuner::AutoTuner auto_tuner(baseline, gpu_usage, cpu_usage);
            auto_tuner.run();
        }
#endif
        else
        {
            chess::UCI uci_loop;
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
