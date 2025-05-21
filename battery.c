#include "battery.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

typedef struct {
    int32_t percent;
    int32_t voltageMv;
} BatteryLevel;

#define SAMPLE_COUNT 10
#define ADC_MAX 4095
#define VREF_MV 3300
#define VOLTAGE_DIVIDER_RATIO 2 // 100k / 100k resistors

#define ELEMENTS(x) (sizeof(x) / sizeof((x)[0]))

static int32_t lastVoltageMv = 0;
static uint8_t lastPercent = 0;

static const BatteryLevel batteryCurve[] = {
    {0,   3312},
    {10,  3670},
    {30,  3754},
    {50,  3795},
    {70,  3905},
    {90,  4043},
    {100, 4150},
};

static int32_t interpolate(BatteryLevel startPoint, BatteryLevel endPoint, int32_t currentVoltageMv)
{
    int32_t deltaPercent = endPoint.percent - startPoint.percent;
    int32_t deltaVoltage = endPoint.voltageMv - startPoint.voltageMv;
    int32_t offset = currentVoltageMv - startPoint.voltageMv;

    return startPoint.percent + (deltaPercent * offset) / deltaVoltage;
}

void batteryInit()
{
    adc_init();
    adc_gpio_init(BATTERY_ADC_PIN);
    adc_select_input(BATTERY_ADC_CHANNEL);
}

int32_t batteryGetPercent()
{
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += adc_read();
        sleep_ms(2);
    }

    uint32_t avgAdc = sum / SAMPLE_COUNT;

    // Convert to mV: adc * VREF / 4095 * (divider ratio)
    // voltageMv = (adc * 3300 / 4095) * 2
    int32_t adcMv = (avgAdc * VREF_MV) / ADC_MAX;
    int32_t batteryVoltageMv = adcMv * VOLTAGE_DIVIDER_RATIO;

    lastVoltageMv = batteryVoltageMv;

    if (batteryVoltageMv <= batteryCurve[0].voltageMv) {
        return 0;
    }

    if (batteryVoltageMv >= batteryCurve[ELEMENTS(batteryCurve) - 1].voltageMv) {
        return 100;
    }

    for (int i = 1; i < ELEMENTS(batteryCurve); i++) {
        if (batteryVoltageMv < batteryCurve[i].voltageMv) {
            lastPercent = interpolate(batteryCurve[i - 1], batteryCurve[i], batteryVoltageMv);
            return lastPercent;
        }
    }

    return 0;
}

int32_t batteryGetVoltageMv()
{
    return lastVoltageMv;
}
