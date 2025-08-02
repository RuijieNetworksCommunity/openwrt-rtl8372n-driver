#include "rtk_error.h"
#include "rtl8372n_asicdrv.h"
#include "rtl8372n_asicdrv_cputag.h"
#include "rtl8372n_switch.h"

ret_t rtl8372n_cpuTag_tpid_set(rtk_uint32 tpid)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if ( tpid > 0xFFFE ) return RT_ERR_INPUT;

    return rtl8372n_setAsicRegBits(0x6038, 0xFFFF, tpid);
}

ret_t rtl8372n_cpuTag_tpid_get(rtk_uint32 *tpid)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    if(!tpid) return RT_ERR_NULL_POINTER;

    return rtl8372n_getAsicRegBits(0x6038u, 0xFFFF, tpid);
}

ret_t rtl8372n_cpuTag_priRemap_set(rtk_uint32 tag_type, rtk_uint32 priority_index, rtk_uint32 priority_value)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT; 
    
    // 步骤2: 验证优先级值范围 (0-7)
    if (priority_value > 7) return RT_ERR_VLAN_PRIORITY;
    
    // 步骤3: 验证优先级索引范围 (0-7)
    if (priority_index > 7) return RT_ERR_VLAN_PRIORITY;
    
    // 步骤4: 验证标签类型范围 (0-1)
    if (tag_type > 1) return RT_ERR_INPUT; 
    
    // 步骤5: 计算寄存器地址
    rtk_uint32 reg_addr = (tag_type == 1) ? 0x51D0 : 0x51CC;
    
    // 步骤6: 计算位域参数
    /*
    寄存器布局:
    每个优先级索引占用4位:
    +-------+-------+-------+-------+-------+-------+-------+-------+
    | 索引7 | 索引6 | 索引5 | 索引4 | 索引3 | 索引2 | 索引1 | 索引0 |
    | [31-28] | [27-24] | [23-20] | [19-16] | [15-12] | [11-8] | [7-4] | [3-0] |
    +-------+-------+-------+-------+-------+-------+-------+-------+
    
    每个索引的低3位存储优先级值
    */
    
    // 步骤7: 设置优先级值
    return rtl8372n_setAsicRegBits(reg_addr, 7 << (4 * priority_index), priority_value);
}

ret_t rtl8372n_cpuTag_priRemap_get(rtk_uint32 tag_type, rtk_uint32 priority_index, rtk_uint32 *priority_value)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT; 

    if(!priority_value) return RT_ERR_NULL_POINTER;
    
    if (priority_index > 7) return RT_ERR_QOS_INT_PRIORITY;

    if (tag_type > 1) return RT_ERR_INPUT; 

    rtk_uint32 reg_addr = (tag_type == 1) ? 0x51D0 : 0x51CC;

    return rtl8372n_getAsicRegBits(reg_addr, 7 << (4 * priority_index), priority_value);
}

ret_t rtl8372n_cpuTag_insertMode_set(rtk_uint32 cpu_port, rtk_uint32 insert_mode)
{
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT; 

    if (insert_mode > 2) return RT_ERR_INPUT; 

    if (cpu_port > 1) return RT_ERR_INPUT;

    if ( cpu_port )
    return rtl8372n_setAsicRegBits(0x6720, 0xC00, insert_mode);
    else
    return rtl8372n_setAsicRegBits(0x6720, 0x300, insert_mode);
}

ret_t rtl8372n_cpuTag_insertMode_get(rtk_uint32 cpu_port, rtk_uint32 *insert_mode)
{
    ret_t result;
    rtk_uint32 reg_value = 0;

    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT; 

    if(!insert_mode) return RT_ERR_NULL_POINTER;

    if (cpu_port > 1) return RT_ERR_INPUT;

    result = rtl8372n_getAsicReg(0x6720u, &reg_value);
    if(result != RT_ERR_OK) return result;

    if (cpu_port == 0) {
        // CPU端口0: 位[9:8]
        *insert_mode = (reg_value >> 8) & 0x3;
    } else {
        // CPU端口1: 位[11:10]
        *insert_mode = (reg_value >> 10) & 0x3;
    }
    
    return result;
}

ret_t rtl8372n_cpuTag_externalCpuPort_set(rtk_uint32 cpu_port)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证CPU端口范围
    /*
    有效端口号:
    - 0-8: 常规端口
    - 15: 特殊端口
    */
    if (cpu_port > 8 && cpu_port != 15) return RT_ERR_PORT_MASK; // 4？
    
    // 步骤3: 设置外部CPU端口寄存器
    /*
    寄存器 0x6724 (外部CPU端口配置寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-4)                | 外部CPU端口号 (位[3:0])     |
    +-------------------------------+-------------------------------+
    
    端口号范围: 0-15 (但仅0-8和15有效)
    */
    return rtl8372n_setAsicRegBits(0x6724u, 0xFu, cpu_port);
}


ret_t rtl8372n_cpuTag_externalCpuPort_get(rtk_uint32 *cpu_port)
{
    if(!cpu_port) return RT_ERR_NULL_POINTER;

    return rtl8372n_getAsicRegBits(0x6724u, 0xFu, cpu_port);
}

