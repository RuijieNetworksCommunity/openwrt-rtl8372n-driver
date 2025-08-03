#include "include/rtk_error.h"
#include "include/rtl8372n_asicdrv.h"
#include "include/rtl8372n_asicdrv_sds.h"
#include "include/rtl8372n_asicdrv_phy.h"
#include "include/rtl8372n_switch.h"

#include <linux/delay.h>
#include <linux/printk.h>

void delay_loop(void){
    mdelay(10);
}

struct PatchEntry {
    rtk_uint16 reg;
    rtk_uint16 page;
    rtk_uint16 val;
}; // 6 bytes 每组配置

ret_t rtl8372n_sds_reg_read(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 *pvalue)
{
    ret_t result;

    // 等待SDS控制器空闲 (最多1000次尝试)
    int retry_count = 1000;
    rtk_uint32 busy_flag;

    while (1) {
        // 读取忙标志位 (寄存器0x3F8的位15)
        result = rtl8372n_getAsicRegBit(0x3F8u, 0xFu, &busy_flag);
        if (result != RT_ERR_OK) return result;

        // 检查是否空闲
        if (!busy_flag) break; // 空闲状态，继续操作
        
        // 减少重试计数
        if (--retry_count == 0) return RT_ERR_BUSYWAIT_TIMEOUT; // RT_ERR_BUSYWAIT_TIMEOUT (超时错误)
    }

    //设置通道号 (寄存器0x3F8的位0)
    result = rtl8372n_setAsicRegBit(0x3F8u, 0u, sdsId);
    if (result != RT_ERR_OK) return result;
    
    // 设置主地址 (寄存器0x3F8的位[6:1])
    result = rtl8372n_setAsicRegBits(0x3F8u, 0x7Eu, sdsReg); // 126 = 0b1111110
    if (result != RT_ERR_OK) return result;
    
    // 设置次地址 (寄存器0x3F8的位[11:7])
    result = rtl8372n_setAsicRegBits(0x3F8u, 0xF80u, sdsPage); // 3968 = 0b111110000000
    if (result != RT_ERR_OK) return result;
    
    result = rtl8372n_setAsicRegBit(0x3F8u, 0xEu, 0);
    if (result != RT_ERR_OK) return result;
    
    result = rtl8372n_setAsicRegBit(0x3F8u, 0xFu, 1);
    if (result != RT_ERR_OK) return result;

    retry_count = 1000;
    while (1) {
        result = rtl8372n_getAsicRegBit(0x3F8u, 0xFu, &busy_flag);
        if (result != RT_ERR_OK) return result;

        // 7.1 检查是否完成
        if (!busy_flag) {
            // 读取数据寄存器 (0x3FC)
            return rtl8372n_getAsicReg(0x3FCu, pvalue);
        }

        // 7.2 减少重试计数
        if (--retry_count == 0) {
            // 超时处理: 尝试读取数据寄存器
            result = rtl8372n_getAsicReg(0x3FCu, pvalue);
            if (result != RT_ERR_OK) return result; // 读取错误
            return RT_ERR_BUSYWAIT_TIMEOUT; // RT_ERR_BUSYWAIT_TIMEOUT (超时错误)
        }
    }
}

ret_t rtl8372n_sds_reg_write(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 value)
{
    ret_t result;

    // 等待SDS控制器空闲 (最多1000次尝试)
    int retry_count = 1000;
    rtk_uint32 busy_flag;

    while (1) {
        // 读取忙标志位 (寄存器0x3F8的位15)
        result = rtl8372n_getAsicRegBit(0x3F8u, 0xFu, &busy_flag);
        if (result != RT_ERR_OK) return result;

        // 检查是否空闲
        if (!busy_flag) break; // 空闲状态，继续操作
        
        // 减少重试计数
        if (--retry_count == 0) return RT_ERR_BUSYWAIT_TIMEOUT; // RT_ERR_BUSYWAIT_TIMEOUT (超时错误)
    }

    //写入数据到数据寄存器 (0x400)
    result = rtl8372n_setAsicReg(0x400u, value);
    if (result != RT_ERR_OK) return result;

    //设置通道号 (寄存器0x3F8的位0)
    result = rtl8372n_setAsicRegBit(0x3F8u, 0u, sdsId);
    if (result != RT_ERR_OK) return result;

    //设置主地址 (寄存器0x3F8的位[6:1])
    result = rtl8372n_setAsicRegBits(0x3F8u, 0x7Eu, sdsReg);
    if (result != RT_ERR_OK) return result;

    //设置次地址 (寄存器0x3F8的位[11:7])
    result = rtl8372n_setAsicRegBits(0x3F8u, 0xF80u, sdsPage);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x3F8u, 0xEu, 1);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_setAsicRegBit(0x3F8u, 0xFu, 1);
    if (result != RT_ERR_OK) return result;

    //等待操作完成 (最多1000次尝试)
    retry_count = 1000;
    while (1) {
        result = rtl8372n_getAsicRegBit(0x3F8u, 0xFu, &busy_flag);
        if (result != RT_ERR_OK) return result;
        
        //检查是否完成
        if (!busy_flag) return RT_ERR_OK; // 操作成功完成
        
        //减少重试计数
        if (--retry_count == 0) return RT_ERR_BUSYWAIT_TIMEOUT; // 超时错误
    }
}

