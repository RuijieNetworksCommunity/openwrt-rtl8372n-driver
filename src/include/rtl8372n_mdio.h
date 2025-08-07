#ifndef _RTL8372N_MDIO_H_
#define _RTL8372N_MDIO_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/of_net.h>
#include <linux/gpio/consumer.h>
#include <linux/switch.h>
#include "../rtl8372n/include/rtl8372n_types.h"
#include "../rtl8372n/include/rtl8372n_switch.h"

#ifdef __cplusplus
extern "C" {
#endif

struct rtl837x_mib_counter {
	uint16_t	base;
	const char	*name;
};

typedef struct rtk_gsw {
	struct device *dev;
	struct mii_bus *bus;
	const char *serdes_mode;
	unsigned int smi_addr;
	unsigned int cpu_port;
	int reset_pin;
	int irq_pin;
	int irq;
	const char *chip_name;
	unsigned int num_ports;
	const uint8_t *port_map;
	switch_chip_t switch_chip;
	struct net_device *ethernet_master;

	struct rtl837x_mib_counter *mib_counters;
	unsigned int num_mib_counters;

	char buf[4096];

	uint16_t flow_control_map;      // 流控配置位图
    uint8_t global_vlan_enable;              // VLAN 是否启用	//你为什么会越界访问sw_dev?????   WHY? Tell Me! WHY!!!! Looking! MY EYES! Tell ME!
    
    struct {
        uint8_t valid;                 // 条目是否有效
        uint16_t vid;                // VLAN ID
        uint16_t mbr;           // 成员端口位图
        uint16_t untag;       // 未标记端口位图
    } vlan_table[4096];        // VLAN 配置表
    
    uint16_t port_pvid[6];  // 端口PVID配置

	struct switch_dev sw_dev; //你为什么放在这就好了?????
}rtk_gsw_t;

extern int rtl8372n_switch_init(rtk_gsw_t *i_gsw);
extern int rtl837x_debug_proc_init(void);
extern int rtl837x_debug_proc_deinit(void);

#ifdef __cplusplus
}
#endif

#endif