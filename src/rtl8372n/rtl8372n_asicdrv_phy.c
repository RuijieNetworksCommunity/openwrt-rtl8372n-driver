#include "include/rtk_error.h"
#include "include/rtl8372n_asicdrv_phy.h"
#include "include/rtl8372n_switch.h"
#include "include/rtl8372n_asicdrv.h"

// 寄存器地址
#define PHY_PORT_SELECT_REG  0x6438  // PHY端口选择寄存器
#define PHY_DATA_REG         0x6444  // PHY数据寄存器
#define PHY_DATA_LOW_REG     0x6440  // PHY数据低位寄存器
#define PHY_CTRL_REG         0x643C  // PHY控制寄存器
#define PHY_STATUS_REG       0x643C  // PHY状态寄存器 (与CTRL相同地址)

// 位掩码
#define PHY_DATA_MASK        0xFFFF  // 数据寄存器位掩码 (低16位)
#define PHY_PORT_MASK        0xFFFF  // 端口索引掩码
#define PHY_OP_STATUS_MASK   0x7000000 // 操作状态位掩码 (bit24-26)

// 命令常量
#define PHY_READ_CMD        0x3     // 读命令 (低3位=011)
#define PHY_WRITE_CMD        0x7     // 写命令 (低3位=111)

// 其他常量
#define MAX_RETRY_COUNT      1000    // 最大重试次数
#define REG_SERDES_CONTROL 0x1Eu //30

ret_t rtl8372n_phy_read(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 *pvalue)
{
    ret_t result;

    // 设置目标PHY端口 (通过数据寄存器高位)
    result = rtl8372n_setAsicRegBits(PHY_DATA_REG, PHY_PORT_MASK, port_index);
    if(result != RT_ERR_OK) return result;

    // 设置PHY访问命令 (0x643C)
    // 命令格式: [31:19] = page, [18:3] = reg_addr*8, [2:0] = 3 (读命令)
    rtk_uint32 command = (page << 19) | (reg_addr << 3) | PHY_READ_CMD;
    result = rtl8372n_setAsicReg(PHY_CTRL_REG, command);
    if(result != RT_ERR_OK) return result;

    // 等待操作完成 (最多1000次重试)
    rtk_uint32 op_status;
    int retry_count = MAX_RETRY_COUNT;
    rtk_uint32 busy_flag;
    while (1) {
        // 检查忙状态位 (bit0)
        result = rtl8372n_getAsicRegBit(PHY_STATUS_REG, 0, &busy_flag);
        if(result != RT_ERR_OK) return result;

        // 检查操作状态位 (bit24)
        result = rtl8372n_getAsicRegBits(PHY_STATUS_REG, PHY_OP_STATUS_MASK, &op_status);
        if(result != RT_ERR_OK) return result;
        
        // 检查是否完成 (忙标志为0且操作状态为0)
        if (!busy_flag && !op_status) {
            // rtl8372n读取结果 (从0x6440寄存器的低16位)
            return rtl8372n_getAsicRegBits(PHY_DATA_LOW_REG, PHY_DATA_MASK, pvalue);
        }
        
        // 减少重试计数
        retry_count--;
        
        // 检查超时
        if (retry_count == 0) {
            return RT_ERR_BUSYWAIT_TIMEOUT; // 10
        }
    }

    return result;
}

