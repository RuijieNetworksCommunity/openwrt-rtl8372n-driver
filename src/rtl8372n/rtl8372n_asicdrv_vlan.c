#include "include/rtk_error.h"
#include "include/rtl8372n_asicdrv.h"
#include "include/rtl8372n_asicdrv_vlan.h"
#include "include/rtl8372n_switch.h"

ret_t rtl8372n_setAsicVlan4kEntry(rtl8372n_user_vlan4kentry *vlan_entry)
{
    ret_t result;

    // 验证VLAN ID范围 (0-4095)
    if (vlan_entry->vid > 0xFFF) return RT_ERR_VLAN_VID;
    
    // 验证成员端口掩码范围 (0-0x3FF)
    if (vlan_entry->mbr > 0x3FF) return RT_ERR_PORT_MASK;
    
    // 验证未标记端口掩码范围 (0-0x3FF)
    if (vlan_entry->untag > 0x3FF) return RT_ERR_PORT_MASK;
    
    // 验证FID范围 (0-15)
    if (vlan_entry->fid > 0xF) return RT_ERR_L2_FID;
    
    // 步验证使能标志范围 (0-2)
    if (vlan_entry->enable > 2) return RT_ERR_INPUT;
    
    // 验证泄漏标志范围 (0-2)
    if (vlan_entry->leaky > 2) return RT_ERR_INPUT;

    rtk_uint32 reg_value =  ((vlan_entry->enable & 0x1) << 25) | 
                            ((vlan_entry->leaky & 0x1) << 24) | 
                            ((vlan_entry->fid & 0xF) << 20) | 
                            ((vlan_entry->untag & 0x3FF) << 10) | 
                             (vlan_entry->mbr & 0x3FF);

    result = rtl8372n_setAsicReg(0x5CB8u, reg_value);
    if(result != RT_ERR_OK) return result;

    // 发送VLAN配置命令 (0x5CAC)
    reg_value = (vlan_entry->vid << 16) | 0x0303u;
    result = rtl8372n_setAsicReg(0x5CACu, reg_value);
    if(result != RT_ERR_OK) return result;

    do
        result = rtl8372n_getAsicReg(0x5CACu, &reg_value);
    while ( !result && (reg_value & 1) != 0 );// 检查忙标志 (bit0)

    return RT_ERR_OK;
}

ret_t rtl8372n_getAsicVlan4kEntry(rtl8372n_user_vlan4kentry *vlan_entry)
{
    ret_t result;
    rtk_uint32 reg_value;

    // 验证VLAN ID范围 (0-4095)
    if (vlan_entry->vid > 0xFFF) return RT_ERR_VLAN_VID;

    // 等待VLAN命令寄存器空闲
    int busy_flag;
    int retry_count = 10;
    do {
        // 读取忙标志 (0x5CAC寄存器的位0)
        result = rtl8372n_getAsicRegBit(0x5CACu, 0, &busy_flag);
        if(result != RT_ERR_OK) return result;
        
        if (--retry_count == 0) return RT_ERR_BUSYWAIT_TIMEOUT;
    } while (busy_flag); // 忙标志为1时继续等待
    
    // 配置VLAN查询命令
    // 3.1: 设置VLAN ID (0x5CAC寄存器的位[31:16])
    reg_value = (vlan_entry->vid << 16) | 0x0301u;
    result = rtl8372n_setAsicReg(0x5CAC, reg_value);
    if(result != RT_ERR_OK) return result;
    
    // 等待操作完成
    retry_count = 10;
    do {
        result = rtl8372n_getAsicRegBit(0x5CACu, 0, &busy_flag);
        if(result != RT_ERR_OK) return result;

        if (--retry_count == 0) {
        return RT_ERR_BUSYWAIT_TIMEOUT;
        }
    } while (busy_flag); // 忙标志为1时继续等待
    
    // 读取VLAN数据寄存器 (0x5CCC)
    result = rtl8372n_getAsicReg(0x5CCCu, &reg_value);
    if(result != RT_ERR_OK) return result;

    vlan_entry->mbr = reg_value & 0x3FF;          // 成员端口掩码 (位0-9)
    vlan_entry->untag = (reg_value >> 10) & 0x3FF;  // 未标记端口掩码 (位10-19)
    vlan_entry->fid = (reg_value >> 20) & 0xF;    // FID (位20-23)
    vlan_entry->leaky = (reg_value >> 24) & 1;      // 泄漏标志 (位24)
    vlan_entry->enable = (reg_value >> 25) & 1;      // 使能标志 (位25)
    
    return RT_ERR_OK; // 成功
}


