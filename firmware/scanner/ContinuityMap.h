#pragma once

#include <cstdint>

#include "Line.h"

namespace Sentinel
{

    /**
     * @brief Canonical representation of one electrical snapshot.
     *
     * ContinuityMap stores the measured electrical continuity between every
     * unique pair of Sentinel's seven logical lines.
     *
     * The mapping between line pairs and bit positions is defined by the
     * Electrical Model specification.
     *
     * This class intentionally contains no knowledge of:
     *
     *   - fencing rules
     *   - GPIO
     *   - processors
     *   - hardware
     *   - timing
     *
     * It is simply a compact representation of measured continuity.
     */
    class ContinuityMap
    {
    public:
        /// Constructs an empty continuity map.
        constexpr ContinuityMap() : m_bits(0)
        {
        }

        /// Clears all continuity measurements.
        constexpr void clear()
        {
            m_bits = 0;
        }

        /// Returns the raw 21-bit bitmap.
        constexpr uint32_t raw() const
        {
            return m_bits;
        }

        /**
         * Returns true if continuity exists between two logical lines.
         */
        constexpr bool hasContinuity(Line a, Line b) const
        {
            if (a == b)
                return false;

            return (m_bits & (1UL << bit(a, b))) != 0;
        }

    private:
        template <typename NodeIO>
        friend class ContinuityScanner;
        /**
         * Records measured continuity between two logical lines.
         *
         * Only the scanner may modify the electrical model.
         */
        constexpr void set(Line a, Line b)
        {
            m_bits |= (1UL << bit(a, b));
        }

        /**
         * Converts a unique unordered line pair into its canonical bit index.
         */
        static constexpr uint8_t bit(Line a, Line b)
        {
            uint8_t x = static_cast<uint8_t>(a);
            uint8_t y = static_cast<uint8_t>(b);

            if (x > y)
            {
                uint8_t t = x;
                x = y;
                y = t;
            }

            constexpr uint8_t N = static_cast<uint8_t>(Line::Count);

            return (x * (2 * N - x - 1)) / 2 +
                   (y - x - 1);
        }

    private:
        uint32_t m_bits;
    };

} // namespace Sentinel
