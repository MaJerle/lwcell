size_t rem_len;
lwcell_conn_p conn;
lwcellr_t res;

/* ... other tasks to make sure connection is established */

/* We are connected to server at this point! */
/*
 * Call write function to write data to memory
 * and do not send immediately unless buffer is full after this write
 *
 * rem_len will give us response how much bytes
 * is available in memory after write
 */
res = lwcell_conn_write(conn, "My string", 9, 0, &rem_len);
if (rem_len == 0) {
    printf("No more memory available for next write!\r\n");
}
res = lwcell_conn_write(conn, "example.com", 11, 0, &rem_len);

/*
 * Data will stay in buffer until buffer is full,
 * except if user wants to force send,
 * call write function with flush mode enabled
 *
 * It will send out together 20 bytes
 */
lwcell_conn_write(conn, NULL, 0, 1, NULL);
