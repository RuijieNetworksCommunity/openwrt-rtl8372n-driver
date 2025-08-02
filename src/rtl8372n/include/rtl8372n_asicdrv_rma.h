#ifndef _RTL8372N_ASICDRV_RMA_H_
#define _RTL8372N_ASICDRV_RMA_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    rtk_uint32 operation;
    rtk_uint32 discard_storm_filter;
    rtk_uint32 trap_priority;
    rtk_uint32 keep_format;
    rtk_uint32 vlan_leaky;
    rtk_uint32 portiso_leaky;
}rtl8372n_rma_t;


extern ret_t rtl8372n_asicRma_set(rtk_uint32 index, rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRma_get(rtk_uint32 index, rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaCdp_set(rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaCdp_get(rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaCsstp_set(rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaCsstp_get(rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaLldp_set(rtk_uint32 enabled, rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaLldp_get(rtk_uint32 *enabled, rtl8372n_rma_t *pRmacfg);
extern ret_t rtl8372n_asicRmaTrapPri_set(rtk_uint32 trap_priority);
extern ret_t rtl8372n_asicRmaTrapPri_get(rtk_uint32 *trap_priority);

#ifdef __cplusplus
}
#endif

#endif