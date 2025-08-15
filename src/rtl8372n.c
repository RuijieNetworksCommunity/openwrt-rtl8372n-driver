#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/skbuff.h>
#include <linux/switch.h>
#include "./include/rtl8372n_mdio.h"

#include "./rtl8372n/include/rtk_error.h"
#include "./rtl8372n/include/rtl8372n_types.h"
#include "./rtl8372n/include/rtl8372n_switch.h"
#include "./rtl8372n/include/rtl8372n_asicdrv.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_phy.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_vlan.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_rma.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_eee.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_sds.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_cputag.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_igracl.h"
#include "./rtl8372n/include/rtl8372n_asicdrv_port.h"

#include <linux/printk.h>

// static struct rtk_gsw *_gsw;

#ifndef SWITCH_PORT_SPEED_10000
#define SWITCH_PORT_SPEED_10000 10000
#endif

#ifndef SWITCH_PORT_SPEED_2500
#define SWITCH_PORT_SPEED_2500 2500
#endif

int rtl837x_sw_get_port_stats_u(struct switch_dev *dev, int port,struct switch_port_stats *stats)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	rtk_portMib_read(gsw->port_map[port], 0, &(stats->tx_bytes));             // tx_bytes
	rtk_portMib_read(gsw->port_map[port], 2u, &(stats->rx_bytes));                // rx_bytes
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_port_stats_u);

/**
 * @brief 应用交换机配置
 * 
 * @param switch_dev 交换机设备结构指针
 * @return int 返回状态码 (0 = 成功)
 */
int rtl837x_sw_apply_config(struct switch_dev *swdev)
{
    struct rtk_gsw *gsw = container_of(swdev, struct rtk_gsw, sw_dev);

    int ret = 0;
    
	// TODO
    // // ====================== 1. 应用流控配置 ======================
    // for (int port = 0; port < swdev->ports; port++) {
    //     // 跳过CPU端口和特定端口
    //     if (port != 0 && port != swdev->cpu_port && port != 5) {
    //         rtk_uint8 ability[2];
    //         ability[0] = (drv->flow_control_map & (1 << port)) ? 
    //                     PORT_ABILITY_FC_BOTH : PORT_ABILITY_FC_NONE;
            
    //         // 设置端口自动协商能力
    //         ret = rtl8372n_phy_autoNegoAbility_set(drv->port_mapping[port], &ability);
    //         if (ret) {
    //             dev_err(drv->dev, "端口 %d 流控配置失败: %d", port, ret);
    //             return ret;
    //         }
    //     }
    // }
    
	printk("rtl837x Apply Config\n");

    // ====================== 2. 应用VLAN配置 ======================
    if (gsw->global_vlan_enable)
    {
		printk("rtl837x Apply Vlan config\n");
        // 重置VLAN配置
        rtl8372n_vlan_reset();
        
        // 设置所有VLAN条目
        for (int vlan_id = 0; vlan_id < swdev->vlans; vlan_id++) {
            if (gsw->vlan_table[vlan_id].valid == 1) {
                rtk_vlan_cfg_t vlan_cfg = {
                    .mbr = gsw->vlan_table[vlan_id].mbr,
                    .untag = gsw->vlan_table[vlan_id].untag,
					.leaky = 0,
					.fid = 0,
					.enable = 1,
                };
				printk("rtl837x VLAN mbr:%u\tntag:%u\n",vlan_cfg.mbr, vlan_cfg.untag);
                
                ret = rtl8372n_vlan_set(vlan_id, &vlan_cfg);
                if (ret) {
                    dev_err(gsw->dev, "VLAN %d 配置失败: %d", vlan_id, ret);
                    return ret;
                }
            }
        }

		printk("rtl837x Apply PVID\n");
        // ====================== 3. 应用PVID配置 ======================
        for (int port = 0; port < swdev->ports; port++) {
            // printk("rtl837x PVID port:%u\tpvid:%u\n",gsw->port_map[port], gsw->port_pvid[port]);
            ret = rtl8372n_vlan_portPvid_set(
                gsw->port_map[port], 
                gsw->port_pvid[port]
            );
            
            if (ret) {
                dev_err(gsw->dev, "端口 %d PVID 配置失败: %d", port, ret);
                return ret;
            }
        }
    }
    // ====================== 4. 应用端口隔离配置 ======================
    else
    {
	printk("rtl837x Apply port isolation\n");
        // 获取CPU端口的物理端口号
        rtk_uint32 cpu_phy_port = gsw->port_map[swdev->cpu_port];
        rtk_uint32 isolation_map = 0;
        
        // 构建隔离映射
        for (int port = 0; port < swdev->ports; port++) {
            // 跳过CPU端口
            if (port != swdev->cpu_port) {
                uint8_t phy_port = gsw->port_map[port];
                
                // 添加端口到隔离映射
                isolation_map |= (1 << phy_port);
                
                // 设置端口隔离
                ret = rtl8372n_port_isolation_set(phy_port, (1 << cpu_phy_port));
                if (ret) {
                    dev_err(gsw->dev, "端口 %d 隔离配置失败: %d", port, ret);
                    return ret;
                }
            }
        }
        
        // 设置CPU端口的隔离
        ret = rtl8372n_port_isolation_set(cpu_phy_port, isolation_map);
        if (ret) {
            dev_err(gsw->dev, "CPU端口隔离配置失败: %d", ret);
            return ret;
        }
    }
    
    return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_apply_config);


