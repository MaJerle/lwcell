LwGSM |version| documentation
=============================

Welcome to the documentation for version |version|.

LwGSM is generic, platform independent, library to control *SIM800/SIM900* or *SIM7000/SIM7020 (NB-Iot LTE)* devices from *SIMCom*.
Its objective is to run on master system, while SIMCom device runs official AT commands firmware developed and maintained by *SIMCom*.

.. image:: static/images/logo.svg
    :align: center

.. rst-class:: center
.. rst-class:: index_links

	:ref:`download_library` :ref:`getting_started` `Open Github <https://github.com/MaJerle/lwgsm>`_ `Donate <https://paypal.me/tilz0R>`_

Features
^^^^^^^^

* Supports ``SIM800/SIM900 (2G)`` and ``SIM7000/SIM7020 (NB-Iot LTE)`` modules
* Platform independent and very easy to port

    * Development of library under Win32 platform
    * Provided examples for ARM Cortex-M or Win32 platforms

* Written in C language (C99)
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

    self
    get-started/index
    user-manual/index
    api-reference/index
    examples/index
