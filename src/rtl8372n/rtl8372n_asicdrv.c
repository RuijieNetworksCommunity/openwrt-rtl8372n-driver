#include <rtl8372n_asicdrv.h>
#include <rtl8372n_smi.h>

ret_t rtl8372n_setAsicRegBit(rtk_uint32 reg, rtk_uint32 bit, rtk_uint32 value)
{
    ret_t result;
    rtk_uint32 regData;

    // 验证位位置有效性
    if (bit >= RTL8372N_REGBITLENGTH) return RT_ERR_INPUT;

    // 读取整个寄存器
    result = smi_read(reg, &regData);
    if (result != RT_ERR_OK) return RT_ERR_SMI;

    // 修改值
    if(value)
        regData = regData | (1 << bit);
    else
        regData = regData & (~(1 << bit));

    // 写回去
    result = smi_write(reg, regData);
    if (result != RT_ERR_OK) return RT_ERR_SMI;
    
    return RT_ERR_OK;
}
ret_t rtl8372n_getAsicRegBit(rtk_uint32 reg, rtk_uint32 bit, rtk_uint32 *pValue)
{
    ret_t result;
    rtk_uint32 regData;

    // 读取整个寄存器值
    result = smi_read(reg, &regData);
    if (result != RT_ERR_OK) return RT_ERR_SMI;

    // 提取特定位的值
    *pValue = ((1 << bit) & regData) >> bit;
    return RT_ERR_OK;
}

ret_t rtl8372n_setAsicRegBits(rtk_uint32 reg, rtk_uint32 bits, rtk_uint32 value)
{
    int bitsShift = 0;
    ret_t result;
    rtk_uint32 regData;

    // 验证掩码有效性
    if(bits >= (1ULL << RTL8372N_REGBITLENGTH) ) return RT_ERR_INPUT;

    // 计算位偏移量
    while (1) {
        // 检查当前位是否在掩码中
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超过32位
        if (++bitsShift == RTL8372N_REGBITLENGTH) {
        return RT_ERR_INPUT;
        }
    }

    // 读取当前寄存器值
    result = smi_read(reg, &regData);

    if (result != RT_ERR_OK) return RT_ERR_SMI;

    // 修改位域值
    regData = regData & (~bits);
    regData = regData | ((value << bitsShift) & bits);
    // 写入寄存器
    result = smi_write(reg, regData); 
    if (result != RT_ERR_OK) return RT_ERR_SMI;

    return RT_ERR_OK;
}

ret_t rtl8372n_getAsicRegBits(rtk_uint32 reg, rtk_uint32 bits, rtk_uint32 *pValue)
{
    int bitsShift = 0;
    ret_t result; 
    rtk_uint32 regData;

    // 验证位掩码有效性
    if(bits >= (1ULL << RTL8372N_REGBITLENGTH) ) return RT_ERR_INPUT;

    // 计算位偏移量 (找到最低有效位)
    while (1) {
        // 检查当前位是否在掩码中置位
        if (bits & (1 << bitsShift)) {
        break; // 找到最低有效位
        }
        
        // 检查是否超出32位范围
        if (++bitsShift == RTL8372N_REGBITLENGTH) {
        return RT_ERR_INPUT; // RT_ERR_INPUT (无效位掩码)
        }
    }

    // 读取寄存器值
    result = smi_read(reg, &regData);
    if (result != RT_ERR_OK) return RT_ERR_SMI;

    // 处理读取结果
    *pValue = (bits & regData) >> bitsShift;

    return RT_ERR_OK;
}

ret_t rtl8372n_setAsicReg(rtk_uint32 reg, rtk_uint32 value)
{
    ret_t result; 
    result = smi_write(reg, value);

    if (result != RT_ERR_OK) return RT_ERR_SMI;

    return RT_ERR_OK;
}

ret_t rtl8372n_getAsicReg(rtk_uint32 reg, rtk_uint32 *pValue)
{
    ret_t result; 
    rtk_uint32 regData;

    result = smi_read(reg, &regData);
    if (result != RT_ERR_OK) return RT_ERR_SMI;
    
    *pValue = regData;

    return RT_ERR_OK;
}
