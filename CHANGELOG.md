# Changelog

## Develop

- Split CMakeLists.txt files between library and executable
- Change license year to 2022
- Timeout: Module returns ERRMEM if no memory to allocate block
- MQTT: update client to be consistent with client from LwESP library
- Port: Add ThreadX port aligned with LwESP library
- Add optional keep-alive system period event
- Add `LWGSM ` prefix for debug messages

## v0.1.1

- Update CMSIS OS driver to support FreeRTOS aware kernel
- Update to support library.json for Platform.IO

## v0.1.0

- First release
- Support for SIM800/SIM900 for the moment
- Added AT commands as per documentation
- Sequential API for network supported
