LwCELL |version| documentation
=============================

Welcome to the documentation for version |version|.

LwCELL is lightweight, platform independent, cellular modem AT commands parser, targeting (as of today) communicion with *SIMCOM* based modules *SIM800/SIM900* or *SIM70xx*.
Module is written in C11 and is independent from used platform. Its main targets are embedded system devices like ARM Cortex-M, AVR, PIC and others, but can easily work under *Windows*, *Linux* or *MAC* environments.

.. image:: static/images/logo.svg
    :align: center

.. rst-class:: center
.. rst-class:: index_links

	:ref:`download_library` :ref:`getting_started` `Open Github <https://github.com/MaJerle/lwcell>`_ `Donate <https://paypal.me/tilz0R>`_

Features
^^^^^^^^

* Supports ``SIM800/SIM900 (2G)`` and ``SIM7000/SIM7020 (NB-Iot LTE)`` modules
* Platform independent and very easy to port

    * Development of library under Win32 platform
    * Provided examples for ARM Cortex-M or Win32 platforms

* Written in C language (C11)
* Allows different configurations to optimize user requirements
* Supports implementation with operating systems with advanced inter-thread communications

    * Currently only OS mode is supported
    * 2 different threads handling user data and received data

        * First (producer) thread (collects user commands from user threads and starts the command processing)
        * Second (process) thread reads the data from GSM device and does the job accordingly

* Allows sequential API for connections in client and server mode
* Includes several applications built on top of library

    * MQTT client for MQTT connection

* User friendly MIT license

Requirements
^^^^^^^^^^^^

* C compiler
* Supported GSM Physical device

Contribute
^^^^^^^^^^

Fresh contributions are always welcome. Simple instructions to proceed:

#. Fork Github repository
#. Respect `C style & coding rules <https://github.com/MaJerle/c-code-style>`_ used by the library
#. Create a pull request to ``develop`` branch with new features or bug fixes

Alternatively you may:

#. Report a bug
#. Ask for a feature request

License
^^^^^^^

.. literalinclude:: ../LICENSE

Table of contents
^^^^^^^^^^^^^^^^^

.. toctree::
    :maxdepth: 2
    :caption: Contents

    self
    get-started/index
    user-manual/index
    api-reference/index
    examples/index
    changelog/index

.. toctree::
    :maxdepth: 2
    :caption: Other projects
    :hidden:

    LwBTN - Button manager <https://github.com/MaJerle/lwbtn>
    LwDTC - DateTimeCron <https://github.com/MaJerle/lwdtc>
    LwESP - ESP-AT library <https://github.com/MaJerle/lwesp>
    LwEVT - Event manager <https://github.com/MaJerle/lwevt>
    LwGPS - GPS NMEA parser <https://github.com/MaJerle/lwgps>
    LwCELL - GSM-AT library <https://github.com/MaJerle/lwcell>
    LwJSON - JSON parser <https://github.com/MaJerle/lwjson>
    LwMEM - Memory manager <https://github.com/MaJerle/lwmem>
    LwOW - OneWire with UART <https://github.com/MaJerle/lwow>
    LwPKT - Packet protocol <https://github.com/MaJerle/lwpkt>
    LwPRINTF - Printf <https://github.com/MaJerle/lwprintf>
    LwRB - Ring buffer <https://github.com/MaJerle/lwrb>
    LwSHELL - Shell <https://github.com/MaJerle/lwshell>
    LwUTIL - Utility functions <https://github.com/MaJerle/lwutil>
    LwWDG - RTOS task watchdog <https://github.com/MaJerle/lwwdg>