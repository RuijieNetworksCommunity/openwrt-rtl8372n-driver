#ifndef _RTL8372N_ASICDRV_H_
#define _RTL8372N_ASICDRV_H_

#include "rtl8372n_types.h"

#define RTL8372N_REGBITLENGTH               32

#ifdef __cplusplus
extern "C" {
#endif
extern ret_t rtl8372n_setAsicRegBit(rtk_uint32 reg, rtk_uint32 bit, rtk_uint32 value);
extern ret_t rtl8372n_getAsicRegBit(rtk_uint32 reg, rtk_uint32 bit, rtk_uint32 *pValue);

extern ret_t rtl8372n_setAsicRegBits(rtk_uint32 reg, rtk_uint32 bits, rtk_uint32 value);
extern ret_t rtl8372n_getAsicRegBits(rtk_uint32 reg, rtk_uint32 bits, rtk_uint32 *pValue);

extern ret_t rtl8372n_setAsicReg(rtk_uint32 reg, rtk_uint32 value);
extern ret_t rtl8372n_getAsicReg(rtk_uint32 reg, rtk_uint32 *pValue);

#ifdef __cplusplus
}
#endif

#endif