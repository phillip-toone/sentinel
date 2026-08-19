#include <Arduino.h>

// ============================================================
// Sentinel Experiment 04
// ESP32 Drive vs. Release Settling Characterization
//
// Purpose:
//   Determine whether the settling time required by Sentinel's
//   complete six-phase / 21-pair continuity scan belongs:
//
//     1. after driving a line LOW,
//     2. after releasing a driven line,
//     3. or in both places.
//
// Reference strategy:
//   Establish a trusted topology using deliberately conservative
//   timing:
//
//       1000 us drive settling
//       1000 us release settling
//       1000 unanimous reference scans required
//
// Measurement method:
//   - All lines idle as INPUT_PULLUP.
//   - One line at a time becomes OUTPUT LOW.
//   - Wait drive-settling interval.
//   - Sample appropriate remaining lines.
//   - Return driven line to INPUT_PULLUP.
//   - Wait release-settling interval.
//   - Continue with next phase.
// ============================================================

// ============================================================
// GPIO mapping - TTGO T-Display V1.1 experimental mapping
// ============================================================

constexpr uint8_t PIN_RA = 21;
constexpr uint8_t PIN_RB = 22;
constexpr uint8_t PIN_RC = 17;
constexpr uint8_t PIN_MT = 32;
constexpr uint8_t PIN_GC = 25;
constexpr uint8_t PIN_GB = 26;
constexpr uint8_t PIN_GA = 27;

constexpr size_t LINE_COUNT = 7;

constexpr uint8_t pins[LINE_COUNT] =
    {
        PIN_RA,
        PIN_RB,
        PIN_RC,
        PIN_MT,
        PIN_GC,
        PIN_GB,
        PIN_GA};

constexpr const char *lineNames[LINE_COUNT] =
    {
        "RA",
        "RB",
        "RC",
        "MT",
        "GC",
        "GB",
        "GA"};

// ============================================================
// Canonical 21-bit representation
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

constexpr size_t PAIR_COUNT = 21;

constexpr const char *pairNames[PAIR_COUNT] =
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

        "GB-GA"};

// ============================================================
// Reference configuration
// ============================================================
//
// Experiment 04 is explicitly testing whether release settling
// matters.
//
// Therefore the trusted reference must not assume that zero
// release settling is safe.
//
// Both sides of the measurement phase receive a deliberately
// conservative 1 ms interval.
// ============================================================

constexpr uint32_t REFERENCE_DRIVE_SETTLE_US = 1000;
constexpr uint32_t REFERENCE_RELEASE_SETTLE_US = 1000;
constexpr uint32_t REFERENCE_SCANS = 1000;

// ============================================================
// Characterization configuration
// ============================================================

constexpr uint32_t SCANS_PER_CONDITION = 10000;

// ============================================================
// Test conditions
// ============================================================
//
// Three conceptual groups:
//
//   A. Drive settling only
//   B. Release settling only
//   C. Equal drive and release settling
//
// Duplicate 0/0 conditions are intentionally eliminated.
// ============================================================

struct TestCondition
{
    const char *group;
    uint32_t driveSettleUs;
    uint32_t releaseSettleUs;
};

constexpr TestCondition testConditions[] =
    {
        // Baseline
        {"Baseline", 0, 0},

        // A - Drive settling only
        {"Drive only", 10, 0},
        {"Drive only", 20, 0},
        {"Drive only", 50, 0},
        {"Drive only", 100, 0},

        // B - Release settling only
        {"Release only", 0, 10},
        {"Release only", 0, 20},
        {"Release only", 0, 50},
        {"Release only", 0, 100},

        // C - Equal drive and release settling
        {"Equal", 10, 10},
        {"Equal", 20, 20},
        {"Equal", 50, 50},
        {"Equal", 100, 100}};

constexpr size_t CONDITION_COUNT =
    sizeof(testConditions) /
    sizeof(testConditions[0]);

// ============================================================
// GPIO handling
// ============================================================

void idleAllLines()
{
    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        pinMode(pins[i], INPUT_PULLUP);
    }
}

void beginMeasurement(
    uint8_t driveIndex,
    uint32_t driveSettleUs)
{
    const uint8_t pin = pins[driveIndex];

    // Historical active-low continuity measurement.
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    if (driveSettleUs > 0)
    {
        delayMicroseconds(driveSettleUs);
    }
}

void endMeasurement(
    uint8_t driveIndex,
    uint32_t releaseSettleUs)
{
    const uint8_t pin = pins[driveIndex];

    // Return the driven line to its idle state.
    pinMode(pin, INPUT_PULLUP);
    digitalWrite(pin, HIGH);

    if (releaseSettleUs > 0)
    {
        delayMicroseconds(releaseSettleUs);
    }
}

// ============================================================
// Full six-phase / 21-pair scan
// ============================================================

