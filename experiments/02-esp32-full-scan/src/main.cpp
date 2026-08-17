#include <Arduino.h>

// ============================================================
// Sentinel ESP32 Full-Scan Experiment
//
// Purpose:
//   Exercise the complete six-phase / 21-pair continuity scan
//   using ordinary Arduino GPIO operations and NO explicit
//   settling delay.
//
// Measurement method:
//   - All lines idle as INPUT_PULLUP.
//   - One line at a time is changed to OUTPUT LOW.
//   - A sensed LOW means continuity with the driven line.
//   - The driven line is then returned to INPUT_PULLUP.
//
// This is experimental characterization code, not production
// Sentinel firmware.
// ============================================================

// ------------------------------------------------------------
// Experimental GPIO mapping
// ------------------------------------------------------------

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

// ------------------------------------------------------------
// Canonical 21-bit layout
//
//  0  RA-RB
//  1  RA-RC
//  2  RA-MT
//  3  RA-GC
//  4  RA-GB
//  5  RA-GA
//  6  RB-RC
//  7  RB-MT
//  8  RB-GC
//  9  RB-GB
// 10  RB-GA
// 11  RC-MT
// 12  RC-GC
// 13  RC-GB
// 14  RC-GA
// 15  MT-GC
// 16  MT-GB
// 17  MT-GA
// 18  GC-GB
// 19  GC-GA
// 20  GB-GA
// ------------------------------------------------------------

constexpr uint8_t firstBit[6] =
    {
        0,  // RA
        6,  // RB
        11, // RC
        15, // MT
        18, // GC
        20  // GB
};

// Number of sense lines remaining for each driven line.
constexpr uint8_t senseCount[6] =
    {
        6,
        5,
        4,
        3,
        2,
        1};

// ------------------------------------------------------------
// Experiment configuration
// ------------------------------------------------------------

// Number of scans used for timing measurement.
constexpr uint32_t TIMING_SCANS = 100000;

// Number of scans used for stability testing.
constexpr uint32_t STABILITY_SCANS = 1000000;

// There is intentionally NO delay between selecting a driven
// line and sampling the sense lines.
constexpr uint32_t SETTLE_US = 0;

// ------------------------------------------------------------
// GPIO handling
// ------------------------------------------------------------

void idleAllLines()
{
    for (size_t i = 0; i < LINE_COUNT; ++i)
    {
        pinMode(pins[i], INPUT_PULLUP);
    }
}

void beginMeasurement(uint8_t driveIndex)
{
    const uint8_t pin = pins[driveIndex];

    // Match the historical Sentinel/Tarsier measurement method.
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    if constexpr (SETTLE_US > 0)
    {
        delayMicroseconds(SETTLE_US);
    }
}

void endMeasurement(uint8_t driveIndex)
{
    const uint8_t pin = pins[driveIndex];

    // Return the line to its normal idle state.
    pinMode(pin, INPUT_PULLUP);
    digitalWrite(pin, HIGH);
}

// ------------------------------------------------------------
// Full 21-pair scan
// ------------------------------------------------------------

uint32_t scanContinuity()
{
    uint32_t map = 0;

    // Six driven-line phases.
    // GA never needs to be driven because GB-GA is measured
    // during the GB phase.
    for (uint8_t drive = 0; drive < 6; ++drive)
    {
        beginMeasurement(drive);

        uint8_t bit = firstBit[drive];

        for (
            uint8_t sense = drive + 1;
            sense < LINE_COUNT;
            ++sense, ++bit)
        {
            // Active-low measurement:
            //
            // LOW  -> continuity
            // HIGH -> no continuity
            //
            if (digitalRead(pins[sense]) == LOW)
            {
                map |= (uint32_t{1} << bit);
            }
        }

        endMeasurement(drive);
    }

    return map;
}

// ------------------------------------------------------------
// Diagnostic output
// ------------------------------------------------------------

