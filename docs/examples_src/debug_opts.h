/* Modifications of lwcell_opts.h file for configuration */

/* Enable global debug */
#define LWCELL_CFG_DBG               LWCELL_DBG_ON

/*
 * Enable debug types.
 * Application may use bitwise OR | to use multiple types:
 *    LWCELL_DBG_TYPE_TRACE | LWCELL_DBG_TYPE_STATE
 */
#define LWCELL_CFG_DBG_TYPES_ON      LWCELL_DBG_TYPE_TRACE

/* Enable debug on custom module */
#define MY_DBG_MODULE               LWCELL_DBG_ON