ret_t rtl8372n_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证逻辑端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 验证PVID范围 (0-4095)
    if (pvid > 0xFFF) return RT_ERR_VLAN_VID;
    
    rtk_uint32 reg_addr = 0x4E1Cu + ((port >> 1) << 2);
    rtk_uint32 bit_offset = 12 * (port & 0x1u);
    rtk_uint32 bit_mask = 0xFFF << bit_offset;
    
    // 设置寄存器位域
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, pvid);
}

ret_t rtl8372n_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pvid)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证逻辑端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;

    // 验证输出指针有效性
    if (!pvid) return RT_ERR_NULL_POINTER;
    
    // 计算寄存器地址和位域
    /*
    寄存器布局:
    =============================================================
    | 基址寄存器 | 覆盖端口 | 位域偏移 | 位域掩码       | 实际地址  |
    |------------|----------|----------|---------------|----------|
    | REG19996   | port0-1  | 0-11     | 0xFFF << 0    | 19996    |
    |            |          | 12-23    | 0xFFF << 12   |          |
    | REG20000   | port2-3  | 0-11     | 0xFFF << 0    | 20000    |
    |            |          | 12-23    | 0xFFF << 12   |          |
    | REG20004   | port4-5  | 0-11     | 0xFFF << 0    | 20004    |
    |            |          | 12-23    | 0xFFF << 12   |          |
    | REG20008   | port6-7  | 0-11     | 0xFFF << 0    | 20008    |
    |            |          | 12-23    | 0xFFF << 12   |          |
    =============================================================
    */
    int reg_addr = 0x4E1Cu + ((port >> 1) << 2);
    int bit_offset = 12 * (port & 0x1);
    int bit_mask = 0xFFF << bit_offset;
    
    // 步骤5: 读取PVID值
    return rtl8372n_getAsicRegBits(reg_addr, bit_mask, pvid);
}

ret_t rtl8372n_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证逻辑端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 验证标签模式范围 (0-3)
    if (tag_mode >= VLAN_TAG_MODE_END) return RT_ERR_INPUT;
    
    // 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位域分配                |
    |------------|--------------|------------------------|
    | 0x6738     | 端口0-9      | 每个端口占2位          |
    | 0x673C     | 端口10-19    | (未使用)               |
    | 0x6740     | 端口20-29    | (未使用)               |
    =====================================================
    
    端口0: 位[1:0]
    端口1: 位[3:2]
    端口2: 位[5:4]
    ...
    端口9: 位[19:18]
    */
    
    // 计算寄存器地址 (每10个端口一个寄存器)
    rtk_uint32 reg_addr = 0x6738u + (4 * (port / 10));
    
    // 计算位偏移 (每个端口占2位)
    rtk_uint32 bit_offset = 2 * (port % 10);
    
    // 创建位掩码 (0b11 << bit_offset)
    rtk_uint32 bit_mask = 0x3 << bit_offset;

    // 设置寄存器位域
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, tag_mode);
}

