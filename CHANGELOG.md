# Changelog

## Develop

- Split CMakeLists.txt files between library and executable
- Change license year to 2022
- Timeout: Module returns ERRMEM if no memory to allocate block
- MQTT: update client to be consistent with client from LwESP library
- Port: Add ThreadX port aligned with LwESP library
- Add optional keep-alive system period event
- Add `LWGSM ` prefix for debug messages
- Update code style with astyle
- Add `.clang-format` draft
- Delete `lwgsm_datetime_t` and use generic `struct tm` instead
- Rename project from `lwgsm` to `lwcell`, indicating cellular
- Rework library CMake with removed INTERFACE type

## v0.1.1

- Update CMSIS OS driver to support FreeRTOS aware kernel
- Update to support library.json for Platform.IO

## v0.1.0

- First release
- Support for SIM800/SIM900 for the moment
- Added AT commands as per documentation
- Sequential API for network supported
