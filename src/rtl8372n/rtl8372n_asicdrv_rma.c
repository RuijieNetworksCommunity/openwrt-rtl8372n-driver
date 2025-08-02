#include "rtk_error.h"
#include "rtl8372n_asicdrv.h"
#include "rtl8372n_asicdrv_rma.h"
#include "rtl8372n_switch.h"

//放弃挣扎
ret_t rtl8372n_asicRma_set(rtk_uint32 index, rtl8372n_rma_t *pRmacfg)
{
    rtk_uint32 reg_addr;

    if ( index > 0x2F ) return RT_ERR_RMA_ADDR;

    // MD 这又是啥套娃(浅浅的死了)
    rtk_uint8 v2 = index - 4 > 3 && index - 9 > 3;
    reg_addr = 0x4EDC;
    if ( v2 && index != 15 )
    {
        reg_addr = 0x4EE0;
        if ( index != 8 )
        {
        reg_addr = 0x4EE4;
        if ( index != 13 )
        {
            reg_addr = 0x4EE8;
            if ( index != 14 )
            {
            reg_addr = 0x4EEC;
            if ( index != 16 )
            {
                reg_addr = 0x4EF0;
                if ( index != 17 )
                {
                reg_addr = 0x4EF4;
                if ( index != 18 )
                {
                    reg_addr = 0x4EF8;
                    if ( (index & 0xFFFFFFF7) - 0x13 > 4 && index != 25 )
                    {
                    reg_addr = 0x4EFC;
                    if ( index != 24 )
                    {
                        reg_addr = 0x4F00;
                        if ( index != 26 )
                        {
                        reg_addr = 0x4F04;
                        if ( index != 32 )
                        {
                            reg_addr = 0x4F08;
                            if ( index != 33 )
                            {
                            if ( (index - 0x22) <= 0xD )
                                reg_addr = 0x4F0C;
                            else
                                reg_addr = ( 4 * (index + 0x13B3));
                            }
                        }
                        }
                    }
                    }
                }
                }
            }
            }
        }
        }
    }

    ret_t result = rtl8372n_setAsicRegBits(reg_addr, 0x30, pRmacfg->operation);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(reg_addr, 3u, pRmacfg->discard_storm_filter);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(reg_addr, 2u, pRmacfg->keep_format);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(reg_addr, 1u, pRmacfg->vlan_leaky);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(reg_addr, 0, pRmacfg->portiso_leaky);
    if (result != RT_ERR_OK) return result;

    return rtl8372n_setAsicRegBits(0x4F1Cu, 7, pRmacfg->trap_priority);
}

ret_t rtl8372n_asicRma_get(rtk_uint32 index, rtl8372n_rma_t *pRmacfg)
{
    rtk_uint32 reg_addr; // w20

    reg_addr = 0x4EDC;
    if ( (index - 4) > 3 && (index - 9) > 3 && index != 15 )
    {
        reg_addr = 0x4EE0;
        if ( index != 8 )
        {
        reg_addr = 0x4EE4;
        if ( index != 13 )
        {
            reg_addr = 0x4EE8;
            if ( index != 14 )
            {
            reg_addr = 0x4EEC;
            if ( index != 16 )
            {
                reg_addr = 0x4EF0;
                if ( index != 17 )
                {
                reg_addr = 0x4EF4;
                if ( index != 18 )
                {
                    reg_addr = 0x4EF8;
                    if ( (index & 0xFFFFFFF7) - 0x13 > 4 && index != 25 )
                    {
                    reg_addr = 0x4EFC;
                    if ( index != 24 )
                    {
                        reg_addr = 0x4F00;
                        if ( index != 26 )
                        {
                        reg_addr = 0x4F04;
                        if ( index != 32 )
                        {
                            reg_addr = 0x4F08;
                            if ( index != 33 )
                            {
                            if ( (index - 0x22) <= 0xD )
                                reg_addr = 0x4F0C;
                            else
                                reg_addr = (4 * (index + 5043));
                            }
                        }
                        }
                    }
                    }
                }
                }
            }
            }
        }
        }
    }

    ret_t result = rtl8372n_getAsicRegBits(reg_addr, 48, &(pRmacfg->operation));
    if (result != RT_ERR_OK) return result;


    result = rtl8372n_getAsicRegBit(reg_addr, 3, &(pRmacfg->discard_storm_filter));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(reg_addr, 2, &(pRmacfg->keep_format));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(reg_addr, 1, &(pRmacfg->vlan_leaky));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(reg_addr, 0, &(pRmacfg->portiso_leaky));
    if (result != RT_ERR_OK) return result;

    return rtl8372n_getAsicRegBits(0x4F1Cu, 7, &(pRmacfg->trap_priority));
}



