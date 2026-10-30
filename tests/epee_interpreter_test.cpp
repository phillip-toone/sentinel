#include "../firmware/interpreter/EpeeInterpreter.h"
#include "../firmware/scanner/ContinuityScanner.h"
#include "MockNodeIO.h"

#include <cassert>
#include <iostream>

using Sentinel::ContinuityScanner;
using Sentinel::EpeeElectricalState;
using Sentinel::Line;
using Sentinel::MockNodeIO;
using Sentinel::interpretEpee;

static EpeeElectricalState interpret(MockNodeIO& io)
{
    ContinuityScanner<MockNodeIO> scanner(io);
    return interpretEpee(scanner.scan());
}

static void expect(const EpeeElectricalState& s,
                   bool redTip, bool greenTip,
                   bool redEarth, bool greenEarth,
                   bool redPiste, bool greenPiste)
{
    assert(s.redTipDepressed == redTip);
    assert(s.greenTipDepressed == greenTip);
    assert(s.redTipConnectedToGreenEarth == redEarth);
    assert(s.greenTipConnectedToRedEarth == greenEarth);
    assert(s.redTipConnectedToPiste == redPiste);
    assert(s.greenTipConnectedToPiste == greenPiste);
}

int main()
{
    {
        MockNodeIO io;
        expect(interpret(io), false, false, false, false, false, false);
    }

    // Exercise each scoring-relevant pair independently.
    {
        MockNodeIO io;
        io.connect(Line::RA, Line::RB);
        expect(interpret(io), true, false, false, false, false, false);
    }
    {
        MockNodeIO io;
        io.connect(Line::GA, Line::GB);
        expect(interpret(io), false, true, false, false, false, false);
    }
    {
        MockNodeIO io;
        io.connect(Line::RB, Line::GC);
        expect(interpret(io), false, false, true, false, false, false);
    }
    {
        MockNodeIO io;
        io.connect(Line::GB, Line::RC);
        expect(interpret(io), false, false, false, true, false, false);
    }
    {
        MockNodeIO io;
        io.connect(Line::RB, Line::CP);
        expect(interpret(io), false, false, false, false, true, false);
    }
    {
        MockNodeIO io;
        io.connect(Line::GB, Line::CP);
        expect(interpret(io), false, false, false, false, false, true);
    }

    // An unrelated pair must not change any of the six fields.
    {
        MockNodeIO io;
        io.connect(Line::RA, Line::GC);
        expect(interpret(io), false, false, false, false, false, false);
    }

    // A physically transitive network: RA--RB--GC--CP.
    // Closing RA--RB causes RB to be connected to earth and piste.
    {
        MockNodeIO io;
        io.connect(Line::RA, Line::RB);
        io.connect(Line::RA, Line::GC);
        io.connect(Line::GC, Line::CP);
        expect(interpret(io), true, false, true, false, true, false);
    }

    // Both tips depressed and their B conductors connected to piste.
    // Both B conductors connect to CP, but neither weapon earth
    // circuit (RC or GC) is connected to that network.
    // Therefore both piste fields are true and both earth fields
    // remain false.
    {
        MockNodeIO io;
        io.connect(Line::RA, Line::RB);
        io.connect(Line::GA, Line::GB);
        io.connect(Line::RB, Line::CP);
        io.connect(Line::GB, Line::CP);
        expect(interpret(io), true, true, false, false, true, true);
    }

    std::cout << "All epee interpreter tests passed.\n";
    return 0;
}
