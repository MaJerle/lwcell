# Lightweight Cellular AT parser

LwCELL is a lightweight, platform independent, cellular modem AT commands parser, targeting (as of today) communication with SIMCOM based modules SIM800/SIM900 or SIM70xx.
The module is written in C11 and is independent of the platform it runs on. Its main targets are embedded system devices like ARM Cortex-M, AVR, PIC and others, but can easily work under `Windows`, `Linux` or `MAC` environments.

[Open documentation](https://docs.majerle.eu/projects/lwcell/)

## Features

* Written in C (C11), compatible with `stdint.h` data types
* Supports `SIM800/SIM900 (2G)` and `SIM7070G (NB-IoT LTE)` modules
* Platform independent and very easy to port
    * Provided examples for ARM Cortex-M or Win32 platforms
* Allows different configurations to optimize user requirements
* Supports operating-system implementations with advanced inter-thread communication
    * Currently only OS mode is supported
    * 2 different threads handling user data and received data
        * First (producer) thread (collects user commands from user threads and starts the command processing)
        * Second (process) thread reads the data from GSM device and does the job accordingly
* Allows sequential API for connections in client mode
* Full SMS API to send, read, delete and list messages
* Voice call API to start, answer and hang up calls
* Phonebook API to add, edit, delete, read, list and search entries
* Supports USSD code execution
* Includes several applications built on top of the library:
    * MQTT client for MQTT connection
* User friendly MIT license

## Contribute

Fresh contributions are always welcome. Simple instructions to proceed:

1. Fork Github repository
2. Follow [C style & coding rules](https://github.com/MaJerle/c-code-style) and use `clang-format` to format the code
3. Create a pull request to `develop` branch with new features or bug fixes

Alternatively you may:

1. Report a bug
2. Ask for a feature request