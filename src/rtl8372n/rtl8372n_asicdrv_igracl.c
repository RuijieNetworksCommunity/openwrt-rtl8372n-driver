#include "rtk_error.h"
#include "rtl8372n_asicdrv.h"
#include "rtl8372n_asicdrv_igracl.h"
#include "rtl8372n_switch.h"

rtl8372n_acltemplate_t acl_template[5] = {
    {{0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x4F}},
    {{0x10, 0x11, 0x12, 0x13, 0x34, 0x35, 0x36, 0x40}},
    {{0x12, 0x13, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48}},
    {{0x10, 0x11, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E}},
    {{0x30, 0x31, 0x32, 0x08, 0x07, 0x41, 0x42, 0x33}}
};

rtk_uint8 filter_fieldSize[] = {
    3, 3, 1, 1, 1, 2, 2, 2, 
    2, 1, 1, 8, 8, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 0, 0, 0, 0, 0
};

rtl8372n_acltemplate_t rtl8372n_filter_fieldTemplateIndex[] = {
    {{0x0,0x1,0x2,0x0,0x0,0x0,0x0,0x0}},
    {{0x3,0x4,0x5,0x0,0x0,0x0,0x0,0x0}},
    {{0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x43,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x44,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x10,0x11,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x12,0x13,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x10,0x11,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x12,0x13,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37}},
    {{0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x14,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x15,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x16,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x15,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x15,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x15,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x16,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x41,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x42,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x47,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x17,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x45,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x46,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x22,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x23,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x25,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x26,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x27,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x32,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x33,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x34,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x35,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x36,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x37,0x0,0x0,0x0,0x0,0x0,0x0,0x0}},
    {{0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0}}
};
/* Function Name:
 *      rtl8367c_setAsicAclTemplate
 * Description:
 *      Set fields of a ACL Template
 * Input:
 *      index   - ACL template index(0~4)
 *      pAclType - ACL type structure for setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - Invalid ACL template index(0~4)
 * Note:
 *      The API can set type field of the 5 ACL rule templates.
 *      Each type has 8 fields. One field means what data in one field of a ACL rule means
 *      8 fields of ACL rule 0~95 is described by one type in ACL group
 */
//不确定对不对，反汇编太乱了
ret_t rtl8372n_setAsicAclTemplate(rtk_uint32 index, rtl8372n_acltemplate_t *pAclType)
{
    ret_t result; // x0

    if ( index > 4 ) return RT_ERR_OUT_OF_RANGE;

    rtk_uint32 reg_value1 = pAclType->field[0] |
                        (pAclType->field[1] << 8) |
                        (pAclType->field[2] << 16) |
                        (pAclType->field[3] << 24);

    rtk_uint32 reg_value2 = pAclType->field[4] |
                        (pAclType->field[5] << 8) |
                        (pAclType->field[6] << 16) |
                        (pAclType->field[7] << 24);

    result = rtl8372n_setAsicReg(8 * index + 0x4824, reg_value1);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicReg(8 * index + 0x4820, reg_value2);
    if (result != RT_ERR_OK) return result;
    
    return RT_ERR_OK;
}

ret_t rtl8372n_igrAcl_cfg_delAll(void)
{
        const rtk_uint32 ACL_CFG_START_ADDR = 0x4848;
        const rtk_uint32 ACL_CFG_END_ADDR = 0x49C4;
        const rtk_uint32 ACL_DISABLE_REG = 0x4810;
        
        if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
        
        for (rtk_uint32 reg_addr = ACL_CFG_START_ADDR; 
            reg_addr <= ACL_CFG_END_ADDR; 
            reg_addr += 4) {
            
            ret_t result = rtl8372n_setAsicReg(reg_addr, 0xFFFFFFFF);
            if (result != RT_ERR_OK) return result;
        }
        
        ret_t result = rtl8372n_setAsicRegBit(ACL_DISABLE_REG, 0, 1);
        if (result != RT_ERR_OK) return result;
    
        return RT_ERR_OK;
}

