#include <Arduino.h>

// ============================================================
// Sentinel Experiment 07
// ESP32 External Pull-Up Characterization
//
// Purpose:
//   Determine how external pull-up resistance affects the
//   settling behavior and reliability of the full 21-pair
//   continuity scan with realistic fencing cabling.
//
// Controlled variables:
//   - Same TTGO T-Display V1.1 GPIO mapping as Experiment 05
//   - Same active-low measurement method
//   - Same external fencing hardware
//   - Same canonical 21-bit representation
//   - Zero explicit release settling
//   - Same trusted-reference methodology
//   - Same Forward, Reverse, and Interleaved scan orders
//
// Independent variables:
//   - External pull-up resistance
//   - Drive-to-sample settling time
//
// Electrical change:
//   Released and sensed GPIOs use INPUT rather than
//   INPUT_PULLUP. External resistors provide the pull-up.
//
// Critical design rule:
//   Scan order NEVER determines canonical bit position.
//   Every measured unordered pair is translated independently
//   into Sentinel's canonical 21-bit representation.
// ============================================================

// ============================================================
// Logical lines
// ============================================================
//
// Numeric order intentionally matches Sentinel's canonical
// logical-line order.
//
//   0 RA
//   1 RB
//   2 RC
//   3 MT
//   4 GC
//   5 GB
//   6 GA
// ============================================================

enum class Line : uint8_t
{
    RA = 0,
    RB = 1,
    RC = 2,
    MT = 3,
    GC = 4,
    GB = 5,
    GA = 6
};

constexpr size_t LINE_COUNT = 7;
constexpr size_t PAIR_COUNT = 21;


// ============================================================
// GPIO mapping - TTGO T-Display V1.1
// ============================================================

constexpr uint8_t pins[LINE_COUNT] =
{
    21, // RA
    22, // RB
    17, // RC
    32, // MT
    25, // GC
    26, // GB
    27  // GA
};

constexpr const char* lineNames[LINE_COUNT] =
{
    "RA",
    "RB",
    "RC",
    "MT",
    "GC",
    "GB",
    "GA"
};


// ============================================================
// Canonical pair names
// ============================================================
//
//  0  RA-RB
//  1  RA-RC
//  2  RA-MT
//  3  RA-GC
//  4  RA-GB
//  5  RA-GA
//
//  6  RB-RC
//  7  RB-MT
//  8  RB-GC
//  9  RB-GB
// 10  RB-GA
//
// 11  RC-MT
// 12  RC-GC
// 13  RC-GB
// 14  RC-GA
//
// 15  MT-GC
// 16  MT-GB
// 17  MT-GA
//
// 18  GC-GB
// 19  GC-GA
//
// 20  GB-GA
// ============================================================

constexpr const char* pairNames[PAIR_COUNT] =
{
    "RA-RB",
    "RA-RC",
    "RA-MT",
    "RA-GC",
    "RA-GB",
    "RA-GA",

    "RB-RC",
    "RB-MT",
    "RB-GC",
    "RB-GB",
    "RB-GA",

    "RC-MT",
    "RC-GC",
    "RC-GB",
    "RC-GA",

    "MT-GC",
    "MT-GB",
    "MT-GA",

    "GC-GB",
    "GC-GA",

    "GB-GA"
};


// ============================================================
// Canonical pair mapping
// ============================================================
//
// For logical line indices a < b:
//
//   bit = number of pairs preceding row a
//         + offset of b within row a
//
// Rows:
//
// RA: 0..5
// RB: 6..10
// RC: 11..14
// MT: 15..17
// GC: 18..19
// GB: 20
//
// This function accepts the pair in either order.
// ============================================================

uint8_t canonicalBit(Line first, Line second)
{
    uint8_t a = static_cast<uint8_t>(first);
    uint8_t b = static_cast<uint8_t>(second);

    if (a > b)
    {
        const uint8_t temp = a;
        a = b;
        b = temp;
    }

    // Self-pairs are never requested by the scanner.
    //
    // Number of canonical pairs before row a:
    //
    // a * (13 - a) / 2
    //
    // Then add the position of b within that row.

    return static_cast<uint8_t>(
        (a * (13 - a)) / 2 +
        (b - a - 1)
    );
}