uint32_t scanContinuity(
    uint32_t driveSettleUs,
    uint32_t releaseSettleUs)
{
    uint32_t map = 0;
    uint8_t bit = 0;

    // Drive RA through GB.
    //
    // GA never needs to be driven because every pair involving
    // GA has already been measured during an earlier phase.

    for (uint8_t drive = 0; drive < 6; ++drive)
    {
        beginMeasurement(
            drive,
            driveSettleUs);

        for (
            uint8_t sense = drive + 1;
            sense < LINE_COUNT;
            ++sense)
        {
            // Active-low:
            //
            // LOW  = continuity
            // HIGH = no continuity

            if (digitalRead(pins[sense]) == LOW)
            {
                map |= (uint32_t{1} << bit);
            }

            ++bit;
        }

        endMeasurement(
            drive,
            releaseSettleUs);
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
                : '0');
    }
}

void printMap(uint32_t value)
{
    Serial.printf(
        "0x%06lX  ",
        static_cast<unsigned long>(value));

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
            pins[i]);
    }
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

    // Give the complete external network time to reach a
    // quiescent idle state before establishing the reference.
    delay(100);

    result.referenceMap =
        scanContinuity(
            REFERENCE_DRIVE_SETTLE_US,
            REFERENCE_RELEASE_SETTLE_US);

    result.scansCompleted = 1;

    for (
        uint32_t scan = 2;
        scan <= REFERENCE_SCANS;
        ++scan)
    {
        const uint32_t observed =
            scanContinuity(
                REFERENCE_DRIVE_SETTLE_US,
                REFERENCE_RELEASE_SETTLE_US);

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
    const ReferenceResult &result)
{
    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "REFERENCE CHARACTERIZATION");

    Serial.println(
        "==================================================");

    Serial.printf(
        "Reference drive settling:   %lu us\n",
        static_cast<unsigned long>(
            REFERENCE_DRIVE_SETTLE_US));

    Serial.printf(
        "Reference release settling: %lu us\n",
        static_cast<unsigned long>(
            REFERENCE_RELEASE_SETTLE_US));

    Serial.printf(
        "Reference scans:             %lu\n",
        static_cast<unsigned long>(
            REFERENCE_SCANS));

    Serial.println();

    Serial.print("Candidate reference map: ");

    printMap(result.referenceMap);

    Serial.println();

    Serial.printf(
        "Reference scans agreeing: %lu / %lu\n",
        static_cast<unsigned long>(
            result.scansCompleted -
            result.disagreeingScans),
        static_cast<unsigned long>(
            result.scansCompleted));

    if (result.valid)
    {
        Serial.println("Reference status: VALID");
        return;
    }

    Serial.println("Reference status: INVALID");

    Serial.println();

    Serial.printf(
        "Disagreeing scans: %lu\n",
        static_cast<unsigned long>(
            result.disagreeingScans));

    Serial.printf(
        "First disagreement at scan: %lu\n",
        static_cast<unsigned long>(
            result.firstDisagreementScan));

    Serial.print("Conflicting map:          ");

    printMap(result.firstDisagreementMap);

    Serial.println();

    Serial.print("Difference mask:          ");

    printMap(
        result.referenceMap ^
        result.firstDisagreementMap);

    Serial.println();
}

// ============================================================
// Characterization statistics
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
// Run one drive/release condition
// ============================================================

TestResults runTest(
    uint32_t referenceMap,
    uint32_t driveSettleUs,
    uint32_t releaseSettleUs)
{
    TestResults result;

    // Warm up execution paths before timing.
    for (uint32_t i = 0; i < 100; ++i)
    {
        (void)scanContinuity(
            driveSettleUs,
            releaseSettleUs);
    }

    const uint32_t startUs = micros();

    for (
        uint32_t scan = 1;
        scan <= SCANS_PER_CONDITION;
        ++scan)
    {
        const uint32_t observed =
            scanContinuity(
                driveSettleUs,
                releaseSettleUs);

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

        // Measured continuity that should not exist.
        const uint32_t falsePositive =
            observed & ~referenceMap;

        // Expected continuity that was not measured.
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
            micros() - startUs);

    return result;
}

// ============================================================
// Report one drive/release condition
// ============================================================