void printBinary21(uint32_t value)
{
    for (int bit = 20; bit >= 0; --bit)
    {
        Serial.print(
            (value & (uint32_t{1} << bit)) ? '1' : '0');
    }
}

void printMap(uint32_t map)
{
    Serial.print("0x");
    Serial.printf("%06lX", static_cast<unsigned long>(map));
    Serial.print("  ");
    printBinary21(map);
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

// ------------------------------------------------------------
// Timing characterization
// ------------------------------------------------------------

void runTimingTest()
{
    Serial.println();
    Serial.println("=== Scan timing ===");

    // Warm up the GPIO paths before timing.
    for (uint32_t i = 0; i < 1000; ++i)
    {
        (void)scanContinuity();
    }

    const uint32_t startUs = micros();

    for (uint32_t i = 0; i < TIMING_SCANS; ++i)
    {
        (void)scanContinuity();
    }

    const uint32_t elapsedUs = micros() - startUs;

    const double usPerScan =
        static_cast<double>(elapsedUs) /
        static_cast<double>(TIMING_SCANS);

    const double scansPerSecond =
        1000000.0 / usPerScan;

    Serial.printf(
        "Scans: %lu\n",
        static_cast<unsigned long>(TIMING_SCANS));

    Serial.printf(
        "Elapsed: %lu us\n",
        static_cast<unsigned long>(elapsedUs));

    Serial.printf(
        "Average: %.3f us/scan\n",
        usPerScan);

    Serial.printf(
        "Rate: %.1f scans/second\n",
        scansPerSecond);
}

// ------------------------------------------------------------
// Stability characterization
// ------------------------------------------------------------

void runStabilityTest()
{
    Serial.println();
    Serial.println("=== Stability test ===");

    // Whatever topology is physically connected when this
    // function begins becomes the expected topology.
    //
    // This experiment therefore tests whether repeated scans
    // remain identical under a static physical configuration.
    const uint32_t expected = scanContinuity();

    Serial.print("Expected map: ");
    printMap(expected);
    Serial.println();

    uint32_t errors = 0;

    uint32_t firstErrorScan = 0;
    uint32_t firstErrorMap = 0;

    for (uint32_t scan = 1; scan <= STABILITY_SCANS; ++scan)
    {
        const uint32_t observed = scanContinuity();

        if (observed != expected)
        {
            ++errors;

            if (errors == 1)
            {
                firstErrorScan = scan;
                firstErrorMap = observed;
            }
        }
    }

    Serial.printf(
        "Scans: %lu\n",
        static_cast<unsigned long>(STABILITY_SCANS));

    Serial.printf(
        "Errors: %lu\n",
        static_cast<unsigned long>(errors));

    if (errors == 0)
    {
        Serial.println("Result: PASS");
    }
    else
    {
        Serial.println("Result: FAIL");

        Serial.printf(
            "First error at scan: %lu\n",
            static_cast<unsigned long>(firstErrorScan));

        Serial.print("First erroneous map: ");
        printMap(firstErrorMap);
        Serial.println();

        Serial.print("Difference mask:    ");
        printMap(expected ^ firstErrorMap);
        Serial.println();
    }
}

// ------------------------------------------------------------
// Arduino entry points
// ------------------------------------------------------------

void setup()
{
    Serial.begin(115200);

    // Give the serial monitor time to connect.
    delay(2000);

    idleAllLines();

    Serial.println();
    Serial.println(
        "Sentinel ESP32 full continuity scan experiment");

    Serial.printf(
        "Explicit settling delay: %lu us\n",
        static_cast<unsigned long>(SETTLE_US));

    printPinConfiguration();

    const uint32_t initialMap = scanContinuity();

    Serial.println();
    Serial.print("Initial map: ");
    printMap(initialMap);
    Serial.println();

    runTimingTest();
    runStabilityTest();

    idleAllLines();

    Serial.println();
    Serial.println("Experiment complete.");
}

void loop()
{
    // Experiment runs once after reset.
}