void setContinuity(
    uint32_t& map,
    Line first,
    Line second
)
{
    const uint8_t bit =
        canonicalBit(first, second);

    map |= (uint32_t{1} << bit);
}


// ============================================================
// Scan orders
// ============================================================

struct ScanOrder
{
    const char* name;
    Line lines[LINE_COUNT];
};


constexpr ScanOrder scanOrders[] =
{
    {
        "Forward",
        {
            Line::RA,
            Line::RB,
            Line::RC,
            Line::MT,
            Line::GC,
            Line::GB,
            Line::GA
        }
    },

    {
        "Reverse",
        {
            Line::GA,
            Line::GB,
            Line::GC,
            Line::MT,
            Line::RC,
            Line::RB,
            Line::RA
        }
    },

    {
        "Interleaved",
        {
            Line::RA,
            Line::GA,
            Line::RB,
            Line::GB,
            Line::RC,
            Line::GC,
            Line::MT
        }
    }
};


constexpr size_t ORDER_COUNT =
    sizeof(scanOrders) /
    sizeof(scanOrders[0]);


// ============================================================
// Reference configuration
// ============================================================
//
// Reference uses deliberately conservative timing.
//
// Experiment 04 found no evidence that release settling corrected
// the observed failure mechanism. Nevertheless, reference
// measurement remains conservative on both sides so that the
// reference does not depend on the experimental scan timing.
//
// Reference always uses canonical Forward order.
// ============================================================

constexpr uint32_t REFERENCE_DRIVE_SETTLE_US = 1000;
constexpr uint32_t REFERENCE_RELEASE_SETTLE_US = 1000;
constexpr uint32_t REFERENCE_SCANS = 1000;


// ============================================================
// Characterization configuration
// ============================================================

constexpr uint32_t SCANS_PER_CONDITION = 10000;

// Experiment 04 showed that 0-20 us was generally far below the
// useful region for sensitive floor-cord/reel configurations.
//
// Experiment 05 therefore concentrates around and above the
// previously observed transition region.

constexpr uint32_t driveSettlingDelaysUs[] =
{
    0,
    10,
    20,
    50,
    100,
    150,
    200
};

constexpr size_t DELAY_COUNT =
    sizeof(driveSettlingDelaysUs) /
    sizeof(driveSettlingDelaysUs[0]);

// Experiment 04 did not show an advantage from explicit release
// settling for the observed failure mechanism.

constexpr uint32_t EXPERIMENT_RELEASE_SETTLE_US = 0;


// ============================================================
// Utility functions
// ============================================================

constexpr uint8_t lineIndex(Line line)
{
    return static_cast<uint8_t>(line);
}


uint8_t pinFor(Line line)
{
    return pins[lineIndex(line)];
}


const char* nameFor(Line line)
{
    return lineNames[lineIndex(line)];
}


// ============================================================
// GPIO handling
// ============================================================

void idleAllLines()
{
    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        pinMode(pins[i], INPUT);
    }
}


void beginMeasurement(
    Line line,
    uint32_t driveSettleUs
)
{
    const uint8_t pin = pinFor(line);

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    if (driveSettleUs > 0)
    {
        delayMicroseconds(driveSettleUs);
    }
}


void endMeasurement(
    Line line,
    uint32_t releaseSettleUs
)
{
    const uint8_t pin = pinFor(line);

    pinMode(pin, INPUT);
//    digitalWrite(pin, HIGH);

    if (releaseSettleUs > 0)
    {
        delayMicroseconds(releaseSettleUs);
    }
}


// ============================================================
// Arbitrary-order full scan
// ============================================================
//
// The order array defines measurement priority.
//
// At position 0:
//   drive that line and sense positions 1..6
//
// At position 1:
//   drive that line and sense positions 2..6
//
// ...
//
// At position 5:
//   drive that line and sense position 6
//
// Position 6 never needs to be driven.
//
// Therefore every unordered pair is measured exactly once:
//
//   6 + 5 + 4 + 3 + 2 + 1 = 21
//
// Critically, canonicalBit() determines where the result is
// stored. Measurement order does NOT determine bitmap order.
// ============================================================

