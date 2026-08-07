#pragma once

#include <cstdint>

namespace Sentinel
{

    /**
     * @brief Logical electrical lines defined by the Sentinel Electrical Model.
     *
     * These values represent the seven logical lines used throughout the
     * electrical model. They are NOT processor GPIO numbers.
     *
     * Mapping logical lines to physical GPIO pins is the responsibility of
     * the platform-specific Hardware Abstraction Layer (HAL).
     */
    enum class Line : uint8_t
    {
        RA = 0,
        RB,
        RC,
        MT,
        GC,
        GB,
        GA,

        Count
    };

} // namespace Sentinel