ret_t rtl8372n_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *tag_mode)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证逻辑端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证输出指针有效性
    if (!tag_mode) return RT_ERR_NULL_POINTER;
    
    // 步骤4: 计算寄存器地址和位域
    /*
    寄存器布局:
    =============================================================
    | 基址寄存器 | 覆盖端口 | 位域偏移 | 位域掩码       | 实际地址  |
    |------------|----------|----------|---------------|----------|
    | REG26424   | port0-9  | 0-1      | 0x3 << 0      | 26424    |
    |            |          | 2-3      | 0x3 << 2      |          |
    |            |          | ...      | ...           |          |
    |            |          | 18-19    | 0x3 << 18     |          |
    | REG26428   | port10-19| 0-1      | 0x3 << 0      | 26428    |
    |            |          | ...      | ...           |          |
    =============================================================
    
    每个端口占用2位:
        端口0: 位0-1
        端口1: 位2-3
        ...
        端口9: 位18-19
    */
    rtk_uint32 reg_addr = 0x6738u + (4 * (port / 10));
    rtk_uint32 bit_offset = 2 * (port % 10);
    rtk_uint32 bit_mask = 0x3 << bit_offset;
    
    // 步骤5: 读取标签模式值
    ret_t result = rtl8372n_getAsicRegBits(reg_addr, bit_mask, tag_mode);
    if(result != RT_ERR_OK) return result;

    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_init(void)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    ret_t result;
    rtl8372n_user_vlan4kentry vlan_config = {
        .vid = 1,
        .mbr = rtk_switch_phyPortMask_get(),
        .untag = rtk_switch_phyPortMask_get(),
        .fid = 0,
        .leaky = 0,
        .enable = 0
    };

    // 设置默认vlan 1
    result =  rtl8372n_setAsicVlan4kEntry(&vlan_config);
    if(result != RT_ERR_OK) return result;

    for (rtk_port_t port = 0; port < 10; port++) {
        if (!rtk_switch_logicalPortCheck(port)) {
            result = rtl8372n_vlan_portPvid_set(port, 1u);
            if(result != RT_ERR_OK) return result;
            result = rtl8372n_vlan_tagMode_set(port, 0u);
            if(result != RT_ERR_OK) return result;
        }
    }

    for (rtk_port_t port = 0; port < 10; port++) {
        result = rtl8372n_vlan_portIgrFilterEnable_set(port, 1u);
        if(result != RT_ERR_OK) return result;
    }

    // Enable 4k VLAN
    // enable egrFilter
    result = rtl8372n_vlan_egrFilterEnable_set(1); 
    if(result != RT_ERR_OK) return result;
    //L2[0] ?
    result = rtl8372n_setAsicReg(0x4E30u, 0);
    if(result != RT_ERR_OK) return result;
    //L2[1] ?
    result = rtl8372n_setAsicReg(0x4E34u, 0);
    if(result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}

ret_t rtl8372n_vlan_disL2Learn_entry_get(rtk_uint32 entry_index, rtl8372n_disL2Learn_entry *output_data)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 验证条目索引范围 (0-1)
    if (entry_index > 1) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER
    
    // 验证输出指针有效性
    if (!output_data) return RT_ERR_NULL_POINTER;

    // 读取寄存器值
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicReg(0x4E30 + 4 * entry_index, &reg_value);
    if(result != RT_ERR_OK) return result;

    output_data->enable = reg_value & 1;
    output_data->vid = (reg_value >> 1) & 0xFFF;
    output_data->mac_refresh = (reg_value >> 13) & 1;
    
    return RT_ERR_OK;
}

ret_t rtl8372n_vlan_disL2Learn_entry_set(rtl8372n_disL2Learn_entry *entry_config)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证配置指针有效性
    if (!entry_config) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER
    
    // 验证配置参数范围
    if (entry_config->vid > 0xFFF || 
        entry_config->mac_refresh > 1 || 
        entry_config->enable > 1) {
        return RT_ERR_INPUT; // RT_ERR_INPUT
    }
    
    // 读取当前配置
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicReg(0x4E30, &reg_value);
    if(result != RT_ERR_OK) return result;

    // 检查条目0是否匹配
    int entry0_match = 0;
    rtk_uint32 entry0_vid = (reg_value >> 1) & 0xFFF;
    if (entry0_vid == entry_config->vid) {
        entry0_match = 1;
    }
    
    // 读取条目1配置
    result = rtl8372n_getAsicReg(0x4E34, &reg_value);
    if(result != RT_ERR_OK) return result;
    
    // 检查条目1是否匹配
    int entry1_match = 0;
    rtk_uint32 entry1_vid = (reg_value >> 1) & 0xFFF;
    if (entry1_vid == entry_config->vid) {
        entry1_match = 1;
    }
    
    // 确定目标条目索引
    int target_entry = -1;
    
    if (entry0_match) {
        target_entry = 0; // 匹配条目0
    } 
    else if (entry1_match) {
        target_entry = 1; // 匹配条目1
    } 
    else {
        // 查找空闲条目
        if (!(reg_value & 1)) { // 条目1空闲
        target_entry = 1;
        } else {
        target_entry = 0; // 默认使用条目0
        }
    }
    
    // 构建寄存器值
    rtk_uint32 reg_data = 
        (entry_config->enable & 1) | 
        ((entry_config->vid & 0xFFF) << 1) | 
        ((entry_config->mac_refresh & 1) << 13);
    
    // 写入目标条目
    rtk_uint32 reg_addr = 0x4E34 + 4 * target_entry;
    return rtl8372n_setAsicReg(reg_addr, reg_data);
}

ret_t rtl8372n_vlan_egrFilterEnable_set(rtk_uint32 enable)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if (enable > 1) return RT_ERR_ENABLE;

    return rtl8372n_setAsicRegBit(0x4E14u, 2u, enable);
}

