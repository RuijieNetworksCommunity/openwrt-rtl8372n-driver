#include "rtk_error.h"
#include "rtl8372n_asicdrv.h"
#include "rtl8372n_asicdrv_port.h"
#include "rtl8372n_switch.h"

ret_t rtl8372n_port_isolation_set(rtk_uint32 port, rtk_uint32 isolation_map)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    if ( isolation_map > 0x3FF ) return RT_ERR_PORT_MASK;

    return rtl8372n_setAsicReg(4 * port + 0x50C0, isolation_map);
}

ret_t rtl8372n_port_isolation_get(rtk_uint32 port, rtk_uint32 *isolation_map)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    if (!isolation_map) return RT_ERR_NULL_POINTER;

    return rtl8372n_getAsicReg(4 * port + 0x50C0, isolation_map);
}

ret_t rtl8372n_portStatus_get(rtk_uint32 port, rtl8372n_port_status_t *status)
{
    ret_t result; // x0
    rtk_uint32 v5; // [xsp+3Ch] [xbp+3Ch] BYREF

    v5 = 0;
    result = rtl8372n_getAsicReg(0x63E8u, &v5);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicReg(0x63E8u, &v5);
    if (result != RT_ERR_OK) return result;
    status->link_up = (v5 >> port) & 1;

    result = rtl8372n_getAsicRegBits(0x63F0u + (4 * (port / 8)), 15 << (4 * (port & 7)), &v5);
    if (result != RT_ERR_OK) return result;
    status->speed = v5;

    result = rtl8372n_getAsicReg(0x63F8u, &v5);
    if (result != RT_ERR_OK) return result;
    status->duplex = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x63FCu, &v5);
    if (result != RT_ERR_OK) return result;
    status->tx_flow = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x63FCu, &v5);
    if (result != RT_ERR_OK) return result;
    status->rx_flow = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x63ECu, &v5);
    if (result != RT_ERR_OK) return result;
    status->field5 = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x6404u, &v5);
    if (result != RT_ERR_OK) return result;
    status->field6 = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x6408u, &v5);
    if (result != RT_ERR_OK) return result;
    status->field7 = (v5 >> port) & 1;

    result = rtl8372n_getAsicReg(0x640Cu, &v5);
    if (result != RT_ERR_OK) return result;
    status->field8 = (v5 >> port) & 1;

    return RT_ERR_OK;
}