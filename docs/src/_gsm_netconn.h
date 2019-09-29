/**
 * \addtogroup      GSM_NETCONN
 * \{
 *
 * Netconn provides sequential API to work with connection 
 * in either server or client mode.
 *
 * Netconn API can handle reading asynchronous network data in synchronous way
 * by using operating system features such as message queues 
 * and by putting thread into blocking mode, it allows zero overhead 
 * from performance point of view.
 * 
 * \section         sect_netconn_client Netconn client
 *
 * \image html netconn_client.svg Netconn API architecture
 *
 * Every netconn structure consists of at least data message queue 
 * to handle received packets before user reads them in user thread.
 *
 * On image we can see blue box showing connection handle data queue.
 * Data queue is filled from connection callback which is dedicated specially for
 * netconn type connections.
 *
 * When user wants to read data from connection,
 * thread will be blocked until something is available in received data message queue.
 *
 * To allow user to handle closed connections while waiting for more data,
 * information about closed connection is also added to received data message queue.
 *
 * \par             Example
 * 
 * Example shows how to use netconn API to write and read data in synchronous way,
 * no need to have complex code structure for asynchronous data reception callbacks
 *
 * \include         _example_netconn_client.c
 *
 * \}
 */