ret_t rtl8372n_sds_regbits_read(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 bits, rtk_uint32 *pvalue)
{

    // 验证位掩码有效性
    if (bits == 0) {
        return RT_ERR_INPUT; //无效位掩码
    }

    int bitsShift = 0;
    // 计算位偏移量
    while (1) {
        // 检查当前位是否在掩码中
        if (bits & (1 << bitsShift)) break; // 找到最低有效位
        
        // 检查是否超过32位
        if (++bitsShift == RTL8372N_REGBITLENGTH) return RT_ERR_INPUT; //无效位掩码
    }

    // 读取整个寄存器值
    rtk_uint32 regData;
    ret_t result = rtl8372n_sds_reg_read(sdsId, sdsReg, sdsPage, &regData);
    if (result != RT_ERR_OK) return result;

    *pvalue = (regData & bits) >> bitsShift;
    return RT_ERR_OK;
}

ret_t rtl8372n_sds_regbits_write(rtk_uint32 sdsId, rtk_uint32 sdsReg, rtk_uint32 sdsPage, rtk_uint32 bits, rtk_uint32 value)
{

    // 验证位掩码有效性
    if (bits == 0) return RT_ERR_INPUT; //无效位掩码
    

    int bitsShift = 0;
    // 计算位偏移量
    while (1) {
        // 检查当前位是否在掩码中
        if (bits & (1 << bitsShift)) break; // 找到最低有效位
        
        // 检查是否超过32位
        if (++bitsShift == RTL8372N_REGBITLENGTH) return RT_ERR_INPUT; //无效位掩码
    }

    // 读取整个寄存器值
    rtk_uint32 regData;
    ret_t result = rtl8372n_sds_reg_read(sdsId, sdsReg, sdsPage, &regData);
    if (result != RT_ERR_OK) return result;

    // 修改位域值
    regData = regData & (~bits);
    regData = regData | ((value << bitsShift) & bits);

    result = rtl8372n_sds_reg_write(sdsId, sdsReg, sdsPage, regData);
    return RT_ERR_OK;
}