uint32_t scanContinuity(
    const ScanOrder& order,
    uint32_t driveSettleUs,
    uint32_t releaseSettleUs
)
{
    uint32_t map = 0;

    for (
        uint8_t drivePosition = 0;
        drivePosition < LINE_COUNT - 1;
        ++drivePosition
    )
    {
        const Line driveLine =
            order.lines[drivePosition];

        beginMeasurement(
            driveLine,
            driveSettleUs
        );

        for (
            uint8_t sensePosition = drivePosition + 1;
            sensePosition < LINE_COUNT;
            ++sensePosition
        )
        {
            const Line senseLine =
                order.lines[sensePosition];

            if (digitalRead(pinFor(senseLine)) == LOW)
            {
                setContinuity(
                    map,
                    driveLine,
                    senseLine
                );
            }
        }

        endMeasurement(
            driveLine,
            releaseSettleUs
        );
    }

    return map;
}


// ============================================================
// Formatting
// ============================================================

void printBinary21(uint32_t value)
{
    for (int bit = 20; bit >= 0; --bit)
    {
        Serial.print(
            (value & (uint32_t{1} << bit))
                ? '1'
                : '0'
        );
    }
}


void printMap(uint32_t value)
{
    Serial.printf(
        "0x%06lX  ",
        static_cast<unsigned long>(value)
    );

    printBinary21(value);
}


void printPinConfiguration()
{
    Serial.println("GPIO mapping:");

    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        Serial.printf(
            "  %s = GPIO %u\n",
            lineNames[i],
            pins[i]
        );
    }
}


void printScanOrder(const ScanOrder& order)
{
    Serial.printf(
        "%s: ",
        order.name
    );

    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        Serial.print(
            nameFor(order.lines[i])
        );

        if (i + 1 < LINE_COUNT)
        {
            Serial.print(" -> ");
        }
    }

    Serial.println();
}


// ============================================================
// Validate scan-order definitions
// ============================================================
//
// This catches accidental duplicate/missing lines in an order.
// ============================================================

bool scanOrderIsValid(const ScanOrder& order)
{
    bool seen[LINE_COUNT] = {};

    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        const uint8_t index =
            lineIndex(order.lines[i]);

        if (index >= LINE_COUNT)
        {
            return false;
        }

        if (seen[index])
        {
            return false;
        }

        seen[index] = true;
    }

    return true;
}


bool validateAllScanOrders()
{
    for (size_t i = 0; i < ORDER_COUNT; ++i)
    {
        if (!scanOrderIsValid(scanOrders[i]))
        {
            Serial.printf(
                "ERROR: Invalid scan order: %s\n",
                scanOrders[i].name
            );

            return false;
        }
    }

    return true;
}


// ============================================================
// Reference characterization
// ============================================================

struct ReferenceResult
{
    bool valid = false;

    uint32_t referenceMap = 0;

    uint32_t scansCompleted = 0;
    uint32_t disagreeingScans = 0;

    uint32_t firstDisagreementScan = 0;
    uint32_t firstDisagreementMap = 0;
};


ReferenceResult establishReference()
{
    ReferenceResult result;

    idleAllLines();
    delay(100);

    // Use Forward order for the conservative reference.
    const ScanOrder& referenceOrder =
        scanOrders[0];

    result.referenceMap =
        scanContinuity(
            referenceOrder,
            REFERENCE_DRIVE_SETTLE_US,
            REFERENCE_RELEASE_SETTLE_US
        );

    result.scansCompleted = 1;

    for (
        uint32_t scan = 2;
        scan <= REFERENCE_SCANS;
        ++scan
    )
    {
        const uint32_t observed =
            scanContinuity(
                referenceOrder,
                REFERENCE_DRIVE_SETTLE_US,
                REFERENCE_RELEASE_SETTLE_US
            );

        ++result.scansCompleted;

        if (observed != result.referenceMap)
        {
            ++result.disagreeingScans;

            if (result.firstDisagreementScan == 0)
            {
                result.firstDisagreementScan = scan;
                result.firstDisagreementMap = observed;
            }
        }
    }

    result.valid =
        (result.disagreeingScans == 0);

    return result;
}


