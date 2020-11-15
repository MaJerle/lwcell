/* Modifications of lwgsm_opts.h file for configuration */

/* Enable global debug */
#define LWGSM_CFG_DBG               LWGSM_DBG_ON

/*
 * Enable debug types.
 * Application may use bitwise OR | to use multiple types:
 *    LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_TYPE_STATE
 */
#define LWGSM_CFG_DBG_TYPES_ON      LWGSM_DBG_TYPE_TRACE

/* Enable debug on custom module */
#define MY_DBG_MODULE               LWGSM_DBG_ON