ret_t rtl8372n_sdsMode_set(rtk_uint32 sdsId, rtk_uint32 mode)
{
    // 步骤1: 检查交换机初始化状态
    if (rtk_switch_initialState_get() != 1) return RT_ERR_NOT_INIT; 
    
    // 步骤2: 读取芯片ID并确定芯片类型
    rtk_uint32 chip_id;
    ret_t result = rtl8372n_getAsicReg(4, &chip_id);
    if (result != RT_ERR_OK) return result;
    chip_id = chip_id >> 8; // 提取高16位芯片ID
    switch_chip_t chip_type = 1; // 默认芯片类型索引
    
    // 步骤3: 根据芯片ID确定芯片类型
    switch (chip_id) {
        case 0x837300: // rtl8373
            // 步骤3.1: 验证通道和模式组合有效性
            if (sdsId == 0) {
                // 通道0只支持模式0
                if (mode != 0) {
                return RT_ERR_NOT_ALLOWED;
                }
            } else if (sdsId == 1) {
                // 通道1支持模式0和31
                if (mode != 0 && mode != 31) {
                return RT_ERR_NOT_ALLOWED;
                }
            } else {
                return RT_ERR_PORT_ID; // RT_ERR_PORT_ID
            }
            chip_type = CHIP_RTL8373; // rtl833
            break;
        case 0x837200: // RTL8372
        chip_type = CHIP_RTL8372;
        break;
        
        case 0x822400: // RTL8224
        chip_type = CHIP_RTL8224;
        break;
        
        case 0x837370: // rtl8372n
        chip_type = CHIP_RTL8372N;
        break;
        
        case 0x837270: // RTL8372n
        chip_type = CHIP_RTL8372N;
        break;
        
        case 0x822470: // RTL8224n
        chip_type = CHIP_RTL8224N;
        break;
        
        default:
        // 其他芯片类型处理
        if (chip_id == 0x8366A8) {
            chip_type = CHIP_RTL8366; // RTL8366特殊版本?
        } else {
            return RT_ERR_CHIP_NOT_FOUND;
        }
    }
    rtk_uint32 v10;
    rtk_uint32 v12;
    rtk_uint32 v13;
    if(mode == 0x1f)
    {
    // CH0_MODE31_HANDLER:
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0x30u, 3);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0x30u, 1);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC0u, 1);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC0u, 3);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC00u, 3);
        delay_loop();
        v12 = 0x1u;
        v13 = 0xC00u;
        goto APPLY_CONFIG;
    }

    if(mode != 0x1f && mode != 0x22 && mode !=0x21)
    {
        rtl8372n_setAsicRegBit(0x7B20u, 0x15u, 0);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0x30u, 3);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0x30u, 1);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC0u, 1);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC0u, 3);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC00u, 3);
        delay_loop();
        rtl8372n_sds_regbits_write(0, 0x20u, 0, 0xC00u, 1);
        delay_loop();
        SDS_MODE_SET_SW(chip_type, 0, mode);// 设置最终模式
        goto FINAL_CONFIG;
    }

APPLY_CONFIG:
      rtl8372n_sds_regbits_write(0, 0x20u, 0, v13, v12);
      delay_loop();
      goto POST_DELAY;

POST_DELAY:
        delay_loop();
LABEL_33:
        if ( mode != 0xD )                      // 特殊模式0xd处理 (保留)
        {
// PHY_RESET_START:
          fw_reset_flow_tgx(sdsId);             // TGX 复位流程
          goto FINAL_DELAY;
        }
TGR_RESET_START:
        fw_reset_flow_tgr(sdsId);               // TGR 复位流程
FINAL_DELAY:
        delay_loop();
        return 0LL;
FINAL_CONFIG:
        delay_loop();
        if ( mode )                           // 根据模式执行最终复位流程
            v10 = mode == 0x1A;
        else
            v10 = 1;
        if ( v10 )
            goto TGR_RESET_START;               // mode==0
        goto LABEL_33;
    return 0LL;
}

ret_t rtl8372n_sdsMode_get(rtk_uint32 sdsId, rtk_uint32 *pMode)
{

    rtk_uint32 bitfield1 = 0;
    rtk_uint32 bitfield2 = 0;

    // 验证通道号并选择寄存器位域
    if (sdsId == 0) {
        // 通道0：寄存器0x7B20的位域配置
        rtl8372n_getAsicRegBits(0x7B20, 0x1F,   &bitfield1);  // 低5位 (bit[4:0])
        rtl8372n_getAsicRegBits(0x7B20, 0x7C00u, &bitfield2);  // 位[14:10]
    } else if (sdsId == 1) {
        // 通道1：寄存器0x7B20的位域配置
        rtl8372n_getAsicRegBits(0x7B20, 0x3E0u,    &bitfield1);  // 位[9:5]
        rtl8372n_getAsicRegBits(0x7B20, 0x1F0000u, &bitfield2);  // 位[20:16]
    } else {
        return RT_ERR_INPUT; // 无效通道，返回错误
    }


    // 特殊模式13处理（USXGMII兼容模式）
    if (bitfield1 == 0xD) { // 13 (0xD)
        if (bitfield2 == 0) {
            *pMode = 0xD; // 标准模式13
            return RT_ERR_OK;
        } else if (bitfield2 == 2) {
            *pMode = 0;   // 特殊兼容模式0
            return RT_ERR_OK;
        } else {
            return RT_ERR_FAILED;      // 无效的辅助位域值
        }
    }

    // 预定义模式值处理
    if (bitfield1 != 0x1A &&  // 26 (0x1A)
        bitfield1 != 0x12 &&  // 18 (0x12)
        bitfield1 != 0x16 &&  // 22 (0x16)
        bitfield1 != 0x2  &&  // 2
        bitfield1 != 0x4  &&  // 4
        bitfield1 != 0x5)     // 5
    {
        // 特殊模式31处理
        if (bitfield1 == 0x1F) { // 31 (0x1F)
            *pMode = 0x1F;
            return RT_ERR_OK;
        }
        
        // 未识别模式值
        return RT_ERR_OK; // 返回成功但模式值未定义
    }

    // 返回标准模式值
    *pMode = bitfield1;
    return RT_ERR_OK; // 成功返回
}