ret_t rtl8372n_vlan_egrFilterEnable_get(rtk_uint32 *is_enabled)
{
    ret_t result;

    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if (!is_enabled) return RT_ERR_NULL_POINTER;

    rtk_uint32 reg_data;
    result = rtl8372n_getAsicRegBit(0x4E14u, 2, &reg_data);
    if(result != RT_ERR_OK) return result;

    *is_enabled = reg_data;
    return RT_ERR_OK;
}

ret_t rtl8372n_vlan_get(rtk_uint32 vlan_id, rtk_vlan_cfg_t *config)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证VLAN ID范围 (0-4095)
    if (vlan_id > 0xFFF) return RT_ERR_VLAN_VID;
    
    // 验证配置指针有效性
    if (!config) return RT_ERR_NULL_POINTER;
    
    // 准备查询数据结构
    rtl8372n_user_vlan4kentry query_entry = {
        .vid = vlan_id,
    };
    
    // 查询VLAN 4K表项
    ret_t result = rtl8372n_getAsicVlan4kEntry(&query_entry);
    if(result != RT_ERR_OK) return result;
    
    config->mbr = query_entry.mbr;
    config->untag = query_entry.untag;
    config->fid = query_entry.fid;

    config->leaky = query_entry.leaky;
    config->enable = query_entry.enable;
    
    return 0; // 成功
}


ret_t rtl8372n_vlan_set(rtk_uint32 vlan_id, rtk_vlan_cfg_t *config)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 验证VLAN ID范围 (0-4095)
    if (vlan_id > 0xFFF) return RT_ERR_VLAN_VID;
    
    // 验证配置指针有效性
    if (!config) return RT_ERR_NULL_POINTER;
    
    // 验证成员端口掩码范围 (0-0x3FF)
    if (config->mbr > 0x3FF) return RT_ERR_PORT_MASK;
    
    // 验证未标记端口掩码范围 (0-0x3FF)
    if (config->untag > 0x3FF) return RT_ERR_PORT_MASK;

    // 验证FID范围 (0-15)
    if (config->fid > 0xF) return RT_ERR_L2_FID;
    
    // 步验证使能标志范围 (0-2)
    if (config->enable > 2) return RT_ERR_INPUT;
    
    // 验证泄漏标志范围 (0-2)
    if (config->leaky > 2) return RT_ERR_INPUT;

    rtl8372n_user_vlan4kentry vlan_entry = {
        .vid = vlan_id,
        .mbr = config->mbr & 0x3FF,      // 10位端口掩码
        .untag = config->untag & 0x3FF,  // 10位端口掩码
        .fid = config->fid & 0xF,        // 4位FID
        .leaky = config->leaky & 1,      // 1位泄漏标志
        .enable = config->enable & 1     // 1位使能标志
    };

    // 设置VLAN 4K表项
    return rtl8372n_setAsicVlan4kEntry(&vlan_entry);
}

ret_t rtl8372n_vlan_portFid_set(rtk_port_t port, rtk_uint32 enable, rtk_uint32 fid)
{

    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证端口号范围 (0-9)
    if (port > 9) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证使能标志范围 (0-1)
    if (enable > 1) return RT_ERR_ENABLE;
    
    // 步骤4: 验证FID值范围 (0-15)
    if (fid > 0xF) return RT_ERR_L2_FID;
    
    // 步骤5: 设置FID使能状态
    // 寄存器0x4E38: 每个端口占1位 (端口0=位0, 端口1=位1, ...)
    ret_t result = rtl8372n_setAsicRegBit(0x4E38, port, enable);
    if (result != RT_ERR_OK) return result;
    
    // 步骤6: 计算FID值寄存器地址和位域
    /*
    寄存器布局:
    =============================================================
    | 寄存器地址 | 覆盖端口 | 位域偏移        | 位域掩码         |
    |------------|----------|-----------------|------------------|
    | 0x4E3C     | 端口0-7  | port * 4     | 0xF << (4*port)  |
    | 0x4E40     | 端口8-9  | (port-8)*4      | 0xF << (4*(port-8)) |
    =============================================================
    
    计算规则:
    reg_addr = 0x4E3C + 4 * (port / 8)
    bit_offset = 4 * (port % 8)
    */
    rtk_uint32 reg_addr = 0x4E3Cu + (4 * (port >> 3));
    rtk_uint32 bit_offset = 4 * (port & 7);
    rtk_uint32 bit_mask = 0xF << bit_offset;
    
    // 步骤7: 设置FID值
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, fid << bit_offset);
}

