/* Request data are sent to server once we are connected */
uint8_t req_data[] = ""
    "GET / HTTP/1.1\r\n"
    "Host: example.com\r\n"
    "Connection: close\r\n"
    "\r\n";

/*
 * \brief           Connection callback function
 *                  Called on several connection events, such as connected, closed, data received, data sent, ...
 * \param[in]       evt: GSM callback event
 */
static gsmr_t
conn_evt(gsm_evt_t* evt) {
    gsm_conn_p conn = gsm_conn_get_from_evt(evt);   /* Get connection from current event */
    if (conn == NULL) {
        return gsmERR;                          /* Return error at this point as this should never happen! */ 
    }
    
    switch (gsm_evt_get_type(evt)) {
        /* A new connection just became active */
        case GSM_EVT_CONN_ACTIVE: {
            printf("Connection active!\r\n");
            
            /*
             * After we are connected,
             * send the HTTP request string in non-blocking way
             */
            gsm_conn_send(conn, req_data, sizeof(req_data) - 1, NULL, 0);
            break;
        }
        
        /* Connection closed event */
        case GSM_EVT_CONN_CLOSED: {
            printf("Connection closed!\r\n");
            if (evt->evt.conn_active_closed.forced) {   /* Was it forced by user? */
                printf("Connection closed by user\r\n");
            } else {
                printf("Connection closed by remote host\r\n");
            }
        }
        
        /* Data received on connection */
        case GSM_EVT_CONN_RECV: {
            gsm_pbuf_p pbuf = cb->evt.conn_data_recv.buff;   /* Get data buffer */
            
            /*
             * Connection data buffer is automatically
             * freed when you return from function
             * If you still want to hold it,
             * then either chain it using gsm_pbuf_chain
             * or reference it using gsm_pbuf_ref functions.
             */
            
            printf("Connection data received!\r\n");
            if (pbuf != NULL) {
                size_t len;
                /*
                 * You should not call gsm_pbuf_free on this variable unless
                 * you used gsm_pbuf_ref before to increase reference
                 */
                len = gsm_pbuf_length(pbuf, 1); /* Get total length of buffer */
                printf("Length of data: %d bytes\r\n", (int)len);
            }
			
			gsm_conn_recved(conn, pbuf);        /* Notify stack about received data */ 
        }
        default:
            break;
    }
    return gsmOK;
}

/*
 * \brief           Thread function
 */
static void
thread_or_main_func(void) {
    /*
     * Start the connection in non-blocking way and set the
     * function argument to NULL and callback function to conn_evt
     */
    gsm_conn_start(NULL, GSM_CONN_TYPE_TCP, "example.com", 80, NULL, conn_evt, 0);
    
    // Do other tasks...
}
