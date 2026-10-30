#pragma once

#include "EpeeElectricalState.h"
#include "../scanner/ContinuityMap.h"
#include "../scanner/Line.h"

namespace Sentinel
{
    /**
     * Interpret one measured continuity map as epee electrical state.
     *
     * This is a pure transformation: no timing, latching, transitive
     * closure, GPIO operations, or hit qualification.
     *
     * The four connectivity fields inspect the B conductor. They do not
     * prove physical contact at the exposed pointe d'arret.
     */
    inline EpeeElectricalState interpretEpee(const ContinuityMap& map)
    {
        return {
            map.hasContinuity(Line::RA, Line::RB),
            map.hasContinuity(Line::GA, Line::GB),
            map.hasContinuity(Line::RB, Line::GC),
            map.hasContinuity(Line::GB, Line::RC),
            map.hasContinuity(Line::RB, Line::CP),
            map.hasContinuity(Line::GB, Line::CP)
        };
    }
}