ret_t rtl8372n_vlan_portFid_get(rtk_port_t port, rtk_uint32 *enable, rtk_uint32 *fid)
{
    // 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 验证端口号有效性
    if (port > 9) return RT_ERR_PORT_ID;

    // 验证输出指针有效性
    if (!enable || !fid) return RT_ERR_NULL_POINTER;

    // 步骤4: 获取FID使能状态 (20024寄存器)
    rtk_uint32 enable_status;
    ret_t result = rtl8372n_getAsicRegBit(0x4E38 , port, &enable_status);
    if (result != RT_ERR_OK) return result;

    // 步骤5: 计算FID值寄存器地址和位域
    /*
    寄存器布局：
    ===========================================================================
    | 寄存器地址 | 覆盖端口 | 位域偏移        | 位域宽度 | 位域掩码            |
    |------------|----------|-----------------|----------|--------------------|
    | 0x4E3C     | 端口0-7  | port * 4     | 4比特    | 0xF << (port*4) |
    | 0x4E40     | 端口8-9  | (port-8)*4   | 4比特    | 0xF << ((port-8)*4) |
    ===========================================================================
    
    计算规则：
    reg_addr = 0x4E3C + 4 * (port / 8)
    bit_offset = 4 * (port % 8) 傻逼AI注释，7都能搞成8
    */
    rtk_uint32 reg_addr = 0x4E3Cu + (4 * (port >> 3));  // port/8
    rtk_uint32 bit_offset = 4 * (port & 7);        // port%8
    rtk_uint32 bit_mask = 0xF << bit_offset;
    
    // 步骤6: 获取FID值
    rtk_uint32 fid_val;
    result = rtl8372n_getAsicRegBits(reg_addr, bit_mask, &fid_val);
    if (result != RT_ERR_OK) return result;
    
    // 步骤7: 提取并返回结果
    *enable = enable_status;
    *fid = (fid_val >> bit_offset) & 0xF;
    
    return RT_ERR_OK; // 0 - 操作成功
}

ret_t rtl8372n_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_uint32 enable)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    
    // 步骤2: 验证端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证使能标志范围 (0-1)
    if (enable > 1) return RT_ERR_ENABLE;
    
    // 步骤4: 计算寄存器地址和位位置
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4E18     | 端口0-9      | 每个端口占1位        |
    | 0x4E1C     | 端口10-19    | (未使用)             |
    | 0x4E20     | 端口20-29    | (未使用)             |
    =====================================================
    
    端口0: 位0
    端口1: 位1
    端口2: 位2
    ...
    端口9: 位9
    */
    
    // 计算寄存器地址 (每10个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4E18u + (4 * (port / 10));
    
    // 计算位位置 (端口在组内的偏移)
    rtk_uint32 bit_pos = port % 10;
    
    // 步骤5: 设置寄存器位
    return rtl8372n_setAsicRegBit(reg_addr, bit_pos, enable);
}

ret_t rtl8372n_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_uint32 *enable)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    
    // 步骤2: 验证端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证输出指针有效性
    if (!enable) return RT_ERR_NULL_POINTER;

    // 步骤4: 计算寄存器地址和位位置
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4E18     | 端口0-9      | 每个端口占1位        |
    | 0x4E1C     | 端口10-19    | (未使用)             |
    | 0x4E20     | 端口20-29    | (未使用)             |
    =====================================================
    
    端口0: 位0
    端口1: 位1
    端口2: 位2
    ...
    端口9: 位9
    */
    
    // 计算寄存器地址 (每10个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4E18u + (4 * (port / 10)); // 0x4E18 - 8?
    
    // 计算位位置 (端口在组内的偏移)
    rtk_uint32 bit_pos = port % 10;
    
    // 步骤5: 读取寄存器位
    rtk_uint32 status;
    ret_t result = rtl8372n_getAsicRegBit(reg_addr, bit_pos, &status);
    if (result != RT_ERR_OK) return result;
    
    // 步骤6: 返回使能状态
    *enable = status & 1; // 确保只返回0或1
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_portAcceptFrameType_set(rtk_port_t port, rtl8372n_accframetype frame_type)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 步骤2: 验证端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证帧类型范围 (0-2)
    if (frame_type > FRAME_TYPE_MAX_BOUND) return RT_ERR_VLAN_ACCEPT_FRAME_TYPE;
    
    // 步骤4: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4E10     | 端口0-9      | 每个端口占2位        |
    | 0x4E14     | 端口10-19    | (未使用)             |
    | 0x4E18     | 端口20-29    | (未使用)             |
    =====================================================
    
    端口0: 位[1:0]
    端口1: 位[3:2]
    端口2: 位[5:4]
    ...
    端口9: 位[19:18]
    */
    
    // 计算寄存器地址 (每10个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4E10u + (4 * (port / 10));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 10;
    
    // 计算位位置 (每个端口占2位)
    rtk_uint32 bit_offset = 2 * port_offset;
    
    // 生成位掩码 (0b11 << bit_offset)
    rtk_uint32 bit_mask = 3 << bit_offset;
    
    // 步骤5: 设置寄存器位域
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, frame_type << bit_offset);
}

