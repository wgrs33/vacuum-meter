#pragma once

#include <Arduino.h>

constexpr int BUTTON = 8;
constexpr int VACCUM_1 = A1;
constexpr int VACCUM_2 = A0;

constexpr int BAR_MIN = 0;
constexpr int BAR_MAX = 70;

constexpr unsigned int VACCUM_AMIN = 41;
constexpr unsigned int VACCUM_AMAX = 962;
constexpr unsigned int VACCUM_VMIN = 150;
constexpr unsigned int VACCUM_VMAX = 1150;
constexpr int VACCUM_DMIN = -850;
constexpr int VACCUM_DMAX = 0;

constexpr double c_MMHG = 1.33322;
constexpr unsigned int ADC_SAMPLES = 10;

void show_menu();

void synchronization();

void pressure_diff();

void pressure_absolute();

void set_bar(int value);

void align_right(int value, int max);

void calibrate();