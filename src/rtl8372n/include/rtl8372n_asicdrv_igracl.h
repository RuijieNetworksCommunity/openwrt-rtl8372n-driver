#ifndef _RTL8372N_ASICDRV_IGRACL_H_
#define _RTL8372N_ASICDRV_IGRACL_H_

#include "rtl8372n_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    rtk_uint8 field[8];
}rtl8372n_acltemplate_t;

/**
 * @brief ACL字段数据结构（精确匹配汇编布局）
 * 
 * 结构体布局必须与汇编代码完全一致
 */
typedef struct acl_field {
    rtk_uint32 field_type;          // 0: 字段类型
    rtk_uint32 reserved[17];        // 4-72: 保留区域（17个uint32_t）
    rtk_uint32 field_size;          // 72: 字段大小
    rtl8372n_acltemplate_t template;    // 76: 模板索引数据（8字节）
    rtk_uint32 reserved2[7];         // 84-112: 保留区域（7个uint32_t）
    struct acl_field *next;       // 112: 指向下一个字段的指针
} acl_field_t;

extern ret_t rtl8372n_setAsicAclTemplate(rtk_uint32 index, rtl8372n_acltemplate_t *pAclType);
extern ret_t rtl8372n_igrAcl_cfg_delAll(void);
extern ret_t rtl8372n_igrAcl_ipRange_set(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperIp, rtk_uint32 lowerIp);
extern ret_t rtl8372n_igrAcl_field_add(acl_field_t **rule_head, acl_field_t *field_data);
extern ret_t rtl8372n_igrAcl_init(void);

#ifdef __cplusplus
}
#endif

#endif