ret_t rtl8372n_vlan_portAcceptFrameType_get(rtk_port_t port, rtl8372n_accframetype *frame_type)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证端口有效性
    if (rtk_switch_logicalPortCheck(port)) return RT_ERR_PORT_ID;
    
    // 步骤3: 验证输出指针有效性
    if (!frame_type) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4E10     | 端口0-9      | 每个端口占2位        |
    | 0x4E14     | 端口10-19    | (未使用)             |
    | 0x4E18     | 端口20-29    | (未使用)             |
    =====================================================
    
    端口0: 位[1:0]
    端口1: 位[3:2]
    端口2: 位[5:4]
    ...
    端口9: 位[19:18]
    */
    
    // 计算寄存器地址 (每10个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4E10u + (4 * (port / 10));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 10;
    
    // 计算位位置 (每个端口占2位)
    rtk_uint32 bit_offset = 2 * port_offset;
    
    // 生成位掩码 (0b11 << bit_offset)
    rtk_uint32 bit_mask = 3 << bit_offset;
    
    // 步骤5: 读取寄存器位域
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicRegBits(reg_addr, bit_mask, &reg_value);
    if (result != RT_ERR_OK) return result;
    
    // 步骤6: 提取帧类型值
    *frame_type = (reg_value >> bit_offset) & 0x3;
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_portTransparent_set(rtk_port_t port, rtk_vlan_t *transparent_vid)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 步骤2: 验证端口索引范围 (0-9)
    if (port > 9) return RT_ERR_PORT_ID; // RT_ERR_PORT_ID - 无效端口索引
    
    // 步骤3: 验证输入指针有效性
    if (!transparent_vid) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 验证透明VLAN ID范围 (0-1023)
    if (*transparent_vid > 0x3FF) return RT_ERR_VLAN_VID; // RT_ERR_VLAN_VID - 无效VLAN ID
    
    // 步骤5: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4EB8     | 端口0-2      | 每个端口占10位       |
    | 0x4EBC     | 端口3-5      | 每个端口占10位       |
    | 0x4EC0     | 端口6-8      | 每个端口占10位       |
    | 0x4EC4     | 端口9        | 位[9:0]             |
    =====================================================
    
    端口0: 位[9:0]
    端口1: 位[19:10]
    端口2: 位[29:20]
    端口3: 下一个寄存器的位[9:0]
    ...
    端口9: 0x4EC4寄存器的位[9:0]
    */
    
    // 计算寄存器地址 (每3个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4EB8u + (4 * (port / 3));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 3;
    
    // 计算位位置 (每个端口占10位)
    rtk_uint32 bit_offset = 10 * port_offset;
    
    // 生成位掩码 (0x3FF << bit_offset)
    rtk_uint32 bit_mask = 0x3FF << bit_offset;
    
    // 步骤6: 设置透明VLAN ID
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, (*transparent_vid) << bit_offset);
}