ret_t cfg_rl6637_sds_mode(rtk_uint8 phy_port, rtk_uint32 sds_mode)
{
    // 步骤1: 创建端口位掩码
    rtk_uint32 port_mask = 1 << phy_port;
    
    // 步骤2: 初始化配置序列
    // 2.1 禁用特殊功能 (页面30, 寄存器30195, 位0)
    rtl8372n_phy_regbits_write(port_mask, 0x1Eu, 0x75F3u, 1u, 0);
    
    // 步骤3: 设置核心SDS模式
    // 页面30, 寄存器27002, 位[5:0] = sds_mode
    rtl8372n_phy_regbits_write(port_mask, 0x1Eu, 0x697Au, 0x3Fu, sds_mode);
    
    // 步骤4: 启用高级功能
    // 4.1 页面31, 寄存器42034, 位5 = 1
    rtl8372n_phy_regbits_write(port_mask, 0x1Fu, 0xA432u, 0x20u, 1);
    
    // 4.2 页面7, 寄存器62, 位0 = 1
    rtl8372n_phy_regbits_write(port_mask, 0x7u, 0x3E, 1, 1);
    
    // 步骤5: 时钟配置
    // 页面31, 寄存器42050, 位[3:2] = 0
    rtl8372n_phy_regbits_write(port_mask, 0x1Fu, 0xA442u, 0xC, 0);
    
    // 步骤6: 设置参考时钟
    // 页面30, 寄存器30133 = 0xE084
    rtl8372n_phy_write(port_mask, 0x1Eu, 0x75B5u, 0xE084u);
    
    // 步骤7: 均衡器配置
    // 页面30, 寄存器30130, 位[6:5] = 3
    rtl8372n_phy_regbits_write(port_mask, 0x1Eu, 0x75B2u, 0x60u, 3);
    
    // 步骤8: 增益控制
    // 页面31, 寄存器53312, 位[9:8] = 2
    rtl8372n_phy_regbits_write(port_mask, 0x1Fu, 0xD040u, 0x300u, 2);
    
    // 步骤9: 启动序列
    // 9.1 设置启动位 (页面31, 寄存器41984, 位14 = 1)
    rtl8372n_phy_regbits_write(port_mask, 0x1Fu, 0xA400u, 0x4000u, 1);
    
    // 9.2 清除启动位 (页面31, 寄存器41984, 位14 = 0)
    return rtl8372n_phy_regbits_write(port_mask, 0x1Fu, 0xA400u, 0x4000u, 0);
}

