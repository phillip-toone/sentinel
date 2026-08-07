#pragma once

#include <array>
#include <cstdint>

#include "../firmware/scanner/Line.h"

namespace Sentinel
{

    /**
     * @brief Software simulation of Sentinel's seven electrical lines.
     *
     * MockNodeIO models physical continuity between lines without requiring
     * any hardware. It implements the interface expected by
     * ContinuityScanner:
     *
     *     void drive(Line line);
     *     uint8_t snapshot();
     */
    class MockNodeIO
    {
    public:
        MockNodeIO()
            : m_driven(Line::RA)
        {
            for (auto &row : m_connected)
            {
                row.fill(false);
            }
        }

        /**
         * @brief Adds physical continuity between two lines.
         */
        void connect(Line a, Line b)
        {
            const auto x = index(a);
            const auto y = index(b);

            m_connected[x][y] = true;
            m_connected[y][x] = true;
        }

        /**
         * @brief Removes physical continuity between two lines.
         */
        void disconnect(Line a, Line b)
        {
            const auto x = index(a);
            const auto y = index(b);

            m_connected[x][y] = false;
            m_connected[y][x] = false;
        }

        /**
         * @brief Selects the line currently being driven HIGH.
         */
        void drive(Line line)
        {
            m_driven = line;
        }

        /**
         * @brief Returns the electrical state of all seven lines.
         *
         * Continuity is transitive. If RA is connected to GC and GC is
         * connected to MT, driving RA causes RA, GC, and MT all to read HIGH.
         *
         * Returned bit positions are:
         *
         *     bit 0 = RA
         *     bit 1 = RB
         *     bit 2 = RC
         *     bit 3 = MT
         *     bit 4 = GC
         *     bit 5 = GB
         *     bit 6 = GA
         */
        uint8_t snapshot() const
        {
            constexpr auto lineCount =
                static_cast<std::size_t>(Line::Count);

            std::array<bool, lineCount> reached{};
            std::array<Line, lineCount> queue{};

            std::size_t head = 0;
            std::size_t tail = 0;

            const auto start = index(m_driven);

            reached[start] = true;
            queue[tail++] = m_driven;

            // Traverse every line electrically reachable from the driven line.
            while (head < tail)
            {
                const Line current = queue[head++];
                const auto currentIndex = index(current);

                for (std::size_t next = 0; next < lineCount; ++next)
                {
                    if (m_connected[currentIndex][next] && !reached[next])
                    {
                        reached[next] = true;
                        queue[tail++] = static_cast<Line>(next);
                    }
                }
            }

            uint8_t state = 0;

            for (std::size_t line = 0; line < lineCount; ++line)
            {
                if (reached[line])
                {
                    state |= static_cast<uint8_t>(1U << line);
                }
            }

            return state;
        }

    private:
        static constexpr std::size_t index(Line line)
        {
            return static_cast<std::size_t>(line);
        }

        static constexpr std::size_t lineCount =
            static_cast<std::size_t>(Line::Count);

        std::array<std::array<bool, lineCount>, lineCount> m_connected{};
        Line m_driven;
    };

} // namespace Sentinel
