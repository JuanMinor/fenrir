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

#include "include/tuner/tuning_parameters.h"

namespace tuner
{
    const std::string BENCHMARK_KIWIPETE_FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    class AutoTuner
    {
        TuningParameters baseline_tuning_parameters;
        TuningParameters real_tuning_parameters;

        void print_detected_hardware();

    public:
        /**
         * @brief Initialize the auto-tuner with baseline tuning parameters.
         * @param tuning_parameters Reference to tuning parameters to optimize.
         */
        AutoTuner(TuningParameters &tuning_parameters);

        /**
         * @brief Destructor.
         */
        ~AutoTuner();

        /**
         * @brief Run the auto-tuning process to find optimal parameters.
         * @returns Optimized TuningParameters based on hardware benchmarks.
         */
        TuningParameters run();
    };
}
