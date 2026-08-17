#include <Arduino.h>

//
// ESP32 continuity settling-time characterization
//
// DRIVE_PIN is actively pulled LOW during the test.
// SENSE_PIN remains INPUT_PULLUP.
//
// Connect DRIVE_PIN to SENSE_PIN through the electrical path
// being characterized.
//

constexpr uint8_t DRIVE_PIN = 18;
constexpr uint8_t SENSE_PIN = 19;

// Number of repetitions at each delay.
constexpr uint32_t TRIALS = 10000;

// Candidate settling delays in microseconds.
constexpr uint32_t delaysUs[] = {
    0,
    1,
    2,
    5,
    10,
    20,
    50,
    100,
    250,
    500,
    1000};

constexpr size_t delayCount =
    sizeof(delaysUs) / sizeof(delaysUs[0]);

void idlePins()
{
    pinMode(DRIVE_PIN, INPUT_PULLUP);
    pinMode(SENSE_PIN, INPUT_PULLUP);
}

void beginMeasurement()
{
    // Match the behavior of the original scoring apparatus.
    pinMode(DRIVE_PIN, OUTPUT);
    digitalWrite(DRIVE_PIN, LOW);
}

void endMeasurement()
{
    // Return the driven line to its normal idle state.
    pinMode(DRIVE_PIN, INPUT_PULLUP);
    digitalWrite(DRIVE_PIN, HIGH);
}

bool testSettleLow(uint32_t delayUs)
{
    idlePins();

    // Allow both lines to establish their idle HIGH state.
    delayMicroseconds(10);

    beginMeasurement();

    if (delayUs != 0)
    {
        delayMicroseconds(delayUs);
    }

    const bool correct =
        (digitalRead(SENSE_PIN) == LOW);

    endMeasurement();

    return correct;
}

bool testReleaseHigh(uint32_t delayUs)
{
    idlePins();

    delayMicroseconds(10);

    beginMeasurement();

    // Give the connected network plenty of time to reach LOW.
    delayMicroseconds(1000);

    endMeasurement();

    if (delayUs != 0)
    {
        delayMicroseconds(delayUs);
    }

    const bool correct =
        (digitalRead(SENSE_PIN) == HIGH);

    return correct;
}

void printHeader()
{
    Serial.println();
    Serial.println(
        "ESP32 continuity settling-time characterization");

    Serial.printf(
        "DRIVE_PIN=%u  SENSE_PIN=%u  TRIALS=%lu\n",
        DRIVE_PIN,
        SENSE_PIN,
        static_cast<unsigned long>(TRIALS));

    Serial.println();
    Serial.println(
        "Delay(us),LOW errors,HIGH errors");
}

void runTests()
{
    printHeader();

    for (size_t i = 0; i < delayCount; ++i)
    {
        const uint32_t delayUs = delaysUs[i];

        uint32_t lowErrors = 0;
        uint32_t highErrors = 0;

        for (uint32_t trial = 0; trial < TRIALS; ++trial)
        {
            if (!testSettleLow(delayUs))
            {
                ++lowErrors;
            }

            if (!testReleaseHigh(delayUs))
            {
                ++highErrors;
            }
        }

        Serial.printf(
            "%lu,%lu,%lu\n",
            static_cast<unsigned long>(delayUs),
            static_cast<unsigned long>(lowErrors),
            static_cast<unsigned long>(highErrors));
    }

    Serial.println();
    Serial.println("Test complete.");
}

void setup()
{
    Serial.begin(115200);

    delay(2000);

    idlePins();

    runTests();
}

void loop()
{
    // Tests run once after reset.
}
