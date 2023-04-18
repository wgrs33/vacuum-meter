# Commands

## Configure

`
mkdir _build
cd _build/
cmake -DAUTO_SET_SKETCHBOOK_PATH=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
`

## Build

`
cd _build
cmake --build .
`

## Flash

`
cd _build
make VacuumMeter_flash
`