ret_t rtl8372n_cpuTag_enable_set(rtk_uint32 tag_type, rtk_uint32 enable)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证参数范围
    if (tag_type > 1 || enable > 1) return RT_ERR_INPUT;
    
    // 步骤3: 读取当前端口掩码
    /*
    寄存器 0x603C (端口掩码寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-10)               | 端口掩码 (位9-0)             |
    +-------------------------------+-------------------------------+
    */
    rtk_uint32 port_mask;
    ret_t result = rtl8372n_getAsicRegBits(0x603Cu, 0x3FFu, &port_mask);
    if (result != RT_ERR_OK) return result;
    
    // 步骤4: 根据标签类型处理
    if (tag_type == 1) { // 外部标签
        // 4.1: 读取外部CPU端口号
        /*
        寄存器 0x6724 (外部CPU端口配置寄存器):
        +-------------------------------+-------------------------------+
        | 保留位 (31-4)                | 外部CPU端口号 (位[3:0])     |
        +-------------------------------+-------------------------------+
        */
        rtk_uint32 ext_cpu_port;
        result = rtl8372n_getAsicRegBits(0x6724u, 0xFu, &ext_cpu_port);
        if (result != RT_ERR_OK) return result;
        
        // 4.2: 更新端口掩码
        if (enable) {
            // 启用: 设置对应端口位
            port_mask |= (1 << ext_cpu_port);
        } else {
            // 禁用: 清除对应端口位
            port_mask &= ~(1 << ext_cpu_port);
        }
        
        // 4.3: 设置外部标签使能位
        /*
        寄存器 0x6720 (CPU标签插入控制寄存器):
        +-------------------------------+-------------------------------+
        | 保留位 (31-12)               | CPU端口1模式 | CPU端口0模式 |
        +-------------------------------+-------------------------------+
        |                               | 位[11:10]   | 位[9:8]     |
        +-------------------------------+-------------------------------+
        位1: 外部标签使能
        */
        result = rtl8372n_setAsicRegBit(0x6720u, 1, enable);
        if (result != RT_ERR_OK) return result;
        
    } else { // 内部标签
        // 4.4: 更新端口掩码
        if (enable) {
            // 启用: 设置端口9 (0x200 = 1<<9)
            port_mask |= 0x200;
        } else {
            // 禁用: 清除端口9
            port_mask &= ~0x200;
        }
        
        // 4.5: 设置内部标签使能位
        /*
        寄存器 0x6720 (CPU标签插入控制寄存器):
        位0: 内部标签使能
        */
        result = rtl8372n_setAsicRegBit(0x6720u, 0, enable);
        if (result != RT_ERR_OK) return result;
        
    }
    
    // 步骤5: 更新端口掩码寄存器
    return rtl8372n_setAsicRegBits(0x603Cu, 0x3FFu, port_mask);
}

ret_t rtl8372n_cpuTag_enable_get(rtk_uint32 tag_type, rtk_uint32 *enable)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证输出指针有效性
    if (!enable) return RT_ERR_NULL_POINTER;
    
    // 步骤3: 验证标签类型范围 (0-1)
    if (tag_type > 1) return RT_ERR_INPUT;
    
    // 步骤4: 读取CPU标签控制寄存器
    /*
    寄存器 0x6720 (CPU标签插入控制寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-12)               | CPU端口1模式 | CPU端口0模式 |
    +-------------------------------+-------------------------------+
    |                               | 位[11:10]   | 位[9:8]     |
    +-------------------------------+-------------------------------+
    位0: 内部标签使能
    位1: 外部标签使能
    */
    rtk_uint32 reg_value;
    ret_t result = rtl8372n_getAsicReg(0x6720, &reg_value);
    if (result != RT_ERR_OK) return result;
    
    // 步骤5: 提取使能状态
    if (tag_type == 0) {
        // 内部标签: 位0
        *enable = reg_value & 0x1;
    } else {
        // 外部标签: 位1
        *enable = (reg_value >> 1) & 0x1;
    }
    
    return RT_ERR_OK; // 成功
}

ret_t rtl8372n_cpuTag_awarePort_set(rtk_portmask_t *port_mask)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;
    
    // 步骤2: 验证输入指针有效性
    if (!port_mask) return RT_ERR_NULL_POINTER;
    
    // 步骤3: 验证端口掩码有效性
    if (rtk_switch_isPortMaskValid(port_mask) != 0) return RT_ERR_PORT_MASK; 
    
    // 步骤4: 将逻辑端口掩码转换为物理端口掩码
    /*
    寄存器 0x603C (CPU标签感知端口寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-10)               | 物理端口掩码 (位[9:0])      |
    +-------------------------------+-------------------------------+
    */
    rtk_uint32 phy_port_mask;
    ret_t result = rtk_switch_portmask_L2P_get(port_mask, &phy_port_mask);
    if (result != RT_ERR_OK) return RT_ERR_FAILED;
    
    // 步骤5: 设置寄存器 (仅使用低10位)
    return rtl8372n_setAsicRegBits(0x603C, 0x3FF, phy_port_mask);
}

ret_t rtl8372n_cpuTag_awarePort_get(rtk_portmask_t *port_mask)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT;

    // 步骤2: 验证输出指针有效性
    if (!port_mask) return RT_ERR_NULL_POINTER; 
    
    // 步骤3: 读取物理端口掩码
    /*
    寄存器 0x603C (CPU标签感知端口寄存器):
    +-------------------------------+-------------------------------+
    | 保留位 (31-10)               | 物理端口掩码 (位[9:0])      |
    +-------------------------------+-------------------------------+
    */
    rtk_uint32 phy_port_mask;
    ret_t result = rtl8372n_getAsicRegBits(0x603C, 0x3FF, &phy_port_mask);
    if (result != RT_ERR_OK) return result;

    // 步骤4: 将物理端口掩码转换为逻辑端口掩码
    result = rtk_switch_portmask_P2L_get(phy_port_mask, port_mask);
    if (result != RT_ERR_OK) return RT_ERR_FAILED;

    
    return RT_ERR_OK; // 成功
}