void printResults(
    uint32_t referenceMap,
    const TestCondition &condition,
    const TestResults &result)
{
    const double usPerScan =
        static_cast<double>(result.elapsedUs) /
        static_cast<double>(result.scans);

    const double scansPerSecond =
        1000000.0 / usPerScan;

    Serial.println();
    Serial.println(
        "--------------------------------------------------");

    Serial.printf(
        "Group: %s\n",
        condition.group);

    Serial.printf(
        "Drive settling:   %lu us\n",
        static_cast<unsigned long>(
            condition.driveSettleUs));

    Serial.printf(
        "Release settling: %lu us\n",
        static_cast<unsigned long>(
            condition.releaseSettleUs));

    Serial.printf(
        "Scans: %lu\n",
        static_cast<unsigned long>(
            result.scans));

    Serial.printf(
        "Incorrect scans: %lu\n",
        static_cast<unsigned long>(
            result.incorrectScans));

    Serial.printf(
        "Scans with false positives: %lu\n",
        static_cast<unsigned long>(
            result.scansWithFalsePositives));

    Serial.printf(
        "Scans with false negatives: %lu\n",
        static_cast<unsigned long>(
            result.scansWithFalseNegatives));

    Serial.printf(
        "Average scan time: %.3f us\n",
        usPerScan);

    Serial.printf(
        "Scan rate: %.1f scans/sec\n",
        scansPerSecond);

    if (result.incorrectScans == 0)
    {
        Serial.println("Result: PASS");
        return;
    }

    Serial.println("Result: FAIL");

    Serial.printf(
        "First error at scan: %lu\n",
        static_cast<unsigned long>(
            result.firstErrorScan));

    Serial.print("Reference map:       ");

    printMap(referenceMap);

    Serial.println();

    Serial.print("First erroneous map: ");

    printMap(result.firstErrorMap);

    Serial.println();

    Serial.print("Difference mask:     ");

    printMap(
        referenceMap ^
        result.firstErrorMap);

    Serial.println();

    Serial.println();
    Serial.println("Per-bit errors:");

    Serial.println(
        "Bit  Pair   False+   False-");

    for (uint8_t bit = 0; bit < PAIR_COUNT; ++bit)
    {
        if (
            result.falsePositiveBits[bit] == 0 &&
            result.falseNegativeBits[bit] == 0)
        {
            continue;
        }

        Serial.printf(
            "%2u   %-5s  %7lu  %7lu\n",
            bit,
            pairNames[bit],
            static_cast<unsigned long>(
                result.falsePositiveBits[bit]),
            static_cast<unsigned long>(
                result.falseNegativeBits[bit]));
    }
}

// ============================================================
// Compact summary table
// ============================================================

void printSummaryHeader()
{
    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "SUMMARY");

    Serial.println(
        "==================================================");

    Serial.println(
        "Group,Drive_us,Release_us,Incorrect,FalsePosScans,"
        "FalseNegScans,AvgScan_us");
}

void printSummaryRow(
    const TestCondition &condition,
    const TestResults &result)
{
    const double usPerScan =
        static_cast<double>(result.elapsedUs) /
        static_cast<double>(result.scans);

    Serial.printf(
        "%s,%lu,%lu,%lu,%lu,%lu,%.3f\n",
        condition.group,
        static_cast<unsigned long>(
            condition.driveSettleUs),
        static_cast<unsigned long>(
            condition.releaseSettleUs),
        static_cast<unsigned long>(
            result.incorrectScans),
        static_cast<unsigned long>(
            result.scansWithFalsePositives),
        static_cast<unsigned long>(
            result.scansWithFalseNegatives),
        usPerScan);
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
        "Sentinel Experiment 04");

    Serial.println(
        "ESP32 drive vs. release settling characterization");

    Serial.println();

    printPinConfiguration();

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
            "==================================================");

        Serial.println(
            "CHARACTERIZATION ABORTED");

        Serial.println(
            "Reference topology was not stable using "
            "the conservative drive/release timing.");

        idleAllLines();

        return;
    }

    // --------------------------------------------------------
    // Characterization
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "DRIVE / RELEASE CHARACTERIZATION");

    Serial.println(
        "==================================================");

    Serial.print("Trusted reference map: ");

    printMap(reference.referenceMap);

    Serial.println();

    Serial.printf(
        "Scans per condition: %lu\n",
        static_cast<unsigned long>(
            SCANS_PER_CONDITION));

    // Store results so we can print a compact summary after
    // the detailed reports.
    TestResults results[CONDITION_COUNT];

    for (size_t i = 0; i < CONDITION_COUNT; ++i)
    {
        results[i] =
            runTest(
                reference.referenceMap,
                testConditions[i].driveSettleUs,
                testConditions[i].releaseSettleUs);

        printResults(
            reference.referenceMap,
            testConditions[i],
            results[i]);
    }

    // --------------------------------------------------------
    // Compact machine-readable/human-readable summary
    // --------------------------------------------------------

    printSummaryHeader();

    for (size_t i = 0; i < CONDITION_COUNT; ++i)
    {
        printSummaryRow(
            testConditions[i],
            results[i]);
    }

    idleAllLines();

    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "Experiment 04 complete.");
}

// ============================================================
// Arduino loop
// ============================================================

void loop()
{
    // Experiment runs once after reset.
}