ret_t rtl8372n_vlan_portTransparent_get(rtk_port_t port, rtk_vlan_t *transparent_vid)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 步骤2: 验证端口索引范围 (0-9)
    if (port > 9) return RT_ERR_PORT_ID; // RT_ERR_PORT_ID - 无效端口索引
    
    // 步骤3: 验证输出指针有效性
    if (!transparent_vid) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x4EB8     | 端口0-2      | 每个端口占10位       |
    | 0x4EBC     | 端口3-5      | 每个端口占10位       |
    | 0x4EC0     | 端口6-8      | 每个端口占10位       |
    | 0x4EC4     | 端口9        | 位[9:0]             |
    =====================================================
    
    端口0: 位[9:0]
    端口1: 位[19:10]
    端口2: 位[29:20]
    端口3: 下一个寄存器的位[9:0]
    ...
    端口9: 0x4EC4寄存器的位[9:0]
    */
    
    // 计算寄存器地址 (每3个端口一个寄存器)
    rtk_uint32 reg_addr = 0x4EB8u + (4 * (port / 3));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 3;
    
    // 计算位位置 (每个端口占10位)
    rtk_uint32 bit_offset = 10 * port_offset;
    
    // 生成位掩码 (0x3FF << bit_offset)
    rtk_uint32 bit_mask = 0x3FF << bit_offset;
    
    // 步骤5: 读取透明VLAN ID
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicRegBits(reg_addr, bit_mask, &reg_value);
    if (result != RT_ERR_OK) return result;
    
    // 步骤6: 提取透明VLAN ID
    *transparent_vid = (reg_value >> bit_offset) & 0x3FF;
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_keep_set(rtk_port_t port, rtk_vlan_t *keep_vid)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证端口索引范围 (0-9)
    if (port > 9) return RT_ERR_PORT_ID; // RT_ERR_PORT_ID - 无效端口索引
    
    // 步骤3: 验证输入指针有效性
    if (!keep_vid) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 验证保留VLAN ID范围 (0-1023)
    if (*keep_vid > 0x3FF) return RT_ERR_VLAN_VID; // RT_ERR_VLAN_VID - 无效VLAN ID
    
    // 步骤5: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x6728     | 端口0-2      | 每个端口占10位       |
    | 0x672C     | 端口3-5      | 每个端口占10位       |
    | 0x6730     | 端口6-8      | 每个端口占10位       |
    | 0x6734     | 端口9        | 位[9:0]             |
    =====================================================
    
    端口0: 位[9:0]
    端口1: 位[19:10]
    端口2: 位[29:20]
    端口3: 下一个寄存器的位[9:0]
    ...
    端口9: 0x6734寄存器的位[9:0]
    */
    
    // 计算寄存器地址 (每3个端口一个寄存器)
    rtk_uint32 reg_addr = 0x6728u + (4 * (port / 3));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 3;
    
    // 计算位位置 (每个端口占10位)
    rtk_uint32 bit_offset = 10 * port_offset;
    
    // 生成位掩码 (0x3FF << bit_offset)
    rtk_uint32 bit_mask = 0x3FF << bit_offset;
    
    // 步骤6: 设置保留VLAN ID
    return rtl8372n_setAsicRegBits(reg_addr, bit_mask, (*keep_vid) << bit_offset);
}

ret_t rtl8372n_vlan_keep_get(rtk_port_t port, rtk_vlan_t *keep_vid)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证端口索引范围 (0-9)
    if (port > 9) return RT_ERR_PORT_ID; // RT_ERR_PORT_ID - 无效端口索引
    
    // 步骤3:验证输出指针有效性
    if (!keep_vid) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 计算寄存器地址和位域
    /*
    寄存器布局：
    =====================================================
    | 寄存器地址 | 管理端口范围 | 位分配                |
    |------------|--------------|-----------------------|
    | 0x6728     | 端口0-2      | 每个端口占10位       |
    | 0x672C     | 端口3-5      | 每个端口占10位       |
    | 0x6730     | 端口6-8      | 每个端口占10位       |
    | 0x6734     | 端口9        | 位[9:0]             |
    =====================================================
    
    端口0: 位[9:0]
    端口1: 位[19:10]
    端口2: 位[29:20]
    端口3: 下一个寄存器的位[9:0]
    ...
    端口9: 0x6734寄存器的位[9:0]
    */
    
    // 计算寄存器地址 (每3个端口一个寄存器)
    rtk_uint32 reg_addr = 0x6728u + (4 * (port / 3));
    
    // 计算位偏移 (端口在组内的偏移)
    rtk_uint32 port_offset = port % 3;
    
    // 计算位位置 (每个端口占10位)
    rtk_uint32 bit_offset = 10 * port_offset;
    
    // 生成位掩码 (0x3FF << bit_offset)
    rtk_uint32 bit_mask = 0x3FF << bit_offset;
    
    // 步骤5: 读取保留VLAN ID
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicRegBits(reg_addr, bit_mask, &reg_value);
    if (result != RT_ERR_OK) return result;
    
    // 步骤6: 提取保留VLAN ID
    *keep_vid = (reg_value >> bit_offset) & 0x3FF;
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_stg_set(rtk_vlan_t vlan_id, rtk_uint32 stg_index)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证VLAN ID范围 (0-4095)
    if (vlan_id > 0xFFF) return RT_ERR_VLAN_VID; // RT_ERR_VLAN_VID - 无效VLAN ID
    
    // 步骤3: 验证STG索引范围 (0-15)
    if (stg_index > 0xF) return RT_ERR_MSTI; // RT_ERR_STG_ID - 无效STG索引
    
    // 步骤4: 准备VLAN查询结构
    rtl8372n_user_vlan4kentry vlan_entry = {
        .vid = vlan_id,
    };
    
    // 步骤5: 获取当前VLAN配置
    ret_t result = rtl8372n_getAsicVlan4kEntry(&vlan_entry);
    if (result != RT_ERR_OK) return result;
    
    // 更新FID字段 (使用低4位存储STG索引)
    vlan_entry.fid = (vlan_entry.fid & 0xFFF0) | (stg_index & 0xF);
    
    // 步骤7: 更新VLAN配置
    return rtl8372n_setAsicVlan4kEntry(&vlan_entry);
}

