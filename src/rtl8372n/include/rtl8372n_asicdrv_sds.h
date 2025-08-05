#ifndef _RTL8372N_ASICDRV_SDS_H_
#define _RTL8372N_ASICDRV_SDS_H_

#include "rtl8372n_types.h"
#include "rtl8372n_switch.h"

#define RTL_SDS_MODE_SGMII		    0x02
#define RTL_SDS_MODE_1000BX_FIBER	0x04
#define RTL_SDS_MODE_QXGMII		    0x0d
#define RTL_SDS_MODE_HISGMII		0x12
#define RTL_SDS_MODE_HSG			0x16
#define RTL_SDS_MODE_10GR		    0x1a
#define RTL_SDS_MODE_OFF			0x1f

#ifdef __cplusplus
extern "C" {
#endif

extern ret_t rtl8372n_sds_reg_read(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 *pvalue);
extern ret_t rtl8372n_sds_reg_write(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 value);

extern ret_t rtl8372n_sds_regbits_read(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 bits, rtk_uint32 *pvalue);
extern ret_t rtl8372n_sds_regbits_write(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 bits, rtk_uint32 value);

extern ret_t rtl8372n_sdsMode_set(rtk_uint32 sdsId, rtk_uint32 mode);
extern ret_t rtl8372n_sdsMode_get(rtk_uint32 sdsId, rtk_uint32 *mode);

extern ret_t cfg_rl6637_sds_mode(rtk_uint8 phy_port, rtk_uint32 sds_mode);

extern ret_t SDS_MODE_SET_SW(switch_chip_t chip_type, rtk_uint32 sdsId, rtk_uint32 mode);
extern ret_t serdes_patch(switch_chip_t chip_type, rtk_uint32 sdsId, rtk_uint32 mode);
extern ret_t fiber_fc_en(rtk_uint32 sdsId, rtk_uint32 mode,rtk_uint32 en_flag);
extern ret_t sds_nway_set(rtk_uint32 sdsId, rtk_uint32 mode,rtk_uint32 en_flag);

extern ret_t fw_reset_flow_tgx(rtk_uint32 sdsId);
extern ret_t fw_reset_flow_tgr(rtk_uint32 sdsId);

#ifdef __cplusplus
}
#endif

#endif