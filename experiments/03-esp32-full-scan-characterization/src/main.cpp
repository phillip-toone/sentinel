#include <Arduino.h>

// ============================================================
// Sentinel ESP32 Full-Scan Characterization
//
// Purpose:
//   Characterize reliability and performance of Sentinel's
//   complete six-phase / 21-pair continuity scan.
//
// Reference strategy:
//   Before testing short settling delays, establish the physical
//   topology using repeated scans with the historically proven
//   1 ms settling delay.
//
//   Every reference scan must agree.
//
//   If the reference is not stable, characterization is aborted.
//
// Measurement method:
//   - All lines idle as INPUT_PULLUP.
//   - One line at a time becomes OUTPUT LOW.
//   - A sensed LOW means continuity.
//   - The driven line returns to INPUT_PULLUP.
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
// The original scoring apparatus used:
//
//     delay(1);
//
// after selecting a driven line.
//
// We therefore use 1000 us as the conservative reference
// settling interval.
//
// The reference topology is accepted only if EVERY reference
// scan produces exactly the same 21-bit map.
// ============================================================

constexpr uint32_t REFERENCE_SETTLE_US = 1000;
constexpr uint32_t REFERENCE_SCANS = 1000;

// ============================================================
// Characterization configuration
// ============================================================
//
// This first pass is intended to locate the interesting settling
// region. Once the transition region is known, it can be tested
// much more heavily in a later experiment.
// ============================================================

constexpr uint32_t SCANS_PER_DELAY = 10000;

constexpr uint32_t settlingDelaysUs[] =
    {
        0,
        1,
        2,
        5,
        10,
        20,
        50,
        100,
        250,
        500};

constexpr size_t DELAY_COUNT =
    sizeof(settlingDelaysUs) /
    sizeof(settlingDelaysUs[0]);

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
    uint32_t settleUs)
{
    const uint8_t pin = pins[driveIndex];

    // Historical active-low measurement method.
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    if (settleUs > 0)
    {
        delayMicroseconds(settleUs);
    }
}

void endMeasurement(uint8_t driveIndex)
{
    const uint8_t pin = pins[driveIndex];

    // Return driven line to its idle state.
    pinMode(pin, INPUT_PULLUP);
    digitalWrite(pin, HIGH);
}

// ============================================================
// Full six-phase / 21-pair scan
// ============================================================

uint32_t scanContinuity(uint32_t settleUs)
{
    uint32_t map = 0;
    uint8_t bit = 0;

    // Drive RA through GB.
    //
    // GA never needs to be driven because every pair involving
    // GA has already been measured during an earlier phase.

    for (uint8_t drive = 0; drive < 6; ++drive)
    {
        beginMeasurement(drive, settleUs);

        for (
            uint8_t sense = drive + 1;
            sense < LINE_COUNT;
            ++sense)
        {
            // Active-low measurement:
            //
            // LOW  = continuity
            // HIGH = no continuity

            if (digitalRead(pins[sense]) == LOW)
            {
                map |= (uint32_t{1} << bit);
            }

            ++bit;
        }

        endMeasurement(drive);
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

    // Allow the physical system to sit in its idle state before
    // establishing the reference.
    idleAllLines();
    delay(100);

    // The first conservative scan becomes the candidate
    // reference. Unlike the previous experiment, it is NOT
    // immediately trusted.
    result.referenceMap =
        scanContinuity(REFERENCE_SETTLE_US);

    result.scansCompleted = 1;

    for (
        uint32_t scan = 2;
        scan <= REFERENCE_SCANS;
        ++scan)
    {
        const uint32_t observed =
            scanContinuity(REFERENCE_SETTLE_US);

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
        "Reference settling delay: %lu us\n",
        static_cast<unsigned long>(
            REFERENCE_SETTLE_US));

    Serial.printf(
        "Reference scans: %lu\n",
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
// Run one settling-delay characterization
// ============================================================

TestResults runTest(
    uint32_t referenceMap,
    uint32_t settleUs)
{
    TestResults result;

    // Warm up GPIO/framework execution paths.
    for (uint32_t i = 0; i < 100; ++i)
    {
        (void)scanContinuity(settleUs);
    }

    const uint32_t startUs = micros();

    for (
        uint32_t scan = 1;
        scan <= SCANS_PER_DELAY;
        ++scan)
    {
        const uint32_t observed =
            scanContinuity(settleUs);

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
// Report one settling-delay characterization
// ============================================================

void printResults(
    uint32_t referenceMap,
    uint32_t settleUs,
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
        "Settling delay: %lu us\n",
        static_cast<unsigned long>(settleUs));

    Serial.printf(
        "Scans: %lu\n",
        static_cast<unsigned long>(result.scans));

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
// Arduino setup
// ============================================================

void setup()
{
    Serial.begin(115200);

    delay(2000);

    idleAllLines();

    Serial.println();
    Serial.println(
        "Sentinel ESP32 full-scan characterization");

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
            "Reference topology was not stable at the "
            "historical 1 ms settling interval.");

        Serial.println(
            "Resolve the reference instability before "
            "testing shorter settling delays.");

        idleAllLines();

        return;
    }

    // --------------------------------------------------------
    // Characterize shorter settling intervals
    // --------------------------------------------------------

    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "SETTLING-TIME CHARACTERIZATION");

    Serial.println(
        "==================================================");

    Serial.print("Trusted reference map: ");

    printMap(reference.referenceMap);

    Serial.println();

    Serial.printf(
        "Scans per delay: %lu\n",
        static_cast<unsigned long>(
            SCANS_PER_DELAY));

    for (size_t i = 0; i < DELAY_COUNT; ++i)
    {
        const uint32_t settleUs =
            settlingDelaysUs[i];

        const TestResults result =
            runTest(
                reference.referenceMap,
                settleUs);

        printResults(
            reference.referenceMap,
            settleUs,
            result);
    }

    idleAllLines();

    Serial.println();
    Serial.println(
        "==================================================");

    Serial.println(
        "Characterization complete.");
}

// ============================================================
// Arduino loop
// ============================================================

void loop()
{
    // Experiment runs once after reset.
}