ret_t rtl8372n_vlan_stg_get(rtk_vlan_t vlan_id, rtk_uint32 *stg_index)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证VLAN ID范围 (0-4095)
    if (vlan_id > 0xFFF) return RT_ERR_VLAN_VID; // RT_ERR_VLAN_VID - 无效VLAN ID
    
    // 步骤3: 验证输出指针有效性
    if (!stg_index) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤4: 准备VLAN查询结构
    // 步骤4: 准备VLAN查询结构
    rtl8372n_user_vlan4kentry vlan_entry = {
        .vid = vlan_id,
    };
    
    // 步骤5: 获取VLAN配置
    ret_t result = rtl8372n_getAsicVlan4kEntry(&vlan_entry);
    if (result != RT_ERR_OK) return result;
    
    // 提取STG索引 (FID的低4位)
    *stg_index = vlan_entry.fid & 0xF;
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_vlan_reservedVidAction_set(rtk_uint32 vlan0_action,rtk_uint32 vlan4095_action)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证VLAN 0动作范围 (0-1)
    if (vlan0_action > 1) return RT_ERR_INPUT; // RT_ERR_INPUT - 无效输入参数
    
    // 步骤3: 验证VLAN 4095动作范围 (0-1)
    if (vlan4095_action > 1) return RT_ERR_INPUT; // RT_ERR_INPUT - 无效输入参数
    
    // 步骤4: 设置保留VLAN动作
    /*
    寄存器 0x4E14 (全局VLAN控制寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-2)                | VLAN4095动作 | VLAN0动作     |
    +-------------------------------+-------------------------------+
    |                               | 位1         | 位0          |
    +-------------------------------+-------------------------------+
    
    位0: VLAN 0动作 (0=丢弃, 1=转发)
    位1: VLAN 4095动作 (0=丢弃, 1=转发)
    */
    
    // 4.1: 设置VLAN 0动作
    ret_t result = rtl8372n_setAsicRegBit(0x4E14u, 0, vlan0_action);
    if (result != RT_ERR_OK) return result;
    
    // 4.2: 设置VLAN 4095动作
    return rtl8372n_setAsicRegBit(0x4E14, 1, vlan4095_action);
}

ret_t rtl8372n_vlan_reservedVidAction_get(rtk_uint32 *vlan0_action,rtk_uint32 *vlan4095_action)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证输出指针有效性
    if (!vlan0_action || !vlan4095_action) return RT_ERR_NULL_POINTER; // RT_ERR_NULL_POINTER - 空指针错误
    
    // 步骤3: 读取VLAN 0动作
    ret_t result = rtl8372n_getAsicRegBit(0x4E14u, 0, vlan0_action);
    if (result != RT_ERR_OK) return result;
    
    // 步骤4: 读取VLAN 4095动作
    return rtl8372n_getAsicRegBit(0x4E14u, 1, vlan4095_action);
}

ret_t rtl8372n_vlan_realKeepRemarkEnable_set(rtk_uint32 enable)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    if (enable > 1) return RT_ERR_ENABLE;
    return rtl8372n_setAsicRegBit(0x673Cu, 0, enable);
}

ret_t rtl8372n_vlan_realKeepRemarkEnable_get(rtk_uint32 *enable)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    if (!enable) return RT_ERR_NULL_POINTER;
    return rtl8372n_getAsicRegBit(0x673Cu, 0, enable);
}

ret_t rtl8372n_vlan_reset(void)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    return rtl8372n_setAsicRegBit(0x4E14u, 3u, 1);
}