ret_t rtl8372n_phy_write(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 value)
{
    ret_t result;

    // 设置目标PHY端口 (0x6438)
    result = rtl8372n_setAsicReg(PHY_PORT_SELECT_REG, port_mask);
    if(result != RT_ERR_OK) return result;

    // 设置要写入的值 (0x6444)
    result = rtl8372n_setAsicRegBits(PHY_DATA_REG, PHY_DATA_MASK, value);
    

    rtk_uint32 command = (page << 19) | (reg_addr * 8) | PHY_WRITE_CMD;
    result = rtl8372n_setAsicReg(PHY_CTRL_REG, command);
    if(result != RT_ERR_OK) return result;

    // 等待操作完成
    rtk_uint32 op_status;
    int retry_count = MAX_RETRY_COUNT;
    rtk_uint32 busy_flag;
    while (1) {
        // 检查忙状态位 (bit0)
        result = rtl8372n_getAsicRegBit(PHY_STATUS_REG, 0, &busy_flag);
        if(result != RT_ERR_OK) break;

        
        // 检查操作状态位 (bit24)
        result = rtl8372n_getAsicRegBits(PHY_STATUS_REG, PHY_OP_STATUS_MASK, &op_status);
        if(result != RT_ERR_OK) break;

        
        // 检查是否完成 (忙标志为0且操作状态为0)
        if (!busy_flag && !op_status) {
          return RT_ERR_OK; // 操作成功
        }
        
        // 减少重试计数
        retry_count--;
        
        // 检查超时
        if (retry_count == 0) {
          return RT_ERR_BUSYWAIT_TIMEOUT; // 10
        }
    }
    return result;
}

ret_t rtl8372n_phy_readBits(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 *pvalue)
{
    int bitsShift = 0;
    ret_t result; 
    rtk_uint32 regData;

    // 验证位掩码有效性
    if((bits >= (1ULL << 16)) || (bits == 0)) return RT_ERR_INPUT;

    // 计算位偏移量 (找到最低有效位)
    while (1) {
        // 检查当前位是否在掩码中置位
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超出32位范围
        if (++bitsShift == 16) {
        return RT_ERR_INPUT; // RT_ERR_INPUT (无效位掩码)
        }
    }

    // 特殊处理页面30 (双寄存器组合)
    if (page == 30) {
        // 读取低位寄存器
        rtk_uint32 low_value;
        result = rtl8372n_phy_read(port_index, 30, reg_addr, &low_value);
        if(result != RT_ERR_OK) return result;
        
        // 读取高位寄存器 (地址+1)
        rtk_uint32 high_value;
        result = rtl8372n_phy_read(port_index, 30, reg_addr + 1, &high_value);
        if(result != RT_ERR_OK) return result;
        
        // 组合32位值 (低位 + 高位<<16)
        regData = low_value | (high_value << 16);
    } 
    // 普通页面处理
    else {
        // 读取单个寄存器
        result = rtl8372n_phy_read(port_index, page, reg_addr, &regData);
        if(result != RT_ERR_OK) return result;
    }
    *pvalue = (regData & bits) >> bitsShift;

    return RT_ERR_OK;
}

