#include "../firmware/scanner/Line.h"
#include "../firmware/scanner/ContinuityMap.h"

#include <iostream>

int main()
{
    Sentinel::ContinuityMap map;

    std::cout << "Sentinel scanner smoke test" << std::endl;
    std::cout << "Raw continuity map: " << map.raw() << std::endl;

    return 0;
}