ret_t SDS_MODE_SET_SW(switch_chip_t chip_type, rtk_uint32 sdsId, rtk_uint32 mode)
{
    // === RTL8224系列芯片特殊处理 ===
    if (chip_type == CHIP_RTL8224N || chip_type == CHIP_RTL8224) { // 识别RTL8224系列芯片
        /*  ***TODO**

        // 步骤1: 配置模式寄存器
        if (mode) {
        // 1.1 特定配置模式
        if (sdsId) {
            // 通道1配置:
            if (sdsId == 1) {
            // 清除高位配置 (0x7B20寄存器的bits[20:16])
            dal_rtl8224_top_regbits_write(0x7B20, 0x1F0000, 0);
            // 设置模式值 (0x7B20寄存器的bits[9:5])
            dal_rtl8224_top_regbits_write(0x7B20, 0x3E0, mode);
            }
        } else {
            // 通道0配置:
            // 清除高位配置 (0x7B20寄存器的bits[14:10])
            dal_rtl8224_top_regbits_write(0x7B20, 0x7C00, 0);
            // 设置模式值 (0x7B20寄存器的bits[4:0])
            dal_rtl8224_top_regbits_write(0x7B20, 0x1F, mode);
        }
        } else {
        // 1.2 默认配置模式
        if (sdsId) {
            // 通道1默认配置:
            if (sdsId == 1) {
            // 设置高位默认值 (0x7B20寄存器的bits[20:16]=2)
            dal_rtl8224_top_regbits_write(0x7B20, 0x1F0000, 2);
            // 设置默认模式值 (0x7B20寄存器的bits[9:5]=13)
            dal_rtl8224_top_regbits_write(0x7B20, 0x3E0, 13);
            }
        } else {
            // 通道0默认配置:
            // 设置高位默认值 (0x7B20寄存器的bits[14:10]=2)
            dal_rtl8224_top_regbits_write(0x7B20, 0x7C00, 2);
            // 设置默认模式值 (0x7B20寄存器的bits[4:0]=13)
            dal_rtl8224_top_regbits_write(0x7B20, 0x1F, 13);
        }
        }
        
        // 步骤2: 复位控制寄存器
        // 复位bit15
        dal_rtl8224_top_regbit_write(0x7B20, 0x15, 0);
        // 复位bit16
        dal_rtl8224_top_regbit_write(0x7B20, 0x16, 0);
        
        // 步骤3: 应用SerDes补丁
        serdes_patch(chip_type, sdsId, mode);
        
        // 步骤4: SerDes配置序列 (17步)
        // 4.1 配置bit[5:4] (0x30)
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0x30, 3);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0x30, 1);
        
        // 4.2 配置bit[7:6] (0xC0)
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC0, 1);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC0, 3);
        
        // 4.3 配置bit[11:10] (0xC00)
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC00, 3);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC00, 1);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC00, 1); // 重复步骤
        
        // 4.4 修改bit[11:10]配置
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC00, 3);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC00, 0);
        
        // 4.5 重新配置bit[7:6]
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC0, 3);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC0, 1);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0xC0, 0);
        
        // 4.6 重新配置bit[5:4]
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0x30, 1);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0x30, 3);
        dal_rtl8224_sds_regbits_write(sdsId, 32, 0, 0x30, 0);
        
        // 4.7 特殊寄存器31配置
        dal_rtl8224_sds_regbits_write(sdsId, 31, 0, 0xFFFF, 11);
        dal_rtl8224_sds_regbits_write(sdsId, 31, 0, 0xFFFF, 0);
        */
        return RT_ERR_OK; // 成功
    }
    
    // === 其他芯片系列处理 ===
    // 步骤1: 配置模式寄存器 (与RTL8224类似但使用通用函数)
    // if (mode) {
    //     // 1.1 特定配置模式
    //     if (sdsId) {
    //         // 通道1配置:
    //         if (sdsId == 1) {
    //             // 清除高位配置
    //             rtl8372n_setAsicRegBits(0x7B20, 0x1F0000, 0);
    //             // 设置模式值
    //             rtl8372n_setAsicRegBits(0x7B20, 0x3E0, mode);
    //     }
    //     } else {
    //         // 通道0配置:
    //         // 清除高位配置
    //         rtl8372n_setAsicRegBits(0x7B20, 0x7C00, 0);
    //         // 设置模式值
    //         rtl8372n_setAsicRegBits(0x7B20, 0x1F, mode);
    //     }
    // } else {
    //     // 1.2 默认配置模式
    //     if (sdsId) {
    //         // 通道1默认配置:
    //         if (sdsId == 1) {
    //             // 设置高位默认值
    //             rtl8372n_setAsicRegBits(0x7B20u, 0x1F0000u, 2u);
    //             // 设置默认模式值
    //             rtl8372n_setAsicRegBits(0x7B20u, 0x3E0u, 13u);
    //         }
    //     } else {
    //         // 通道0默认配置:
    //         // 设置高位默认值
    //         rtl8372n_setAsicRegBits(0x7B20u, 0x7C00u, 2u);
    //         // 设置默认模式值
    //         rtl8372n_setAsicRegBits(0x7B20u, 0x1Fu, 13u);
    //     }
    // }
    
    rtl8372n_setAsicReg(0x7B20u, 0x3fau); //重要模式参数

    // 步骤2: 应用SerDes补丁
    serdes_patch(chip_type, sdsId, mode);
    
    // 步骤3: 高级功能配置
    fiber_fc_en(sdsId, mode, 1); // 启用光纤流控
    sds_nway_set(sdsId, mode, 1); // 设置SDS NWay协商
    
    // 步骤4: SerDes配置序列
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0x30u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0x30u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC0u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC0u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC00u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC00u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC00u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC00u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC00u, 0u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC0u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC0u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0xC0u, 0u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0x30u, 1u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0x30u, 3u);
    rtl8372n_sds_regbits_write(sdsId, 0x20u, 0u, 0x30u, 0u);
    rtl8372n_sds_regbits_write(sdsId, 0x1Fu, 0u, 0xFFFFu, 11u);
    rtl8372n_sds_regbits_write(sdsId, 0x1Fu, 0u, 0xFFFFu, 0u);
    
    return RT_ERR_OK; // 成功
}