void printReferenceResult(
    const ReferenceResult& result
)
{
    Serial.println();
    Serial.println(
        "=================================================="
    );

    Serial.println(
        "REFERENCE CHARACTERIZATION"
    );

    Serial.println(
        "=================================================="
    );

    Serial.println(
        "Reference scan order: Forward"
    );

    Serial.printf(
        "Reference drive settling:   %lu us\n",
        static_cast<unsigned long>(
            REFERENCE_DRIVE_SETTLE_US
        )
    );

    Serial.printf(
        "Reference release settling: %lu us\n",
        static_cast<unsigned long>(
            REFERENCE_RELEASE_SETTLE_US
        )
    );

    Serial.printf(
        "Reference scans:             %lu\n",
        static_cast<unsigned long>(
            REFERENCE_SCANS
        )
    );

    Serial.println();

    Serial.print("Candidate reference map: ");

    printMap(result.referenceMap);

    Serial.println();

    Serial.printf(
        "Reference scans agreeing: %lu / %lu\n",
        static_cast<unsigned long>(
            result.scansCompleted -
            result.disagreeingScans
        ),
        static_cast<unsigned long>(
            result.scansCompleted
        )
    );

    if (result.valid)
    {
        Serial.println("Reference status: VALID");
        return;
    }

    Serial.println("Reference status: INVALID");

    Serial.printf(
        "Disagreeing scans: %lu\n",
        static_cast<unsigned long>(
            result.disagreeingScans
        )
    );

    if (result.firstDisagreementScan != 0)
    {
        Serial.printf(
            "First disagreement at scan: %lu\n",
            static_cast<unsigned long>(
                result.firstDisagreementScan
            )
        );

        Serial.print("Conflicting map:          ");

        printMap(result.firstDisagreementMap);

        Serial.println();

        Serial.print("Difference mask:          ");

        printMap(
            result.referenceMap ^
            result.firstDisagreementMap
        );

        Serial.println();
    }
}


// ============================================================
// Test statistics
// ============================================================

struct TestResults
{
    uint32_t scans = 0;

    uint32_t incorrectScans = 0;

    uint32_t scansWithFalsePositives = 0;
    uint32_t scansWithFalseNegatives = 0;

    uint32_t falsePositiveBits[PAIR_COUNT] = {};
    uint32_t falseNegativeBits[PAIR_COUNT] = {};

    uint32_t firstErrorScan = 0;
    uint32_t firstErrorMap = 0;

    uint64_t elapsedUs = 0;
};


// ============================================================
// Run one order/delay condition
// ============================================================

TestResults runTest(
    uint32_t referenceMap,
    const ScanOrder& order,
    uint32_t driveSettleUs
)
{
    TestResults result;

    // Warm up execution paths.
    for (uint32_t i = 0; i < 100; ++i)
    {
        (void)scanContinuity(
            order,
            driveSettleUs,
            EXPERIMENT_RELEASE_SETTLE_US
        );
    }

    const uint32_t startUs = micros();

    for (
        uint32_t scan = 1;
        scan <= SCANS_PER_CONDITION;
        ++scan
    )
    {
        const uint32_t observed =
            scanContinuity(
                order,
                driveSettleUs,
                EXPERIMENT_RELEASE_SETTLE_US
            );

        ++result.scans;

        if (observed == referenceMap)
        {
            continue;
        }

        ++result.incorrectScans;

        if (result.firstErrorScan == 0)
        {
            result.firstErrorScan = scan;
            result.firstErrorMap = observed;
        }

        const uint32_t falsePositive =
            observed & ~referenceMap;

        const uint32_t falseNegative =
            referenceMap & ~observed;

        if (falsePositive != 0)
        {
            ++result.scansWithFalsePositives;
        }

        if (falseNegative != 0)
        {
            ++result.scansWithFalseNegatives;
        }

        for (uint8_t bit = 0; bit < PAIR_COUNT; ++bit)
        {
            const uint32_t mask =
                uint32_t{1} << bit;

            if ((falsePositive & mask) != 0)
            {
                ++result.falsePositiveBits[bit];
            }

            if ((falseNegative & mask) != 0)
            {
                ++result.falseNegativeBits[bit];
            }
        }
    }

    result.elapsedUs =
        static_cast<uint32_t>(
            micros() - startUs
        );

    return result;
}


