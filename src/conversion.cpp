#include "conversion.h"

int convert_adc_kPa(int value) {
    int voltage = (value * 5000.0 / 1023.0);
    int kpa = (voltage / 45) + 15;

    return kpa;
}

int convert_kPa_PSI(int value) {
    // Convert kPa to PSI in x100
    long temp = (long)value;
    temp *= 100;
    temp /= 689;

    return (int)temp;
}

int convert_kPa_inHg(int value) {
    // Convert kPa to inHg in x100
    long temp = (long)value;
    temp *= 100;
    temp /= 339;

    return (int)temp;
}

int convert_kPa_mmHg(int value) {
    // Convert kPa to mmHg in x10
    long temp = (long)value;
    temp *= 75;
    temp /= 100;

    return (int)temp;
}