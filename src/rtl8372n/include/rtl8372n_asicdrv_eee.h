#ifndef _RTL8372N_ASICDRV_EEE_H_
#define _RTL8372N_ASICDRV_EEE_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ret_t rtl8372n_eee_init(void);
extern ret_t rtl8372n_eee_macForceSpeedEn_set(rtk_port_t port, rtk_uint32 uno_1, rtk_uint32 enable);
extern ret_t rtl8372n_eee_macForceSpeedEn_get(rtk_port_t port, rtk_uint32 uno_1, rtk_uint32 *enabled);
extern ret_t rtl8372n_eee_portTxRxEn_set(rtk_port_t port, rtk_uint32 tx_enable, rtk_uint32 rx_enable);
extern ret_t rtl8372n_eee_portTxRxEn_get(rtk_port_t port, rtk_uint32 *tx_enable, rtk_uint32 *rx_enable);
extern ret_t rtl8372n_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *enabled);

#ifdef __cplusplus
}
#endif

#endif