int rtl837x_sw_get_vlan_ports_u(struct switch_dev *dev, struct switch_val *val)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	val->len = 0;
    // if(!(gsw->vlan_table[val->port_vlan].valid)) return 0;
	rtk_vlan_cfg_t vlan_cfg;
	if (rtl8372n_vlan_get(val->port_vlan, &vlan_cfg)) return -22;
    if (!vlan_cfg.enable) return 0; //跳过下面的多余循环
	// printk("rtl837x vid:%u\tVLAN mbr:%u\tuntag:%u\tfid:%u\n",val->port_vlan ,vlan_cfg.mbr, vlan_cfg.untag, vlan_cfg.fid);

	struct switch_port *port = &val->value.ports[0];
	for(int i = 0;i < gsw->num_ports;i++){
		if (!(vlan_cfg.mbr & BIT(gsw->port_map[i]))) continue;

		port->id = i;
		port->flags = (vlan_cfg.untag & BIT(gsw->port_map[i])) ? 0 : BIT(SWITCH_PORT_FLAG_TAGGED);
		val->len++;
		port++;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_vlan_ports_u);

/**
 * @brief 设置 VLAN 的端口成员
 * 
 * @param switch_dev 交换机设备结构指针
 * @param vlan_val VLAN 值结构指针
 * @return 返回状态码 (0 = 成功)
 */
int rtl837x_sw_set_vlan_ports(struct switch_dev *dev, struct switch_val *vlan_val)
{
    // 获取 VLAN ID
    rtk_uint32 vlan_id = vlan_val->port_vlan;
    
    // 验证 VLAN ID 范围 (1-4094)
    if (vlan_id < 1 || vlan_id > 4094) return -22;
    
    // 获取设备数据
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
    
    // 验证端口数量
    uint32_t port_count = vlan_val->len;
    
    // 初始化端口位图
    rtk_uint32 vlan_mbr = 0;      // 所有成员端口位图
    rtk_uint32 vlan_untag = 0; // 未标记端口位图
    
    // 处理每个端口
    if (port_count > 0) {
        struct switch_port *port_list = vlan_val->value.ports;
        
        for (uint32_t i = 0; i < port_count; i++) {
            // 获取物理端口号
            rtk_uint32 physical_port = port_list[i].id;
            
            // 获取逻辑端口索引
            rtk_uint32 logical_port = gsw->port_map[physical_port];
            
            // 计算端口位掩码
            rtk_uint32 port_mask = BIT(logical_port);
            
            // 添加到所有端口位图
            vlan_mbr |= port_mask;
            
            // 如果是未标记端口，添加到未标记位图
            if (!(port_list[i].flags & BIT(SWITCH_PORT_FLAG_TAGGED))) {
                vlan_untag |= port_mask;
                // printk("port:%u\tflags:%d\n",physical_port,port_list[i].flags);
            }
        }
    }
    
    // 更新 VLAN 配置
    gsw->vlan_table[vlan_id].vid = vlan_id;
    gsw->vlan_table[vlan_id].mbr = vlan_mbr;
    gsw->vlan_table[vlan_id].untag = vlan_untag;
    gsw->vlan_table[vlan_id].valid = 1;
	printk("vlanid:%u\tportmap:%016x\tuntag:%016x\tvalid:%u\n",gsw->vlan_table[vlan_id].vid, gsw->vlan_table[vlan_id].mbr, gsw->vlan_table[vlan_id].untag, gsw->vlan_table[vlan_id].valid);
	// rtl837x_apply_config(dev);
    return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_set_vlan_ports);