ret_t rtl8372n_asicRmaCdp_set(rtl8372n_rma_t *pRmacfg)
{
    ret_t result;

    if ( pRmacfg->operation > 3 ) return RT_ERR_RMA_ACTION;
    if ( pRmacfg->trap_priority > 7 ) return RT_ERR_QOS_INT_PRIORITY;

    result = rtl8372n_setAsicRegBits(0x4F10u, 48, pRmacfg->operation);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F10u, 3u, pRmacfg->discard_storm_filter);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F10u, 2u, pRmacfg->keep_format);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F10u, 1u, pRmacfg->vlan_leaky);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F10u, 0, pRmacfg->portiso_leaky);
    if (result != RT_ERR_OK) return result;

    return rtl8372n_setAsicRegBits(0x4F1Cu, 7, pRmacfg->trap_priority);
}


ret_t rtl8372n_asicRmaCdp_get(rtl8372n_rma_t *pRmacfg)
{
    ret_t result;

    result = rtl8372n_getAsicRegBits(0x4F10u, 48, &(pRmacfg->operation));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F10u, 3, &(pRmacfg->discard_storm_filter));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F10u, 2, &(pRmacfg->keep_format));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F10u, 1, &(pRmacfg->vlan_leaky));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F10u, 0, &(pRmacfg->portiso_leaky));
    if (result != RT_ERR_OK) return result;

    return rtl8372n_getAsicRegBits(0x4F1Cu, 7, &(pRmacfg->trap_priority));
}



ret_t rtl8372n_asicRmaCsstp_set(rtl8372n_rma_t *pRmacfg)
{
  ret_t result; // x0

    if ( pRmacfg->operation > 3 ) return RT_ERR_RMA_ACTION;
    if ( pRmacfg->trap_priority > 7 ) return RT_ERR_QOS_INT_PRIORITY;

    result = rtl8372n_setAsicRegBits(0x4F14u, 48, pRmacfg->operation);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F14u, 3u, pRmacfg->discard_storm_filter);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F14u, 2u, pRmacfg->keep_format);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F14u, 1u, pRmacfg->vlan_leaky);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F14u, 0, pRmacfg->portiso_leaky);
    if (result != RT_ERR_OK) return result;

    return rtl8372n_setAsicRegBits(0x4F1Cu, 7, pRmacfg->trap_priority);
}


ret_t rtl8372n_asicRmaCsstp_get(rtl8372n_rma_t *pRmacfg)
{
    ret_t result;

    result = rtl8372n_getAsicRegBits(0x4F14u, 48, &(pRmacfg->operation));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F14u, 3, &(pRmacfg->discard_storm_filter));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F14u, 2, &(pRmacfg->keep_format));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F14u, 1, &(pRmacfg->vlan_leaky));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F14u, 0, &(pRmacfg->portiso_leaky));
    if (result != RT_ERR_OK) return result;

    return rtl8372n_getAsicRegBits(0x4F1Cu, 7, &(pRmacfg->trap_priority));

}

ret_t rtl8372n_asicRmaLldp_set(rtk_uint32 enabled, rtl8372n_rma_t *pRmacfg)
{
    ret_t result; // x0

    if ( enabled > 1 ) return RT_ERR_ENABLE;
    if ( pRmacfg->operation > 3 ) return RT_ERR_RMA_ACTION;
    if ( pRmacfg->trap_priority > 7 ) return RT_ERR_QOS_INT_PRIORITY;

    result = rtl8372n_setAsicRegBit(0x4F1Cu, 3u, enabled);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBits(20248LL, 48LL, pRmacfg->operation);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F18u, 3u, pRmacfg->discard_storm_filter);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F18u, 2u, pRmacfg->keep_format);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F18u, 1u, pRmacfg->vlan_leaky);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x4F18u, 0, pRmacfg->portiso_leaky);
    if (result != RT_ERR_OK) return result;

    return rtl8372n_setAsicRegBits(20252LL, 7LL, pRmacfg->trap_priority);
}

ret_t rtl8372n_asicRmaLldp_get(rtk_uint32 *enabled, rtl8372n_rma_t *pRmacfg)
{
    ret_t result; // x0

    result = rtl8372n_getAsicRegBit(0x4F1Cu, 3, enabled);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBits(0x4F18u, 48, &(pRmacfg->operation));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F18u, 3, &(pRmacfg->discard_storm_filter));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F18u, 2, &(pRmacfg->keep_format));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F18u, 1, &(pRmacfg->vlan_leaky));
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_getAsicRegBit(0x4F18u, 0, &(pRmacfg->portiso_leaky));
    if (result != RT_ERR_OK) return result;

    return rtl8372n_getAsicRegBits(0x4F1Cu, 7, &(pRmacfg->trap_priority));
}

ret_t rtl8372n_asicRmaTrapPri_set(rtk_uint32 trap_priority)
{
    return rtl8372n_setAsicRegBits(0x4F1Cu, 7, trap_priority);
}

ret_t rtl8372n_asicRmaTrapPri_get(rtk_uint32 *trap_priority)
{
    return rtl8372n_getAsicRegBits(0x4F1Cu, 7, trap_priority);
}