ret_t fw_reset_flow_tgx(rtk_uint32 sdsId)
{
    ret_t result;

    rtk_uint32 status_value;   // 状态寄存器值
    rtk_uint32 control_reg;     // 控制寄存器值

    // 读取SDS状态寄存器 (地址32，偏移0，掩码0x30)
    // 0x30 = 0b110000 (位5-4)
    result = rtl8372n_sds_regbits_read(sdsId, 0x20u, 0u, 0x30u, &status_value);
    if (result != RT_ERR_OK) return result;
    
    // 检查状态寄存器值
    if (status_value == 1) return RT_ERR_OK; // 状态正常，无需复位
    
    // 读取控制寄存器1 (地址1，偏移29)
    result = rtl8372n_sds_reg_read(sdsId, 1u, 0x1Du, &control_reg);
    if (result != RT_ERR_OK) return result;

    // 检查控制寄存器状态
    // 检查位8 (0x100) 是否置位
    // 检查位0 (0x1) 是否清空 或 位4 (0x10) 是否清空
    if ((control_reg & 0x100u) != 0 && 
        ((control_reg & 0x1u) == 0 || (control_reg & 0x10u) == 0)) {
        
        // 执行复位序列
        rtl8372n_sds_regbits_write(sdsId, 0u, 0u, 0x2u, 1u);
        rtl8372n_sds_regbits_write(sdsId, 0u, 0u, 0x2u, 0u);
        rtl8372n_sds_regbits_write(sdsId, 0u, 0u, 0x2u, 1u);
    }

    return RT_ERR_OK;
}

ret_t fw_reset_flow_tgr(rtk_uint32 sdsId)
{
    ret_t result; // x0
    rtk_uint32 reg_value; // [xsp+28h] [xbp+28h] BYREF

    result = rtl8372n_sds_regbits_read(sdsId, 0x20u, 0, 0x30u, &reg_value);
    if(result != RT_ERR_OK) return result;
    if(reg_value == 1) return RT_ERR_OK;

    result = rtl8372n_sds_regbits_write(sdsId, 0x21u, 0, 4u, 1);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_sds_regbits_write(sdsId, 0x36u, 5, 0x7800u, 8);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_sds_reg_write(sdsId, 0x1Fu, 2, 0x1Fu);
    if (result != RT_ERR_OK) return result;

    result = rtl8372n_sds_reg_read(sdsId, 0x1Fu, 0x15u, &reg_value);
    if (result != RT_ERR_OK) return result;

    if ( !((reg_value & 0x40u) | ((reg_value & 0x80u) == 0)) ) return RT_ERR_OK;

    result = rtl8372n_sds_reg_read(sdsId, 5u, 0, &reg_value);
    if (result != RT_ERR_OK) return result;

    if ( (reg_value & 1) != 0 )
    {
        result = rtl8372n_sds_reg_read(sdsId, 5u, 0, &reg_value);
        if (result != RT_ERR_OK) return result;

        if (!((reg_value & 0x2) | ((reg_value & 0x1000) == 0))) return RT_ERR_OK;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 3);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 1);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 3);
        if (result != RT_ERR_OK) return result;

        result =  rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 0);
        if (result != RT_ERR_OK) return result;
        return RT_ERR_OK;
    }
    else
    {
        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 3);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 1);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 3);
        if (result != RT_ERR_OK) return result;

        result = rtl8372n_sds_regbits_write(sdsId, 0x20u, 0, 0x30u, 0);
        if (result != RT_ERR_OK) return result;

        result =  rtl8372n_sds_reg_read(sdsId, 0x05u, 0, &reg_value);
        if (result != RT_ERR_OK) return result;
        return RT_ERR_OK;
    }
    
    return result;
}