ret_t rtl8372n_phy_writeBits(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 value)
{
    int bitsShift = 0;
    ret_t result; 

    // 验证位掩码有效性
    if((bits >= (1ULL << 16)) || (bits == 0)) return RT_ERR_INPUT;

    // 计算位偏移量 (找到最低有效位)
    while (1) {
        // 检查当前位是否在掩码中置位
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超出32位范围
        if (++bitsShift == 32) {
        return RT_ERR_INPUT; // RT_ERR_INPUT (无效位掩码)
        }
    }

    // 将值对齐到正确位置
    rtk_uint32 aligned_value = value << bitsShift;
    
    // 特殊处理页面30 (双寄存器组合)
    if (page == REG_SERDES_CONTROL) {
        int next_reg = reg_addr + 1; // 高位寄存器地址
        
        // 遍历所有可能的端口 (0-8)
        for (int port_index = 0; port_index < 9; port_index++) {
            // 检查端口是否在掩码中
            if ((port_mask & (1 << port_index)) == 0) {
            continue; // 跳过未选择端口
            }
            
            //读取当前32位值 (双寄存器组合)
            rtk_uint32 high_value;
            result = rtl8372n_phy_read(port_index, REG_SERDES_CONTROL, next_reg, &high_value);
            if(result != RT_ERR_OK) return result;
            
            rtk_uint32 low_value;
            result = rtl8372n_phy_read(port_index, REG_SERDES_CONTROL, reg_addr, &low_value);
            if(result != RT_ERR_OK) return result;

            // 组合32位值
            rtk_uint32 combined_value = low_value | (high_value << 16);
            
            // 更新指定位域
            rtk_uint32 new_value = (combined_value & ~bits) | (aligned_value & bits);
            
            // 拆分回两个寄存器
            rtk_uint32 new_high = new_value >> 16;
            rtk_uint32 new_low = new_value & 0xFFFF;
            
            // 写入高位寄存器
            result = rtl8372n_phy_write(1 << port_index, REG_SERDES_CONTROL, next_reg, new_high);
            if(result != RT_ERR_OK) return result;
            
            // 写入低位寄存器
            result = rtl8372n_phy_write(1 << port_index, REG_SERDES_CONTROL, reg_addr, new_low);
            if(result != RT_ERR_OK) return result;
        }
        
        return RT_ERR_OK;
    }

    //普通页面处理 (16位寄存器)
    for (int port_index = 0; port_index < 9; port_index++) {
        // 检查端口是否在掩码中
        if ((port_mask & (1 << port_index)) == 0) {
        continue; // 跳过未选择端口
        }
        
        // 读取当前寄存器值
        rtk_uint32 current_value;
        result = rtl8372n_phy_read(port_index, page, reg_addr, &current_value);
        if(result != RT_ERR_OK) return result;
        
        // 更新指定位域
        rtk_uint32 new_value = (current_value & ~bits) | (aligned_value & bits);
        
        // 写入新值
        result = rtl8372n_phy_write(1 << port_index, page, reg_addr, new_value);
        if(result != RT_ERR_OK) return result;
    }

    return RT_ERR_OK;
}

ret_t rtl8372n_phy_regbits_write(rtk_uint32 port_mask, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 value)
{
    int bitsShift = 0;
    ret_t result; 
    rtk_uint32 regData;

    // 验证位掩码有效性
    if((bits >= (1ULL << 16)) || (bits == 0)) return RT_ERR_INPUT;

    // 计算位偏移量 (找到最低有效位)
    while (1) {
        // 检查当前位是否在掩码中置位
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超出32位范围
        if (++bitsShift == 32) {
        return RT_ERR_INPUT; // RT_ERR_INPUT (无效位掩码)
        }
    }

    // 将值对齐到正确位置
    rtk_uint32 aligned_value = value << bitsShift;
    
    // 遍历所有可能的端口 (0-8)
    for (int port_index = 0; port_index < 9; port_index++) {
        // 检查端口是否在掩码中
        if ((port_mask & (1 << port_index)) == 0) {
        continue; // 跳过未选择端口
        }
        
        // 读取当前寄存器值
        result = rtl8372n_phy_read(port_index, page, reg_addr, &regData);
        if(result != RT_ERR_OK) return result;

        // 更新指定位域
        // 公式: new_value = (regData & ~bits) | (aligned_value & bits)
        regData = (regData & ~bits) | (aligned_value & bits);

        // 写入新值
        result = rtl8372n_phy_write(1 << port_index, page, reg_addr, regData);
        if(result != RT_ERR_OK) return result;
    }
    
    return RT_ERR_OK;

}

ret_t rtl8372n_phy_regbits_read(rtk_uint32 port_index, rtk_uint32 page, rtk_uint32 reg_addr, rtk_uint32 bits, rtk_uint32 *pvalue)
{
    int bitsShift = 0;
    ret_t result; 
    rtk_uint32 regData;

    // 验证位掩码有效性
    if((bits >= (1ULL << 16)) || (bits == 0)) return RT_ERR_INPUT;

    // 计算位偏移量 (找到最低有效位)
    while (1) {
        // 检查当前位是否在掩码中置位
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超出32位范围
        if (++bitsShift == 32) {
        return RT_ERR_INPUT; // RT_ERR_INPUT (无效位掩码)
        }
    }
    result = rtl8372n_phy_read(port_index, page, reg_addr, &regData);
    if(result != RT_ERR_OK) return result;

    rtk_uint32 masked_value = regData & bits;
    *pvalue = masked_value >> bitsShift;
    
    return RT_ERR_OK;
}