// ============================================================
// Detailed result reporting
// ============================================================

void printResults(
    uint32_t referenceMap,
    const ScanOrder& order,
    uint32_t driveSettleUs,
    const TestResults& result
)
{
    const double usPerScan =
        static_cast<double>(result.elapsedUs) /
        static_cast<double>(result.scans);

    const double scansPerSecond =
        1000000.0 / usPerScan;

    Serial.println();
    Serial.println(
        "--------------------------------------------------"
    );

    Serial.printf(
        "Scan order: %s\n",
        order.name
    );

    Serial.printf(
        "Drive settling: %lu us\n",
        static_cast<unsigned long>(
            driveSettleUs
        )
    );

    Serial.printf(
        "Release settling: %lu us\n",
        static_cast<unsigned long>(
            EXPERIMENT_RELEASE_SETTLE_US
        )
    );

    Serial.printf(
        "Scans: %lu\n",
        static_cast<unsigned long>(
            result.scans
        )
    );

    Serial.printf(
        "Incorrect scans: %lu\n",
        static_cast<unsigned long>(
            result.incorrectScans
        )
    );

    Serial.printf(
        "Scans with false positives: %lu\n",
        static_cast<unsigned long>(
            result.scansWithFalsePositives
        )
    );

    Serial.printf(
        "Scans with false negatives: %lu\n",
        static_cast<unsigned long>(
            result.scansWithFalseNegatives
        )
    );

    Serial.printf(
        "Average scan time: %.3f us\n",
        usPerScan
    );

    Serial.printf(
        "Scan rate: %.1f scans/sec\n",
        scansPerSecond
    );

    if (result.incorrectScans == 0)
    {
        Serial.println("Result: PASS");
        return;
    }

    Serial.println("Result: FAIL");

    Serial.printf(
        "First error at scan: %lu\n",
        static_cast<unsigned long>(
            result.firstErrorScan
        )
    );

    Serial.print("Reference map:       ");

    printMap(referenceMap);

    Serial.println();

    Serial.print("First erroneous map: ");

    printMap(result.firstErrorMap);

    Serial.println();

    Serial.print("Difference mask:     ");

    printMap(
        referenceMap ^
        result.firstErrorMap
    );

    Serial.println();

    Serial.println();
    Serial.println("Per-bit errors:");

    Serial.println(
        "Bit  Pair   False+   False-"
    );

    for (uint8_t bit = 0; bit < PAIR_COUNT; ++bit)
    {
        if (
            result.falsePositiveBits[bit] == 0 &&
            result.falseNegativeBits[bit] == 0
        )
        {
            continue;
        }

        Serial.printf(
            "%2u   %-5s  %7lu  %7lu\n",
            bit,
            pairNames[bit],
            static_cast<unsigned long>(
                result.falsePositiveBits[bit]
            ),
            static_cast<unsigned long>(
                result.falseNegativeBits[bit]
            )
        );
    }
}


// ============================================================
// Summary reporting
// ============================================================

void printSummaryHeader()
{
    Serial.println();
    Serial.println(
        "=================================================="
    );

    Serial.println("SUMMARY");

    Serial.println(
        "=================================================="
    );

    Serial.println(
        "Order,Drive_us,Release_us,Incorrect,"
        "FalsePosScans,FalseNegScans,AvgScan_us"
    );
}


