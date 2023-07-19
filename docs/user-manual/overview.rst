.. _um_overview:

Overview
========

IoT activity is embedded in today's application. Almost every device uses some type of communication,
from WiFi, BLE, Sub-GHz or NB-IoT/LTE/2G/3G.

*LwCELL* has been developed to allow customers to:

* Develop on single (host MCU) architecture at the same time and do not care about device arch
* Shorten time to market

Customers using *LwCELL* do not need to take care about proper command for specific task,
they can call API functions to execute the task.
Library will take the necessary steps in order to send right command to device via low-level driver (usually UART) and 
process incoming response from device before it will notify application layer if it was successfuly or not.

To summarize:

* *GSM* device runs official *AT* firmware, provided by device vendor
* Host MCU runs custom application, together with *LwCELL* library
* Host MCU communicates with *GSM* device with UART or similar interface.

.. toctree::
    :maxdepth: 2
    :glob: