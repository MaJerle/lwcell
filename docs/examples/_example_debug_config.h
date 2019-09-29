/*
 * Modifications of gsm_config.h file for configuration
 */

/* Enable global debug */
#define GSM_CFG_DBG                             GSM_DBG_ON

/*
 * Enable debug types. 
 * You may use OR | to use multiple types: GSM_DBG_TYPE_TRACE | GSM_DBG_TYPE_STATE
 */
#define GSM_CFG_DBG_TYPES_ON                    GSM_DBG_TYPE_TRACE

/* Enable debug on custom module */
#define MY_DBG_MODULE                           GSM_DBG_ON
