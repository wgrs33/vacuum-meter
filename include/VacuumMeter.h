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

/// @brief Show user menu
void show_menu();

/// @brief Show synchronization status (2 strokes)
void synchronization();

/// @brief Show differential pressure
void pressure_diff();

/// @brief Show absolute pressure with mbar and mmHg
void pressure_absolute();

/// @brief Set bar graph value
/// @param value value in the range [0 - 70]
void set_bar(int value);

/// @brief Align text to right
/// @param value Value to show
/// @param max max length
void align_right(int value, int max);

/// @brief Calibrate inputs
/// @details Both inputs must not be connected to any source.
/// Atmospheric pressure is stored.
/// Delta is calculated between two inputs.
void calibrate();