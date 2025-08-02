#ifndef _RTL8372N_ASICDRV_SVLAN_H_
#define _RTL8372N_ASICDRV_SVLAN_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    rtk_uint8 tx_flow;     // 状态字段0
    rtk_uint8 rx_flow;     // 状态字段1
    rtk_uint8 link_up;     // 状态字段2
    rtk_uint8 duplex;     // 状态字段3
    rtk_uint8 speed;     // 状态字段4 (4位值)
    rtk_uint8 field5;     // 状态字段5
    rtk_uint8 field6;     // 状态字段6
    rtk_uint8 field7;     // 状态字段7
    rtk_uint8 field8;     // 状态字段8
} rtl8372n_port_status_t;

extern ret_t rtl8372n_port_isolation_get(rtk_uint32 port, rtk_uint32 *isolation_map);
extern ret_t rtl8372n_port_isolation_set(rtk_uint32 port, rtk_uint32 isolation_map);
extern ret_t rtl8372n_portStatus_get(rtk_uint32 port, rtl8372n_port_status_t *status);

#ifdef __cplusplus
}
#endif

#endif