ret_t uc1_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
    ret_t result; 
    result = rtl8372n_phy_regbits_write(1 << port, 31, 0xA436, 0xFFFF, addr);
    if(result != RT_ERR_OK) return result;
    return rtl8372n_phy_regbits_write(1 << port, 31, 0xA438, 0xFF00, value);
}

rtk_uint32 uc1_sram_read_8b(rtk_uint32 port, rtk_uint32 addr)
{
    rtk_uint32 value;

    rtl8372n_phy_regbits_write((1 << port), 31LL, 0xA436, 0xFFFFLL, addr);
    rtl8372n_phy_regbits_read(port, 31LL, 0xA438, 0xFF00, &value);
    return value;
}

ret_t uc2_sram_write_8b(rtk_uint32 port, rtk_uint32 addr, rtk_uint32 value)
{
    ret_t result; 
    result = rtl8372n_phy_regbits_write(1 << port, 31, 0xB87C, 0xFFFF, addr);
    if(result != RT_ERR_OK) return result;
    return rtl8372n_phy_regbits_write(1 << port, 31, 0xB87E, 0xFF00, value);
}

rtk_uint32 uc2_sram_read_8b(rtk_uint32 port, rtk_uint32 addr)
{
    rtk_uint32 value;

    rtl8372n_phy_regbits_write((1 << port), 31LL, 0xB87C, 0xFFFFLL, addr);
    rtl8372n_phy_regbits_read(port, 31LL, 0xB87E, 0xFF00, &value);
    return value;
}

ret_t data_ram_write_8b(rtk_uint8 port, rtk_uint32 addr, rtk_uint32 value)
{
    // rtl8372n设置地址寄存器 (47246 = 0xB88E)
    rtl8372n_phy_regbits_write(1 << port, 31, 0xB88E, 0xFFFF, addr);
    // rtl8372n根据地址LSB确定写入位置
    if ( (addr & 1) != 0 )
        // 地址为奇数: 写入低8位
        return rtl8372n_phy_regbits_write(1 << port, 31, 0xB890, 0xFF, value);
    else
        // 地址为偶数: 写入高8位
        return rtl8372n_phy_regbits_write(1 << port, 31, 0xB890, 0xFF00, value);
}

ret_t rtl8372n_phy_common_c45_an_restart(rtk_uint32 port)
{
    ret_t result; // x0
    rtk_uint32 reg_val; // [xsp+2Ch] [xbp+2Ch] BYREF

    result = rtl8372n_phy_read(port, 7, 0, &reg_val);
    if (result != RT_ERR_OK) return result;

    if ( (reg_val & 0x1000) != 0 )
    {
        result = rtl8372n_phy_write(1LL << port, 7, 0, reg_val | 0x200);
        if (result != RT_ERR_OK) return result;
    }

    return RT_ERR_OK;
}

ret_t rtl8372n_phy_common_c45_autoNegoEnable_get(rtk_uint32 port, rtk_uint32 *enable)
{
    ret_t result; // x0
    rtk_uint32 reg_val; // [xsp+2Ch] [xbp+2Ch] BYREF

    result = rtl8372n_phy_read(port, 7, 0, &reg_val);
    if (result != RT_ERR_OK) return result;

    *enable = (reg_val >> 12) & 1;

    return RT_ERR_OK;
}

