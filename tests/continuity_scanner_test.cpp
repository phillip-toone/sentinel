#include "../firmware/scanner/ContinuityScanner.h"
#include "MockNodeIO.h"

#include <cassert>
#include <iostream>

using Sentinel::ContinuityScanner;
using Sentinel::Line;
using Sentinel::MockNodeIO;

int main()
{
    MockNodeIO io;

    // Simulate one electrically connected group:
    //
    //     RA ----- GC ----- MT
    //
    // Because continuity is transitive, all three pairs should
    // measure as continuous:
    //
    //     RA-GC
    //     RA-MT
    //     GC-MT
    //
    io.connect(Line::RA, Line::GC);
    io.connect(Line::GC, Line::MT);

    ContinuityScanner<MockNodeIO> scanner(io);

    const auto map = scanner.scan();

    assert(map.hasContinuity(Line::RA, Line::GC));
    assert(map.hasContinuity(Line::GC, Line::RA));

    assert(map.hasContinuity(Line::RA, Line::MT));
    assert(map.hasContinuity(Line::MT, Line::RA));

    assert(map.hasContinuity(Line::GC, Line::MT));
    assert(map.hasContinuity(Line::MT, Line::GC));

    // An unrelated line must remain disconnected.
    assert(!map.hasContinuity(Line::RA, Line::RB));

    std::cout << "Continuity scanner test passed." << std::endl;
    std::cout << "Raw map: 0x"
              << std::hex
              << map.raw()
              << std::endl;

    return 0;
}
