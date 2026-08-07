#pragma once

#include <cstdint>

#include "ContinuityMap.h"
#include "Line.h"

namespace Sentinel
{

    /**
     * @brief Measures continuity between Sentinel's seven logical lines.
     *
     * NodeIO must provide:
     *
     *     void drive(Line line);
     *     uint8_t snapshot();
     *
     * snapshot() returns the current state of all seven logical lines:
     *
     *     bit 0 = RA
     *     bit 1 = RB
     *     bit 2 = RC
     *     bit 3 = MT
     *     bit 4 = GC
     *     bit 5 = GB
     *     bit 6 = GA
     *
     * The scanner contains no processor-specific or GPIO-specific code.
     */
    template <typename NodeIO>
    class ContinuityScanner
    {
    public:
        explicit constexpr ContinuityScanner(NodeIO &io)
            : m_io(io)
        {
        }

        ContinuityMap scan()
        {
            ContinuityMap map;

            scanFrom(map, Line::RA, 1);
            scanFrom(map, Line::RB, 2);
            scanFrom(map, Line::RC, 3);
            scanFrom(map, Line::MT, 4);
            scanFrom(map, Line::GC, 5);
            scanFrom(map, Line::GB, 6);

            return map;
        }

    private:
        void scanFrom(ContinuityMap &map, Line driveLine, uint8_t firstSense)
        {
            m_io.drive(driveLine);

            const uint8_t state = m_io.snapshot();

            for (uint8_t sense = firstSense;
                 sense < static_cast<uint8_t>(Line::Count);
                 ++sense)
            {
                const uint8_t mask = static_cast<uint8_t>(1U << sense);

                if ((state & mask) != 0)
                {
                    map.set(
                        driveLine,
                        static_cast<Line>(sense));
                }
            }
        }

        NodeIO &m_io;
    };

} // namespace Sentinel