void printSummaryRow(
    const ScanOrder& order,
    uint32_t driveSettleUs,
    const TestResults& result
)
{
    const double usPerScan =
        static_cast<double>(result.elapsedUs) /
        static_cast<double>(result.scans);

    Serial.printf(
        "%s,%lu,%lu,%lu,%lu,%lu,%.3f\n",
        order.name,
        static_cast<unsigned long>(
            driveSettleUs
        ),
        static_cast<unsigned long>(
            EXPERIMENT_RELEASE_SETTLE_US
        ),
        static_cast<unsigned long>(
            result.incorrectScans
        ),
        static_cast<unsigned long>(
            result.scansWithFalsePositives
        ),
        static_cast<unsigned long>(
            result.scansWithFalseNegatives
        ),
        usPerScan
    );
}


// ============================================================
// Arduino setup
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);

    idleAllLines();

    Serial.println();
    Serial.println(
        "Sentinel Experiment 07"
    );

    Serial.println(
        "External pull-up characterization"
    );

    Serial.println();

    printPinConfiguration();

    Serial.println();
    Serial.println("Configured scan orders:");

    for (size_t i = 0; i < ORDER_COUNT; ++i)
    {
        printScanOrder(scanOrders[i]);
    }

    // --------------------------------------------------------
    // Validate the experiment definition
    // --------------------------------------------------------

    if (!validateAllScanOrders())
    {
        Serial.println();
        Serial.println(
            "EXPERIMENT ABORTED: invalid scan order."
        );

        idleAllLines();
        return;
    }

    // --------------------------------------------------------
    // Establish trusted reference
    // --------------------------------------------------------

    const ReferenceResult reference =
        establishReference();

    printReferenceResult(reference);

    if (!reference.valid)
    {
        Serial.println();
        Serial.println(
            "=================================================="
        );

        Serial.println(
            "CHARACTERIZATION ABORTED"
        );

        Serial.println(
            "Reference topology was not stable."
        );

        idleAllLines();
        return;
    }

    // --------------------------------------------------------
    // Characterize scan order
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "=================================================="
    );

    Serial.println(
        "SCAN-ORDER CHARACTERIZATION"
    );

    Serial.println(
        "=================================================="
    );

    Serial.print("Trusted reference map: ");

    printMap(reference.referenceMap);

    Serial.println();

    Serial.printf(
        "Scans per condition: %lu\n",
        static_cast<unsigned long>(
            SCANS_PER_CONDITION
        )
    );

    Serial.printf(
        "Experimental release settling: %lu us\n",
        static_cast<unsigned long>(
            EXPERIMENT_RELEASE_SETTLE_US
        )
    );

    // 3 orders x 4 delays = 12 result sets.
    TestResults results[ORDER_COUNT][DELAY_COUNT];

    for (size_t orderIndex = 0;
         orderIndex < ORDER_COUNT;
         ++orderIndex)
    {
        const ScanOrder& order =
            scanOrders[orderIndex];

        for (size_t delayIndex = 0;
             delayIndex < DELAY_COUNT;
             ++delayIndex)
        {
            const uint32_t driveSettleUs =
                driveSettlingDelaysUs[delayIndex];

            results[orderIndex][delayIndex] =
                runTest(
                    reference.referenceMap,
                    order,
                    driveSettleUs
                );

            printResults(
                reference.referenceMap,
                order,
                driveSettleUs,
                results[orderIndex][delayIndex]
            );
        }
    }

    // --------------------------------------------------------
    // Compact comparison table
    // --------------------------------------------------------

    printSummaryHeader();

    for (size_t orderIndex = 0;
         orderIndex < ORDER_COUNT;
         ++orderIndex)
    {
        for (size_t delayIndex = 0;
             delayIndex < DELAY_COUNT;
             ++delayIndex)
        {
            printSummaryRow(
                scanOrders[orderIndex],
                driveSettlingDelaysUs[delayIndex],
                results[orderIndex][delayIndex]
            );
        }
    }

    idleAllLines();

    Serial.println();
    Serial.println(
        "=================================================="
    );

    Serial.println(
        "Experiment 07 complete."
    );
}


// ============================================================
// Arduino loop
// ============================================================

void loop()
{
    // Experiment runs once after reset.
}
