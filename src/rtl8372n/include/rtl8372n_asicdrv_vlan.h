#ifndef _RTL8372N_ASICDRV_VLAN_H_
#define _RTL8372N_ASICDRV_VLAN_H_

#include "rtl8372n_types.h"

#define RTL8372N_REGBITLENGTH               32

#ifdef __cplusplus
extern "C" {
#endif

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,       // 原始模式
    VLAN_TAG_MODE_KEEP_FORMAT,         // 保持格式模式
    VLAN_TAG_MODE_PRI,                 // 优先级模式
    VLAN_TAG_MODE_REAL_KEEP_FORMAT,    // 真实保持格式模式
    VLAN_TAG_MODE_END                  // 结束标记
} rtk_vlan_tagMode_t;

typedef struct{
    rtk_uint16 vid;        // VLAN ID (0-4095)
    rtk_uint16 mbr;     // 成员端口掩码 (10位)
    rtk_uint16 untag;      // 未标记端口掩码 (10位)
    rtk_uint16 fid;        // FID (0-15) (未使用)
    rtk_uint16 leaky;      // 泄漏标志 (0-2) (未使用)
    rtk_uint16 enable;     // 使能标志 (0-2) (未使用)
}rtl8372n_user_vlan4kentry;

typedef struct{
    rtk_uint32 enable;        // ​​禁用学习标志​​
    rtk_uint32 vid;     // ​​VID​​
    rtk_uint32 mac_refresh;      // ​​禁用刷新标志​​
}rtl8372n_disL2Learn_entry;

typedef struct {
    rtk_uint32 mbr;  // 成员端口掩码
    rtk_uint32 untag;   // 未标记端口掩码
    rtk_uint16 fid;
    rtk_uint16 leaky;               //？
    rtk_uint16 enable;              // VLAN使能标志
}rtk_vlan_cfg_t;

typedef enum
{//不确定是否正确
    FRAME_TYPE_BOTH = 0,
    FRAME_TYPE_TAGGED_ONLY,
    FRAME_TYPE_UNTAGGED_ONLY,
    FRAME_TYPE_MAX_BOUND
} rtl8372n_accframetype; 

extern ret_t rtl8372n_setAsicVlan4kEntry(rtl8372n_user_vlan4kentry *vlan_entry);
extern ret_t rtl8372n_getAsicVlan4kEntry(rtl8372n_user_vlan4kentry *vlan_entry);
extern ret_t rtl8372n_vlan_set(rtk_uint32 vlan_id, rtk_vlan_cfg_t *config);
extern ret_t rtl8372n_vlan_get(rtk_uint32 vlan_id, rtk_vlan_cfg_t *config);
extern ret_t rtl8372n_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid);
extern ret_t rtl8372n_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pvid);
extern ret_t rtl8372n_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);
extern ret_t rtl8372n_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *tag_mode);
extern ret_t rtl8372n_vlan_disL2Learn_entry_set(rtl8372n_disL2Learn_entry *entry_config);
extern ret_t rtl8372n_vlan_disL2Learn_entry_get(rtk_uint32 entry_index, rtl8372n_disL2Learn_entry *output_data);
extern ret_t rtl8372n_vlan_egrFilterEnable_set(rtk_uint32 enable);
extern ret_t rtl8372n_vlan_egrFilterEnable_get(rtk_uint32 *is_enabled);
extern ret_t rtl8372n_vlan_portFid_set(rtk_port_t port, rtk_uint32 enable, rtk_uint32 fid);
extern ret_t rtl8372n_vlan_portFid_get(rtk_port_t port, rtk_uint32 *enable, rtk_uint32 *fid);
extern ret_t rtl8372n_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_uint32 enable);
extern ret_t rtl8372n_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_uint32 *enable);
extern ret_t rtl8372n_vlan_portAcceptFrameType_set(rtk_port_t port, rtl8372n_accframetype frame_type);
extern ret_t rtl8372n_vlan_portAcceptFrameType_get(rtk_port_t port, rtl8372n_accframetype *frame_type);
extern ret_t rtl8372n_vlan_portTransparent_set(rtk_port_t port, rtk_vlan_t *transparent_vid);
extern ret_t rtl8372n_vlan_portTransparent_get(rtk_port_t port, rtk_vlan_t *transparent_vid);
extern ret_t rtl8372n_vlan_keep_set(rtk_port_t port, rtk_vlan_t *keep_vid);
extern ret_t rtl8372n_vlan_keep_get(rtk_port_t port, rtk_vlan_t *keep_vid);
extern ret_t rtl8372n_vlan_stg_set(rtk_vlan_t vlan_id, rtk_uint32 stg_index);
extern ret_t rtl8372n_vlan_stg_get(rtk_vlan_t vlan_id, rtk_uint32 *stg_index);
extern ret_t rtl8372n_vlan_reservedVidAction_set(rtk_uint32 vlan0_action,rtk_uint32 vlan4095_action);
extern ret_t rtl8372n_vlan_reservedVidAction_get(rtk_uint32 *vlan0_action,rtk_uint32 *vlan4095_action);
extern ret_t rtl8372n_vlan_realKeepRemarkEnable_set(rtk_uint32 enable);
extern ret_t rtl8372n_vlan_realKeepRemarkEnable_get(rtk_uint32 *enable);

extern ret_t rtl8372n_vlan_reset(void);
extern ret_t rtl8372n_vlan_init(void);

#ifdef __cplusplus
}
#endif

#endif