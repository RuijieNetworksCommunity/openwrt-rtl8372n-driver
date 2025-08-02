#ifndef _RTL8372N_ASICDRV_CPUTAG_H_
#define _RTL8372N_ASICDRV_CPUTAG_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ret_t rtl8372n_cpuTag_awarePort_set(rtk_portmask_t *port_mask);
extern ret_t rtl8372n_cpuTag_awarePort_get(rtk_portmask_t *port_mask);
extern ret_t rtl8372n_cpuTag_enable_set(rtk_uint32 tag_type, rtk_uint32 enable);
extern ret_t rtl8372n_cpuTag_enable_get(rtk_uint32 tag_type, rtk_uint32 *enable);
extern ret_t rtl8372n_cpuTag_externalCpuPort_set(rtk_uint32 cpu_port);
extern ret_t rtl8372n_cpuTag_externalCpuPort_get(rtk_uint32 *cpu_port);
extern ret_t rtl8372n_cpuTag_insertMode_set(rtk_uint32 cpu_port, rtk_uint32 insert_mode);
extern ret_t rtl8372n_cpuTag_insertMode_get(rtk_uint32 cpu_port, rtk_uint32 *insert_mode);
extern ret_t rtl8372n_cpuTag_priRemap_set(rtk_uint32 tag_type, rtk_uint32 priority_index, rtk_uint32 priority_value);
extern ret_t rtl8372n_cpuTag_priRemap_get(rtk_uint32 tag_type, rtk_uint32 priority_index, rtk_uint32 *priority_value);
extern ret_t rtl8372n_cpuTag_tpid_set(rtk_uint32 tpid);
extern ret_t rtl8372n_cpuTag_tpid_get(rtk_uint32 *tpid);

#ifdef __cplusplus
}
#endif

#endif