ret_t serdes_patch(switch_chip_t chip_type, rtk_uint32 sdsId, rtk_uint32 mode)
{

    // 根据配置模式选择补丁数据
    rtk_uint32 patch_size = 0;

    static const struct PatchEntry patch_125M[] = {
        {0x0021, 0x0010, 0x6480},
        {0x0021, 0x0013, 0x0400},
        {0x0021, 0x0018, 0x6D02},
        {0x0021, 0x001B, 0x424E},
        {0x0021, 0x001D, 0x0002},
        {0x0036, 0x001C, 0x1390},
        {0x0026, 0x0004, 0x0080},
        {0x0026, 0x0009, 0x0601},
        {0x0026, 0x000B, 0x232C},
        {0x0026, 0x000C, 0x9217},
        {0x0026, 0x000F, 0x5B50},
        {0x0026, 0x0016, 0x0443},
        {0x0026, 0x001D, 0xABB0}
    };

    static const struct PatchEntry patch_3P125G[] = {
        {0x0021, 0x0010, 0x6480}, 
        {0x0021, 0x0013, 0x0004}, 
        {0x0021, 0x0018, 0x6D02}, 
        {0x0021, 0x001B, 0x424E}, 
        {0x0021, 0x001D, 0x0002}, 
        {0x0036, 0x001C, 0x1390}, 
        {0x0028, 0x0004, 0x0080}, 
        {0x0028, 0x0009, 0x0601},
        {0x0028, 0x000B, 0x232C},
        {0x0028, 0x000C, 0x9217}, 
        {0x0028, 0x000F, 0x5B50}, 
        {0x0028, 0x0016, 0x0443}, 
        {0x0028, 0x001D, 0xABB0}
    };

    static const struct PatchEntry patch_1P25G[] = {
        {0x0021, 0x0010, 0x6480},
        {0x0021, 0x0013, 0x0400},
        {0x0021, 0x0018, 0x6D02},
        {0x0021, 0x001B, 0x424E},
        {0x0021, 0x001D, 0x0002},
        {0x0036, 0x001C, 0x1390},
        {0x0036, 0x0014, 0x003F},
        {0x0036, 0x0010, 0x0300},
        {0x0024, 0x0004, 0x0080},
        {0x0024, 0x0007, 0x1201},
        {0x0024, 0x0009, 0x0601},
        {0x0024, 0x000B, 0x232C},
        {0x0024, 0x000C, 0x9217},
        {0x0024, 0x000F, 0x5B50},
        {0x0024, 0x0015, 0xE7C1},
        {0x0024, 0x0016, 0x0443},
        {0x0024, 0x001D, 0xABB0}
    };

    static const struct PatchEntry patch_10P3125G[] = { //rechecked
        {0x0021, 0x0010, 0x4480},
        {0x0021, 0x0013, 0x0400},
        {0x0021, 0x0018, 0x6D02},
        {0x0021, 0x001B, 0x424E},
        {0x0021, 0x001D, 0x0002},
        {0x0036, 0x001C, 0x1390},
        {0x002E, 0x0004, 0x0080},
        {0x002E, 0x0006, 0x0408},
        {0x002E, 0x0007, 0x020D},
        {0x002E, 0x0009, 0x0601},
        {0x002E, 0x000B, 0x222C},
        {0x002E, 0x000C, 0xa217},
        {0x002E, 0x000D, 0xfe40},
        {0x002E, 0x0015, 0xf5c1},
        {0x002E, 0x0016, 0x0443},
        {0x002E, 0x001D, 0xABB0}
    };

    const struct PatchEntry *patch_data_0 = NULL;

    if (mode == 0x5u) { // 125M模式
        patch_data_0 = patch_125M;
        patch_size = 78;
        printk("serdes_patch: patch_125M\n");
    } 
    else if (mode == 0x12u) { // 3.125G模式
        patch_data_0 = patch_3P125G;
        patch_size = 78;
        printk("serdes_patch: patch_3P125G\n");
    } 
    else if (mode == 0x2u || mode == 0x4u) { // 1.25G模式
        patch_data_0 = patch_1P25G;
        patch_size = 102;
        printk("serdes_patch: patch_1P25G\n");
    } 
    else if (mode == 0u || mode == 0xDu) { // 默认/10.3125G模式
        patch_data_0 = patch_10P3125G;
        patch_size = 96;
        printk("serdes_patch: patch_10P3125G\n");
    } 
    else {
        return RT_ERR_INPUT; // 不支持的配置模式
    }
    
    /*
    dig_patch_phy
    0006 0012 5078
    0006 0003 c45c
    0006 001e 000c
    0006 001f 2100

    dig_patch_mac
    0006 0012 5078 
    0007 0006 9401
    0007 0008 9401
    0007 000A 9401
    0007 000C 9401
    001F 000B 0003
    0006 0003 C45C
    0006 001F 2100
    */


    struct PatchEntry patch_data_mac[] = { //rechecked
        {0x0006, 0x0012, 0x5078},
        {0x0007, 0x0006, 0x9401},
        {0x0007, 0x0008, 0x9401},
        {0x0007, 0x000A, 0x9401},
        {0x0007, 0x000C, 0x9401},
        {0x001F, 0x000B, 0x0003},
        {0x0006, 0x0003, 0xC45C},
        {0x0006, 0x001F, 0x2100}
    };
    
    // struct PatchEntry patch_data_phy[] = {
    //     {0x0006, 0x0012, 0x5078},
    //     {0x0006, 0x0003, 0xc45c},
    //     {0x0006, 0x001e, 0x000c},
    //     {0x0006, 0x001f, 0x2100}
    // };

    // 应用扩展补丁数据
    for(int i = 0 ;i < (patch_size / 6);i++){
        rtl8372n_sds_reg_write(sdsId, patch_data_0[i].reg, patch_data_0[i].page, patch_data_0[i].val);
        // printk("serdes_patch: patch_data: reg:%016x\tpage:%016x\tval%016x\n",patch_data_0[i].reg, patch_data_0[i].page, patch_data_0[i].val);
    }
    
    // 应用固定补丁序列
    for (int i = 0; i < 8; i++) {
        rtl8372n_sds_reg_write(sdsId, patch_data_mac[i].reg, patch_data_mac[i].page, patch_data_mac[i].val);
        // printk("serdes_patch: patch_data: reg:%016x\tpage:%016x\tval%016x\n",patch_data_mac[i].reg, patch_data_mac[i].page, patch_data_mac[i].val);
    }
    
    return RT_ERR_OK;
}

