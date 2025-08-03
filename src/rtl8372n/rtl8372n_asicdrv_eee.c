#include "include/rtk_error.h"
#include "include/rtl8372n_asicdrv.h"
#include "include/rtl8372n_asicdrv_eee.h"
#include "include/rtl8372n_switch.h"

ret_t rtl8372n_eee_init(void)
{
  return 0LL;
}

ret_t rtl8372n_eee_macForceSpeedEn_set(rtk_port_t port, rtk_uint32 uno_1, rtk_uint32 enable)
{
    ret_t result; // x0
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    result = 23LL;
    if ( uno_1 > 8 ) return 23LL;
    if ( enable > 1 ) return RT_ERR_ENABLE;

    result = rtl8372n_setAsicRegBit(4 * port + 0x636C, uno_1, enable);
    if ( result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}


ret_t rtl8372n_eee_macForceSpeedEn_get(rtk_port_t port, rtk_uint32 uno_1, rtk_uint32 *enabled)
{
    ret_t result; // x0
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    if (uno_1 > 8) return 23LL;

    if (!enabled) return RT_ERR_NULL_POINTER;
    
    result = rtl8372n_getAsicRegBit(4 * port + 0x636C, uno_1, enabled);
    if ( result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}


ret_t rtl8372n_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *enabled)
{
    ret_t result; // x0

    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    if (!enabled) return RT_ERR_NULL_POINTER;

    result = rtl8372n_getAsicRegBits(4 * port + 0x636C, 0x1FF, enabled);
    if ( result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}

ret_t rtl8372n_eee_portTxRxEn_set(rtk_port_t port, rtk_uint32 tx_enable, rtk_uint32 rx_enable)
{
    ret_t result; // x0
    rtk_uint32 reg_addr; // w19

    // 步骤1: 验证端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    // 步骤2: 验证使能标志范围
    if ( tx_enable > 1 ) return 23LL;
    if ( rx_enable > 1 ) return 23LL;
    
    // 步骤3: 计算EEE控制寄存器地址
    // 寄存器地址 = 4700 + (端口号 * 256)
    reg_addr = (port << 8) + 0x125C;

    // 步骤4: 设置EEE发送使能 (寄存器位0)
    result = rtl8372n_setAsicRegBit(reg_addr, 0u, tx_enable);
    if ( result != RT_ERR_OK) return result;

    // 步骤5: 设置EEE接收使能 (寄存器位1)
    result = rtl8372n_setAsicRegBit(reg_addr, 1u, rx_enable);
    if ( result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}


ret_t rtl8372n_eee_portTxRxEn_get(rtk_port_t port, rtk_uint32 *tx_enable, rtk_uint32 *rx_enable)
{
    ret_t result; // x0
    rtk_uint32 reg_addr; // w19

    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    if (!tx_enable) return RT_ERR_NULL_POINTER;
    if (!rx_enable) return RT_ERR_NULL_POINTER;

    reg_addr = (port << 8) + 4700;
    result = rtl8372n_getAsicRegBit(reg_addr, 0u, tx_enable);
    if ( result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(reg_addr, 1u, rx_enable);
    if ( result != RT_ERR_OK) return result;

    return result;
}