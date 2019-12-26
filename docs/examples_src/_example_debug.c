#include "gsm/gsm_debug.h"

/*
 * Print debug message to the screen
 * Trace message will be printed as it is enabled in types 
 * while state message will not be printed.
 */
GSM_DEBUGF(MY_DBG_MODULE | GSM_DBG_TYPE_TRACE, "This is trace message on my program\r\n");
GSM_DEBUGF(MY_DBG_MODULE | GSM_DBG_TYPE_STATE, "This is state message on my program\r\n");