int rtl837x_sw_get_port_pvid_u(struct switch_dev *dev, int port, int *val)
{
	int result; // x0
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	if (port > gsw->num_ports) return -22;

	result = rtl8372n_vlan_portPvid_get(gsw->port_map[port], val);
	if ( result )
	{
		dev_err(gsw->dev, "%s: rtk_vlan_portPvid_get failed, ret=%d\n", "rtl837x_get_port_pvid", result);
		return -22;
	}
	return result;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_port_pvid_u);

int rtl837x_sw_set_port_pvid_u(struct switch_dev *dev, int port, int val)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

    if (port > gsw->num_ports) return -22;

    gsw->port_pvid[port] = val;

	// rtl837x_apply_config(dev);
    return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_set_port_pvid_u);

/**
 * @brief 转换速度代码为具体速率值
 * 
 * @param speed_code 硬件速度代码
 * 
 * @return uint32_t 实际速率值 (Mbps)
 */
static uint32_t convert_speed_code(uint32_t speed_code)
{
    switch (speed_code) {
        case 0:		return SWITCH_PORT_SPEED_10;
        case 1:   	return SWITCH_PORT_SPEED_100;
        case 2:     return SWITCH_PORT_SPEED_1000;
        case 3:   	return SWITCH_PORT_SPEED_UNKNOWN;
        case 4:    	return SWITCH_PORT_SPEED_10000;
        case 5:    	return SWITCH_PORT_SPEED_2500;
        default:                return 0; // 未知状态
    }
}

int rtl837x_sw_get_port_link_status(struct switch_dev *dev, int port, struct switch_port_link *link)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

    // 检查端口有效性
    if (port >= gsw->num_ports) {
        dev_err(gsw->dev, "无效端口号: %u", port);
        return -22;
    }
    
    // 获取物理端口号
    rtk_uint32 phy_port = gsw->port_map[port];
    
    // 获取MAC状态信息
    rtl8372n_port_status_t port_status;
    int status = rtl8372n_portStatus_get(phy_port, &port_status);
    if (status != RT_ERR_OK) {
        dev_err(gsw->dev, 
                "获取端口 %u (物理端口 %u) MAC状态失败: %d", 
                port, phy_port, status);
        return status;
    }
    
    // 获取自动协商状态（特殊端口除外）
    bool auto_neg_enabled = 1;
    
    // 特殊端口：管理端口或端口5
    const bool is_special_port = (port == gsw->cpu_port) || (port == 5);
    
    if (!is_special_port) {
        rtk_uint32 auto_neg_value;
        status = rtl8372n_phy_common_c45_autoNegoEnable_get(phy_port, &auto_neg_value);
        if (status != RT_ERR_OK) {
            dev_err(gsw->dev, 
                    "获取端口 %u (物理端口 %u) 自动协商状态失败: %d", 
                    port, phy_port, status);
            return status;
        }
        auto_neg_enabled = (auto_neg_value != 0);
    }
    
    // 解析并填充链路状态
    struct switch_port_link result = {
        .link = (port_status.link_up != 0),
        .duplex = (port_status.duplex != 0),
        .aneg = auto_neg_enabled,
        .tx_flow = (port_status.tx_flow != 0),
        .rx_flow = (port_status.rx_flow != 0),
        .speed = convert_speed_code(port_status.speed)
    };

    *link = result;
    return RT_ERR_OK;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_port_link_status);

int rtl837x_sw_set_port_link_u(struct switch_dev *dev, int port, struct switch_port_link *link)
{
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_set_port_link_u);

int rtl837x_sw_reset_switch(struct switch_dev *dev)
{
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_reset_switch);

int rtl837x_sw_set_vlan_enable(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
	gsw->global_vlan_enable = val->value.i;
    // printk("set %u\n",gsw->global_vlan_enable);
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_set_vlan_enable);


int rtl837x_sw_get_vlan_enable(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
    val->value.i = gsw->global_vlan_enable;
    // printk("get %u\n",gsw->global_vlan_enable);
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_vlan_enable);

