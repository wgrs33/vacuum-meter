# Commands

## Configure

`
cmake -DCMAKE_TOOLCHAIN_FILE=arduino-cmake/cmake/Arduino-Toolchain.cmake -DAUTO_SET_SKETCHBOOK_PATH=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
`

## Build

`
cmake --build .
`

## Flash

`
make VacuumMeter_flash
`
