#pragma once

namespace Sentinel
{
    /**
     * Interpreted epee electrical conditions from one ContinuityMap.
     *
     * Each field describes measured electrical connectivity or an inferred
     * switch condition. No hit qualification or scoring decision is made.
     */
    struct EpeeElectricalState
    {
        bool redTipDepressed;
        bool greenTipDepressed;

        // "Earth" means the corresponding fencer's C circuit.
        bool redTipConnectedToGreenEarth;
        bool greenTipConnectedToRedEarth;

        // "Piste" means the distinct CP logical line.
        bool redTipConnectedToPiste;
        bool greenTipConnectedToPiste;
    };
}