int rtl837x_sw_reset_mibs(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{
    // struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);
    rtk_globalMib_rst();
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_reset_mibs);

int rtl837x_sw_reset_port_mibs(struct switch_dev *dev,const struct switch_attr *attr,struct switch_val *val)
{
	rtk_uint32 port;
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	port = val->port_vlan;
	if (port >= 9) return -EINVAL;

	return rtk_portMib_rst(gsw->port_map[port]);
}
EXPORT_SYMBOL_GPL(rtl837x_sw_reset_port_mibs);

int rtl837x_sw_get_port_mib(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val)
{	
    struct rtk_gsw *gsw = container_of(dev, struct rtk_gsw, sw_dev);

	int i, len = 0;
	unsigned long long counter = 0;
	char *buf = gsw->buf;

	if (val->port_vlan >= gsw->num_ports)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(gsw->buf) - len, "Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < gsw->num_mib_counters; ++i) {
		len += snprintf(buf + len, sizeof(gsw->buf) - len, "%-36s: ", gsw->mib_counters[i].name);

		if (!rtk_portMib_read(val->port_vlan, gsw->mib_counters[i].base, &counter))
			len += snprintf(buf + len, sizeof(gsw->buf) - len, "%llu\n", counter);
		else
			len += snprintf(buf + len, sizeof(gsw->buf) - len, "%s\n", "error");
	}

	val->value.s = buf;
	val->len = len;
	return 0;
}
EXPORT_SYMBOL_GPL(rtl837x_sw_get_port_mib);

static struct switch_attr rtl832n_globals[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "Enable VLAN mode",
		.set = rtl837x_sw_set_vlan_enable,
		.get = rtl837x_sw_get_vlan_enable,
		.max = 1,
	}, {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mibs",
		.description = "Reset all MIB counters",
		.set = rtl837x_sw_reset_mibs,
	}//, {
	// 	.type = SWITCH_TYPE_INT,
	// 	.name = "enable_flowcontrol",
	// 	.description = "set hw flow control of port mask (1f: all ports)",
	// 	.set = sub_1648,
	// 	.get = sub_1690,
    // }, {
	// 	.type = SWITCH_TYPE_INT,
	// 	.name = "enable_igmp_snooping",
	// 	.description = "igmp snooping (1:enabled)",
	// 	.set = sub_16A0, Null
	// 	.get = sub_1EE8, Null
    // }
};

static struct switch_attr rtl837x_port[] = {
	{
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset_mib",
		.description = "Reset single port MIB counters",
		.set = rtl837x_sw_reset_port_mibs,
	}, {
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.set = NULL,
		.get = rtl837x_sw_get_port_mib,
	},
};

static const struct switch_dev_ops rtl8372n_sw_ops = {
    .attr_global = { .attr = rtl832n_globals, .n_attr = ARRAY_SIZE(rtl832n_globals)},
    .attr_port = { .attr = rtl837x_port, .n_attr = ARRAY_SIZE(rtl837x_port) },
    .attr_vlan = { .attr = NULL, .n_attr = 0 },

	.get_vlan_ports = rtl837x_sw_get_vlan_ports_u,
	.set_vlan_ports = rtl837x_sw_set_vlan_ports,

	.get_port_pvid = rtl837x_sw_get_port_pvid_u,
	.set_port_pvid = rtl837x_sw_set_port_pvid_u,
	
	.apply_config = rtl837x_sw_apply_config,
	.reset_switch = rtl837x_sw_reset_switch,

	.get_port_link = rtl837x_sw_get_port_link_status,
	.set_port_link = rtl837x_sw_set_port_link_u,

	.get_port_stats = rtl837x_sw_get_port_stats_u,
};

int rtl8372n_switch_init(struct rtk_gsw *i_gsw)
{   

	struct switch_dev *dev = &i_gsw->sw_dev;
	int err;

	dev->name = "RTL8372n";
	dev->cpu_port = i_gsw->cpu_port;
	dev->ports = i_gsw->num_ports;
	dev->vlans = 4096;
	dev->ops = &rtl8372n_sw_ops;
	dev->alias = dev_name(i_gsw->dev);

	err = register_switch(dev, NULL);
	if (err)
		dev_err(i_gsw->dev, "switch registration failed\n");

	i_gsw->global_vlan_enable = true; 

    // printk("init %u\n",gsw->global_vlan_enable);

	return err;
}