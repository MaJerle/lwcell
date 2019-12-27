.. _api_app_netconn:

Netconn API
===========

*Netconn API* is addon on top of existing connection module and allows sending and receiving data with sequential API calls, similar to *POSIX socket* API.

It can operate in client mode and uses operating system features, such as message queues and semaphore to link non-blocking callback API for connections with sequential API for application thread.

.. note::
    Connection API does not directly allow receiving data with sequential and linear code execution.
    All is based on connection event system. 
    Netconn adds this functionality as it is implemented on top of regular connection API.

.. warning::
    Netconn API are designed to be called from application threads ONLY. 
    It is not allowed to call any of *netconn API* functions from within interrupt or callback event functions.

Netconn client
^^^^^^^^^^^^^^

.. figure:: ../../static/images/netconn_client.svg
	:align: center
	:alt: Netconn API client block diagram

	Netconn API client block diagram

Above block diagram shows basic architecture of netconn client application. 
There is always one application thread (in green) which calls *netconn API* functions to interact with connection API in synchronous mode.

Every netconn connection uses dedicated structure to handle message queue for data received packet buffers.
Each time new packet is received (red block, *data received event*), reference to it is written to message queue of netconn structure, while application thread reads new entries from the same queue to get packets.

.. literalinclude:: ../../../snippets/netconn_client.c
    :language: c
    :caption: Netconn client example

Non-blocking receive
^^^^^^^^^^^^^^^^^^^^

By default, netconn API is written to only work in separate application thread,
dedicated for network connection processing. Because of that, by default every function is fully blocking.
It will wait until result is ready to be used by application.

It is, however, possible to enable timeout feature for receiving data only.
When this feature is enabled, :cpp:func:`gsm_netconn_receive` will block for maximal timeout set with
:cpp:func:`gsm_netconn_set_receive_timeout` function.

When enabled, if there is no received data for timeout amount of time, function will return with timeout status and application needs to process it accordingly.

.. tip::
    :c:macro:`GSM_CFG_NETCONN_RECEIVE_TIMEOUT` must be set to ``1`` to use this feature.

.. doxygengroup:: GSM_NETCONN