ret_t fiber_fc_en(rtk_uint32 sdsId, rtk_uint32 mode,rtk_uint32 en_flag)
{
    rtk_uint32 config_value;          // 配置值变量
    
    // 模式类型5 (125M模式)
    if (mode == 0x05) {
        rtl8372n_sds_regbits_write(sdsId, 31, 5, 0x4, 1);
        rtl8372n_sds_regbits_write(sdsId, 31, 5, 0x8, 1);
        
        config_value = en_flag ? 3 : 0;
        return rtl8372n_sds_regbits_write(sdsId, 2, 4, 0xC00, config_value);
    }
    
    // 模式类型4或22
    if (mode == 0x04 || mode == 0x16) {
        rtl8372n_sds_regbits_write(sdsId, 31, 5, 0x4, 1);
        rtl8372n_sds_regbits_write(sdsId, 31, 5, 0x8, 0);
        
        config_value = en_flag ? 3 : 0;
        return rtl8372n_sds_regbits_write(sdsId, 2, 4, 0x180, config_value);
    }
    
    // 模式类型26
    if (mode == 0x1A) {
        // 根据使能标志设置SerDes寄存器 (页31, 寄存器11) 的位[3:2] (0xC)
        if (en_flag) {
            return rtl8372n_sds_regbits_write(sdsId, 31, 11, 0xC, 3);
        } else {
            return rtl8372n_sds_regbits_write(sdsId, 31, 11, 0xC, 0);
        }
    }
    
    // 其他模式类型不处理
    return sdsId;
}

ret_t sds_nway_set(rtk_uint32 sdsId, rtk_uint32 mode,rtk_uint32 en_flag)
{
    ret_t result;
    // 模式0和13的处理路径
    if (mode == 0x0 || mode == 0xD) {
        if (en_flag) {
            // 启用NWay
            return rtl8372n_sds_regbits_write(sdsId, 0x7u, 0x11u, 0xF, 0xFu);
        } else {
            // 禁用NWay
            return rtl8372n_sds_regbits_write(sdsId, 0x7u, 0x11u, 0xF, 0);
        }
    }
    
    // 模式2、4、18、22的处理路径
    if (mode == 0x02 || mode == 0x04 || mode == 0x12 || mode == 0x16) {
        rtk_uint32 value = en_flag ? 3 : 1;
        
        result = rtl8372n_sds_regbits_write(sdsId, 0, 2, 0x300u, value);
        if (result != RT_ERR_OK) return result;
        
        result = rtl8372n_sds_regbits_write(sdsId, 0, 4, 0x4u, 1);
        if (result != RT_ERR_OK) return result;
        return RT_ERR_OK;
    }
    
    // 其他模式不处理
    return sdsId;
}