ret_t rtl8372n_phy_common_c45_autoNegoEnable_set(rtk_uint32 port, rtk_uint32 enable)
{
    ret_t result; // x0
    rtk_uint32 reg_val; // [xsp+2Ch] [xbp+2Ch] BYREF

    reg_val = 0;
    result = rtl8372n_phy_read(port, 7, 0, &reg_val);
    if (result != RT_ERR_OK) return result;

    if ( enable == 1 )
        reg_val = (reg_val & 0xFFFFEDFF) | 0x1200;
    else 
        reg_val = reg_val & 0xFFFFEDFF;

    result = rtl8372n_phy_write(1LL << port, 7, 0, reg_val);
    return result;
}

ret_t rtl8372n_phy_autoNegoAbility_set(rtk_uint32 port, rtl8372n_autoNegoAbility_t *a2)
{
    ret_t result = 0; // x0
    rtk_uint32 port_mask = 0x1u << port; // x22
    rtk_uint32 reg_value; // [xsp+3Ch] [xbp+3Ch] BYREF
    const rtk_uint32 REG_AUTO_NEGOTIATION = 0x07u;
    const rtk_uint32 REG_PHY_CONTROL = 0x1fu;

    result = rtl8372n_phy_read(port, REG_AUTO_NEGOTIATION, 0x10u, &reg_value);
    if (result != RT_ERR_OK) return result;

    reg_value = (((rtk_uint32)a2->data1 << 9) & 0xC00) | (((rtk_uint32)a2->data0 & 0xF) << 5) | (reg_value & 0xFFFFF21Fu);
    result = rtl8372n_phy_write(port_mask, REG_AUTO_NEGOTIATION, 0x10u, reg_value); 
    if (result != RT_ERR_OK) return result;


    result = rtl8372n_phy_read(port, REG_AUTO_NEGOTIATION, 0x20u, &reg_value);
    if (result != RT_ERR_OK) return result;

    reg_value = (((rtk_uint32)a2->data0 << 1) & 0x80) | (reg_value & 0xFFFFEE7Fu);
    result = rtl8372n_phy_write(port_mask, REG_AUTO_NEGOTIATION, 0x20u, reg_value);
    if (result != RT_ERR_OK) return result;


    result = rtl8372n_phy_read(port, REG_PHY_CONTROL, 0xA412u, &reg_value);
    if (result != RT_ERR_OK) return result;

    reg_value = (((rtk_uint32)a2->data0 << 4) & 0x200) | (reg_value & 0xFFFFFDFFu);
    result = rtl8372n_phy_write(port_mask, REG_PHY_CONTROL, 0xA412u, reg_value);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_phy_common_c45_an_restart(port);
    if (result != RT_ERR_OK) return result;

    return RT_ERR_OK;  
}

ret_t rtl8372n_phy_conmmon_c45_autoSpeed_set(rtk_uint32 port, rtk_uint8 *a2)
{
    ret_t result; // x0
    rtl8372n_autoNegoAbility_t v5; // [xsp+28h] [xbp+28h] BYREF

    rtk_uint8 v3 = a2[0];

    v5.data0 = (v3 & 1) | 
        ((2 * ((v3 & 2) != 0LL)) & 0x03) | 
        ((4 * ((v3 & 4) != 0LL)) & 0x07) | 
        ((8 * ((v3 & 8) != 0LL)) & 0xF) | 
        ((16 * ((v3 & 0x10) != 0LL)) & 0x1F) | 
        ((32 * ((v3 & 0x20) != 0LL)) & 0x3F) | 
        ((((v3 & 0x40) != 0LL) << 6) & 0x7F) | 
        (((v3 & 0x80) != 0LL) << 7);

    v5.data1 = (a2[1] & 1) |
            ((2 * ((a2[1] & 2) != 0LL)) & 0xFB) |
            (4 * ((a2[1] & 4) != 0LL));
    result = rtl8372n_phy_common_c45_autoNegoEnable_set(port, 1LL);
    if (result != RT_ERR_OK) return result;

    result =  rtl8372n_phy_autoNegoAbility_set(port, &v5);
    if (result != RT_ERR_OK) return result;

    return RT_ERR_OK;
}
