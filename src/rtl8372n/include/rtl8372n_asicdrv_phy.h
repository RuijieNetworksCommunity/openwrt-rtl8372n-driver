#ifndef _RTL8372N_ASICDRV_PHY_H_
#define _RTL8372N_ASICDRV_PHY_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif
extern ret_t rtl8372n_phy_read(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 *pvalue);
extern ret_t rtl8372n_phy_write(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 value);

extern ret_t rtl8372n_phy_readBits(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 *pvalue);
extern ret_t rtl8372n_phy_writeBits(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 value);

extern ret_t rtl8372n_phy_regbits_read(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 *pvalue);
extern ret_t rtl8372n_phy_regbits_write(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 value);

extern rtk_uint32 uc1_sram_read_8b(rtk_uint32 port, rtk_uint32 addr);
extern ret_t uc1_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value);
extern rtk_uint32 uc2_sram_read_8b(rtk_uint32 port, rtk_uint32 addr);
extern ret_t uc2_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value);
extern ret_t data_ram_write_8b(rtk_uint8 port, rtk_uint32 addr, rtk_uint32 value);

extern ret_t rtl8372n_phy_autoNegoAbility_set(rtk_uint32 port, rtk_uint8 *a2);
extern ret_t rtl8372n_phy_conmmon_c45_autoSpeed_set(rtk_uint32 port, rtk_uint8 *a2);
extern ret_t rtl8372n_phy_common_c45_autoNegoEnable_set(rtk_uint32 port, rtk_uint32 enable);
extern ret_t rtl8372n_phy_common_c45_autoNegoEnable_get(rtk_uint32 port, rtk_uint32 *enable);
extern ret_t rtl8372n_phy_common_c45_an_restart(rtk_uint32 port);

#ifdef __cplusplus
}
#endif

#endif