ret_t rtl8372n_igrAcl_init(void)
{
    ret_t result;

    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 步骤2: 删除所有ACL配置
    result = rtl8372n_igrAcl_cfg_delAll();
    if (result != RT_ERR_OK) return result;

    for(rtk_uint32 index = 0; index < 5; index++){
        result = rtl8372n_setAsicAclTemplate(index, &(acl_template[index]));
        if (result != RT_ERR_OK) return result;
    }

    // 步骤5: 遍历所有物理端口
    for(rtk_uint32 port = 0;port < 10;port++){
        if(((1 << port) & (rtk_uint32)rtk_switch_phyPortMask_get()) == 0) continue;

        result = rtl8372n_setAsicRegBit(0x4818u, port, 1);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_setAsicRegBit(0x481Cu, port, 1);
        if (result != RT_ERR_OK) return result;
    }
    
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicAclIpRange
 * Description:
 *      Set ACL IP range check
 * Input:
 *      index       - ACL IP range check index(0~15)
 *      type        - Range check type
 *      upperIp     - IP range upper bound
 *      lowerIp     - IP range lower bound
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - Invalid ACL IP range check index(0~15)
 * Note:
 *      None
 */
ret_t rtl8372n_igrAcl_ipRange_set(rtk_uint32 index, rtk_uint32 type, rtk_uint32 upperIp, rtk_uint32 lowerIp)
{
    ret_t result;

    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 验证参数有效性
    // 范围类型检查 (0-4)
    // 范围索引检查 (0-15)
    if(type > 4 || index > 0xF) return RT_ERR_OUT_OF_RANGE;

    //验证IP范围有效性 
    if (lowerIp > upperIp) return RT_ERR_INPUT;

    // 计算寄存器基址
    // 每个范围规则占用3个寄存器 (12字节)
    rtk_uint32 base_offset = 12 * index;
    // 设置范围类型寄存器
    result = rtl8372n_setAsicReg(base_offset + 0x4A10, type);
    if (result != RT_ERR_OK) return result;

    // 设置结束IP寄存器
    result = rtl8372n_setAsicReg(base_offset + 0x4A18, lowerIp);
    if (result != RT_ERR_OK) return result;

    // 设置起始IP寄存器
    result = rtl8372n_setAsicReg(base_offset + 0x4A14, upperIp);
    if (result != RT_ERR_OK) return result;
    
    return RT_ERR_OK;
}

/**
 * @brief 向 ACL 规则添加字段
 * 
 * @param rule_head ACL规则链表头指针
 * @param field_data ACL字段数据结构指针
 * @return ret_t 返回状态码
 */
ret_t rtl8372n_igrAcl_field_add(acl_field_t **rule_head, acl_field_t *field_data)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证输入参数
    if (!rule_head || !field_data) return RT_ERR_NULL_POINTER;
    
    // 步骤3: 验证字段类型有效性
    if (field_data->field_type > 0x2A) return RT_ERR_ENTRY_INDEX;
    
    // 步骤4: 初始化字段大小和模板结构
    if (field_data->template.field[0] == 0) {
        // 从全局数组获取字段大小
        field_data->field_size = filter_fieldSize[field_data->field_type];
        
        // 如果字段大小有效，复制模板结构
        if (field_data->field_size > 0) {
            // 获取模板结构
            const rtl8372n_acltemplate_t *template = &rtl8372n_filter_fieldTemplateIndex[field_data->field_type];
            
            // 复制模板结构
            field_data->template = *template;
        }
    }
    
    // 步骤5: 将字段添加到规则链表
    if (*rule_head) {
        // 遍历链表找到末尾
        acl_field_t *current = *rule_head;
        while (current->next) {
            current = current->next;
        }
        
        // 将新字段添加到链表末尾
        current->next = field_data;
    } else {
        // 链表为空，设置为头节点
        *rule_head = field_data;
    }
    
    // 初始化新字段的next指针
    field_data->next = NULL;
    
    return RT_ERR_OK;
}