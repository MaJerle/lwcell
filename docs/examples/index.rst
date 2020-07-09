.. _examples:

Examples and demos
==================

Various examples are provided for fast library evaluation on embedded systems. These are optimized prepared and maintained for ``2`` platforms, but could be easily extended to more platforms:

* WIN32 examples, prepared as `Visual Studio Community <https://visualstudio.microsoft.com/vs/community/>`_ projects
* ARM Cortex-M examples for STM32, prepared as `STM32CubeIDE <https://www.st.com/en/development-tools/stm32cubeide.html>`_ GCC projects

.. warning::
	Library is platform independent and can be used on any platform.

Example architectures
^^^^^^^^^^^^^^^^^^^^^

There are many platforms available today on a market, however supporting them all would be tough task for single person.
Therefore it has been decided to support (for purpose of examples) ``2`` platforms only, `WIN32` and `STM32`.

WIN32
*****

Examples for *WIN32* are prepared as `Visual Studio Community <https://visualstudio.microsoft.com/vs/community/>`_ projects.
You can directly open project in the IDE, compile & debug.

Application opens *COM* port, set in the low-level driver. 
External USB to UART converter (FTDI-like device) is necessary in order to connect to *GSM* device.

.. note::
	*GSM* device is connected with *USB to UART converter* only by *RX* and *TX* pins.

Device driver is located in ``/lwgsm/src/system/lwgsm_ll_win32.c``

STM32
*****

Embedded market is supported by many vendors and STMicroelectronics is, with their `STM32 <https://www.st.com/en/microcontrollers-microprocessors/stm32-32-bit-arm-cortex-mcus.html>`_ series of microcontrollers, one of the most important players.
There are numerous amount of examples and topics related to this architecture.

Examples for *STM32* are natively supported with `STM32CubeIDE <https://www.st.com/en/development-tools/stm32cubeide.html>`_, an official development IDE from STMicroelectronics.

You can run examples on one of official development boards, available in repository examples.

.. table:: Supported development boards

	+----------------------+-----------------------------+----------------------+
	| Board name           | GSM settings                | Debug settings       |
	|                      +--------+------+------+------+--------+------+------+
	|                      | UART   | MTX  | MRX  | RST  | UART   | MDTX | MDRX |
	+======================+========+======+======+======+========+======+======+
	| STM32F429ZI-Nucleo   | USART6 | PC6  | PC7  | PC5  | USART3 | PD8  | PD9  |
	+----------------------+--------+------+------+------+--------+------+------+

Pins to connect with GSM device:

* *MTX*: MCU TX pin, connected to GSM RX pin
* *MRX*: MCU RX pin, connected to GSM TX pin
* *RST*: MCU output pin to control reset state of GSM device

Other pins are for your information and are used for debugging purposes on board.

* MDTX: MCU Debug TX pin, connected via on-board ST-Link to PC
* MDRX: MCU Debug RX pin, connected via on-board ST-Link to PC
* Baudrate is always set to ``921600`` bauds

Examples list
^^^^^^^^^^^^^

Here is a list of all examples coming with this library.

.. tip::
	Examples are located in ``/examples/`` folder in downloaded package.
	Check :ref:`download_library` section to get your package.

.. tip::
	Do not forget to set PIN & PUK codes of your SIM card before running any of examples.
	Open ``/snippets/sim_manager.c`` and update ``pin_code`` and ``puk_code`` variables.

Device info
***********

Simple example which prints basic device information:

* Device Manufacturer
* Device Model
* Device serial number
* Device revision number

MQTT Client API
***************

Similar to *MQTT Client* examples, but it uses separate thread to process
events in blocking mode. Application does not use events to process data,
rather it uses blocking API to receive packets

Netconn client
**************

Netconn client is based on sequential API.
It starts connection to server, sends initial request and then waits to receive data.

Processing is in separate thread and fully sequential, no callbacks or events.

Call
****

Call example answers received call. If GSM device supports calls and has microphone/speaker connected
to module itself, it can simply communicate over voice.

Call & SMS
**********

This example shows how to receive a call and send reply with SMS.
When application receives call, it hangs-up immediately and sends back SMS asking caller to send SMS instead.

When application receives SMS, it will send same SMS content back to the sender's number.

SMS Send receive
****************

It demonstrates sending and receiving SMS either in events or using thread processing.

.. toctree::
	:maxdepth: 2
