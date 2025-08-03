
#include "include/rtk_error.h"
#include "include/rtl8372n_types.h"
#include "include/rtl8372n_smi.h"

/* MDC/MDIO, redefine/implement the following Macro */ /*carlos*/

#define u32      unsigned int
extern unsigned int rtl837x_mdio_lock(void);
extern unsigned int rtl837x_mdio_unlock(void);
extern unsigned int rtl837x_mdio_read(unsigned int phy_register, unsigned int *read_data);
extern unsigned int rtl837x_mdio_write(unsigned int phy_register, unsigned int write_data);

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{

    if(mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    if(rData == NULL)
        return RT_ERR_NULL_POINTER;

    u32 busy_flag;
    u32 value_low;
    u32 value_high;

    // 获取MDIO锁
    rtl837x_mdio_lock();

    rtl837x_mdio_read(0x15, &busy_flag);
    if (busy_flag & 0x4) {
        // 设备忙，返回错误
        rtl837x_mdio_unlock();
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    // 设置要读取的寄存器地址
    rtl837x_mdio_write(MDC_MDIO_ADDRESS_REG, mAddrs);

    // 发送读命令 
    rtl837x_mdio_write(0x15, MDC_MDIO_READ_OP);
    
    // 再次检查忙标志
    rtl837x_mdio_read(0x15, &busy_flag);
    if (busy_flag & 0x4) {
        // 操作超时
        rtl837x_mdio_unlock();
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    // 读取数据 (低16位)
    rtl837x_mdio_read(MDC_MDIO_DATAL_REG, &value_low);
    
    // 读取数据 (高16位)
    rtl837x_mdio_read(MDC_MDIO_DATAH_REG, &value_high);
    
    // 组合32位值
    *rData = value_low | (value_high << 16);
    
    // 释放MDIO锁
    rtl837x_mdio_unlock();
    return RT_ERR_OK;
}



rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{

    if(mAddrs > 0xFFFF)
        return RT_ERR_INPUT;

    u32 busy_flag;

    // 获取MDIO锁
    rtl837x_mdio_lock();

    // 检查忙标志
    rtl837x_mdio_read(0x15, &busy_flag);
    if (busy_flag & 0x4) {
        // 设备忙，返回错误
        rtl837x_mdio_unlock();
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }

    //设置要写入的寄存器地址
    rtl837x_mdio_write(MDC_MDIO_ADDRESS_REG, mAddrs);
    
    // 写入数据低16位 (寄存器0x17)
    rtl837x_mdio_write(MDC_MDIO_DATAL_REG, rData & 0xFFFF);
    
    //写入数据高16位 (寄存器0x18)
    rtl837x_mdio_write(MDC_MDIO_DATAH_REG, (rData >> 16) & 0xFFFF);
    
    //发送写命令 (0x19)
    rtl837x_mdio_write(0x15, MDC_MDIO_WRITE_OP);
    
    //再次检查忙标志
    rtl837x_mdio_read(0x15, &busy_flag);
    if (busy_flag & 0x4) { // 位2 (0x4) 表示忙状态
        rtl837x_mdio_unlock();
        return RT_ERR_BUSYWAIT_TIMEOUT;
    }
    
    // 释放MDIO锁
    rtl837x_mdio_unlock();

    return RT_ERR_OK;

}

