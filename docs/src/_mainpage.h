/**
 * \mainpage
 * \tableofcontents
 * 
 * GSM AT is generic and advanced library for communicating to SIM800/900 and SIM7000/7020
 * wG/LTE modems using AT commands
 *
 * It is intented to work with embedded systems and is device and platform independent.
 *
 * \section         sect_features Features
 *
 *  - Supports `SIM800/900 (2G)` or `SIM7000/SIM7020 (NB-IoT)` modules
 *  - Platform independent and very easy to port
 *     - Development of library under Win32 platform
 *     - Provided examples for ARM Cortex-M or Win32 platforms
 *  - Written in C language (C99)
 *  - Allows different configurations to optimize user requirements
 *  - Supports implementation with operating systems with advanced inter-thread communications
 *      - Currently only OS mode is supported
 *      - 2 different threads handling user data and received data
 *          - First (producer) thread (collects user commands from user threads and starts the command processing)
 *          - Second (process) thread reads the data from ESP device and does the job accordingly
 *  - Allows sequential API for connections in client and server mode
 *  - Includes several applications built on top of library:
 *      - \ref GSM_APP_MQTT_CLIENT for MQTT connection
 *  - User friendly MIT license
 *
 * \section         sect_resources Download & Resources
 *
 *  - <a class="download_url" href="https://github.com/MaJerle/GSM_AT_Lib/releases">Download library at Github releases</a>
 *  - <a href="https://github.com/MaJerle/GSM_AT_Lib">Resources and examples repository</a>
 *  - Read \ref page_appnote before you start development
 *  - <a href="https://github.com/MaJerle/GSM_AT_Lib">Official development repository on Github</a>
 *
 * \section         sect_contribute How to contribute
 * 
 *  - Official development repository is hosted on Github
 *  - <a href="https://github.com/MaJerle/c_code_style">Respect C style and coding rules</a>
 *
 * \section         sect_license License
 *
 * \verbatim        
 * Copyright (c) 2019 Tilen MAJERLE
 *  
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. \endverbatim
 *
 */