#include "../firmware/scanner/ContinuityScanner.h"
#include "MockNodeIO.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

using Sentinel::ContinuityScanner;
using Sentinel::Line;
using Sentinel::MockNodeIO;

struct PairTest
{
    Line a;
    Line b;
    uint8_t expectedBit;
};

constexpr std::array<PairTest, 21> pairTests{{
    {Line::RA, Line::RB, 0},
    {Line::RA, Line::RC, 1},
    {Line::RA, Line::MT, 2},
    {Line::RA, Line::GC, 3},
    {Line::RA, Line::GB, 4},
    {Line::RA, Line::GA, 5},

    {Line::RB, Line::RC, 6},
    {Line::RB, Line::MT, 7},
    {Line::RB, Line::GC, 8},
    {Line::RB, Line::GB, 9},
    {Line::RB, Line::GA, 10},

    {Line::RC, Line::MT, 11},
    {Line::RC, Line::GC, 12},
    {Line::RC, Line::GB, 13},
    {Line::RC, Line::GA, 14},

    {Line::MT, Line::GC, 15},
    {Line::MT, Line::GB, 16},
    {Line::MT, Line::GA, 17},

    {Line::GC, Line::GB, 18},
    {Line::GC, Line::GA, 19},

    {Line::GB, Line::GA, 20},
}};

void testAllCanonicalPairs()
{
    for (const auto &test : pairTests)
    {
        MockNodeIO io;
        io.connect(test.a, test.b);

        ContinuityScanner<MockNodeIO> scanner(io);
        const auto map = scanner.scan();

        const uint32_t expected =
            uint32_t{1} << test.expectedBit;

        assert(map.raw() == expected);

        // Continuity must be symmetric at the API boundary.
        assert(map.hasContinuity(test.a, test.b));
        assert(map.hasContinuity(test.b, test.a));
    }
}

void testTransitiveTriangle()
{
    MockNodeIO io;

    // Physical network:
    //
    //     RA ----- GC ----- MT
    //
    io.connect(Line::RA, Line::GC);
    io.connect(Line::GC, Line::MT);

    ContinuityScanner<MockNodeIO> scanner(io);
    const auto map = scanner.scan();

    assert(map.hasContinuity(Line::RA, Line::GC));
    assert(map.hasContinuity(Line::RA, Line::MT));
    assert(map.hasContinuity(Line::GC, Line::MT));

    // Canonical bits:
    //
    // RA-MT = bit 2
    // RA-GC = bit 3
    // MT-GC = bit 15
    //
    constexpr uint32_t expected =
        (uint32_t{1} << 2) |
        (uint32_t{1} << 3) |
        (uint32_t{1} << 15);

    assert(map.raw() == expected);
}

void testIndependentComponents()
{
    MockNodeIO io;

    // Three electrically independent components:
    //
    //     RA ----- GC ----- MT
    //
    //     RB ----- RC
    //
    //     GB ----- GA
    //
    io.connect(Line::RA, Line::GC);
    io.connect(Line::GC, Line::MT);

    io.connect(Line::RB, Line::RC);

    io.connect(Line::GB, Line::GA);

    ContinuityScanner<MockNodeIO> scanner(io);
    const auto map = scanner.scan();

    // Component 1
    assert(map.hasContinuity(Line::RA, Line::GC));
    assert(map.hasContinuity(Line::RA, Line::MT));
    assert(map.hasContinuity(Line::GC, Line::MT));

    // Component 2
    assert(map.hasContinuity(Line::RB, Line::RC));

    // Component 3
    assert(map.hasContinuity(Line::GB, Line::GA));

    // Verify representative lines from different components
    // remain electrically isolated.
    assert(!map.hasContinuity(Line::RA, Line::RB));
    assert(!map.hasContinuity(Line::GC, Line::RC));
    assert(!map.hasContinuity(Line::MT, Line::GA));
    assert(!map.hasContinuity(Line::RB, Line::GB));

    constexpr uint32_t expected =
        (uint32_t{1} << 2) |  // RA-MT
        (uint32_t{1} << 3) |  // RA-GC
        (uint32_t{1} << 6) |  // RB-RC
        (uint32_t{1} << 15) | // MT-GC
        (uint32_t{1} << 20);  // GB-GA

    assert(map.raw() == expected);
}

int main()
{
    testAllCanonicalPairs();
    testTransitiveTriangle();
    testIndependentComponents();

    std::cout
        << "All continuity scanner tests passed."
        << std::endl;

    return 0;
}
