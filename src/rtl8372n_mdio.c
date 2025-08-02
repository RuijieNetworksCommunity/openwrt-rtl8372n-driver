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

const uint8_t rtl8373_port_map[16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, // 物理端口0-8
    0, 0, 0, 0, 0, 0, 0        // 填充
};

const uint8_t rtl8372_port_map[16] = {
    3, 4, 5, 6, 7, 8, // 物理端口3-8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // 填充
};

static struct rtl837x_mib_counter rtl837x_mib_counters[] ={
	{0,"ifInOctets"},
	{2,"ifOutOctets"},
	{4,"ifInUcastPkts"},
	{6,"ifInMulticastPkts"},
	{8,"ifInBroadcastPkts"},
	{0xA,"ifOutUcastPkts"},
	{0xC,"ifOutMulticastPkts"},
	{0xE,"ifOutBroadcastPkts"},
	{0x10,"ifOutDiscards"},
	{0x19,"InPauseFrames"},
	{0x1A,"OutPauseFrames"},
	{0x1C,"TxBroadcastPkts"},
	{0x1D,"TxMulticastPkts"},
	{0x20,"TxUndersizePkts"},
	{0x21,"RxUndersizePkts"},
	{0x22,"TxOversizePkts"},
	{0x23,"RxOversizePkts"},
	{0x24,"TxFragments"},
	{0x25,"RxFragments"},
	{0x26,"TxJabbers"},
	{0x27,"RxJabbers"},
	{0x28,"TxCollisions"},
	{0x29,"Tx64Octets"},
	{0x2A,"Rx64Octets"},
	{0x2B,"Tx65to127Bytes"},
	{0x2C,"Rx65to127Bytes"},
	{0x2D,"Tx128to255Bytes"},
	{0x2E,"Rx128to255Bytes"},
	{0x2F,"Tx256to511Bytes"},
	{0x30,"Rx256to511Bytes"},
	{0x31,"Tx512to1023Bytes"},
	{0x32,"Rx512to1023Bytes"},
	{0x33,"Tx1024to1518Bytes"},
	{0x34,"Rx1024to1518Bytes"},
	{0x36,"RxUndersizedropPkts"},
	{0x37,"Tx1519toMaxBytes"},
	{0x38,"Rx1519toMaxBytes"},
	{0x39,"TxOverMaxBytes"},
	{0x3A,"RxOverMaxBytes"}
};

static struct rtk_gsw *_gsw;

unsigned int rtl837x_mdio_lock(void)
{
	struct mii_bus *bus = _gsw->bus;
	mutex_lock(&bus->mdio_lock);
	return 0;
}

unsigned int rtl837x_mdio_unlock(void)
{
	struct mii_bus *bus = _gsw->bus;
	mutex_unlock(&bus->mdio_lock);
	return 0;
}

unsigned int rtl837x_mdio_read(unsigned int phy_register,unsigned int *read_data)
{
	struct mii_bus *bus = _gsw->bus;

	*read_data = bus->read(bus, _gsw->smi_addr, phy_register);
	// printk("mdio_read:addr:%d\tphy_register:%d\tdata:%d\n", _gsw->smi_addr,phy_register ,*read_data);
	return 0;
}

unsigned int rtl837x_mdio_write(unsigned int phy_register,unsigned int write_data)
{
	struct mii_bus *bus =  _gsw->bus;

	bus->write(bus, _gsw->smi_addr, phy_register, write_data);
	// printk("mdio_write:addr:%d\tphy_register:%d\tdata:%d\n", _gsw->smi_addr, phy_register ,write_data);
	return 0;
}

static int rtl837x_switch_probe(void)
{
	struct rtk_gsw *gsw = _gsw;
	int i = 0;
	while (i <= 3)
	{
		i++;
		switch_chip_t sw_chip;
		if (rtk_switch_probe(&sw_chip) != RT_ERR_OK) {
			dev_warn(gsw->dev , "Error: Detect switch type failed\n");
			continue; // 重试
		}

		//RTL8372芯片识别 (0x8372)
		if (sw_chip == CHIP_RTL8372N || sw_chip == CHIP_RTL8372) {
			gsw->chip_name = rtk_chipid_to_chip_name(sw_chip);
			gsw->num_ports = 6;
			gsw->port_map = rtl8372_port_map;
			goto END_DETECT_CHIP;
		}

		//rtl8372n芯片识别 (0x8373)
		if (sw_chip == CHIP_RTL8373N || sw_chip == CHIP_RTL8373) {
			gsw->chip_name = rtk_chipid_to_chip_name(sw_chip);
			gsw->num_ports = 9;
			gsw->port_map = rtl8373_port_map;
			goto END_DETECT_CHIP;
		}
	}

	//未知芯片ID
    rtk_uint32 regValue;
    rtl8372n_getAsicReg(4, &regValue);
	dev_err(gsw->dev, "Error: Can not support this device, devid 0x%x\n", regValue);
	return RT_ERR_CHIP_NOT_SUPPORTED;
END_DETECT_CHIP:
	dev_info(gsw->dev, "Found Realtek RTL chip %s\n", gsw->chip_name);
	return RT_ERR_OK;
}

static int rtl837x_hw_reset(void)
{
	struct rtk_gsw *gsw = _gsw;

	if (gsw->reset_pin < 0)
		return 0;
	printk("START HW RESET");
	gpio_direction_output(gsw->reset_pin, 0);

	gpio_set_value(gsw->reset_pin, 1);
	mdelay(50);

	gpio_set_value(gsw->reset_pin, 0);
	mdelay(50);

	gpio_set_value(gsw->reset_pin, 1);
	mdelay(50);

	printk("FINISH HW RESET");
	return 0;
}

ret_t rtl8372n_setup_port_u(void)
{
	struct rtk_gsw *gsw = _gsw;
	rtk_uint32 cpu_port_nonzero; // w20
	cpu_port_nonzero = gsw->cpu_port != 0;

	// rtl8372n_sdsMode_set(0, 0x0u); 
	rtl8372n_sdsMode_set(0, 0x1Fu); 

	// 步骤3: 延时约8.59毫秒 (等待配置生效)
	mdelay(10);
	rtl8372n_sdsMode_set(0, 0x0Du);// 先写死，之后再改

	// 步骤5: 设置CPU端口为外部端口
	if ( rtl8372n_cpuTag_externalCpuPort_set(gsw->port_map[gsw->cpu_port]))
		return RT_ERR_FAILED;
	else
		return RT_ERR_OK;
}

static int rtl8372n_hw_init(void)
{
	struct rtk_gsw *gsw = _gsw;

	rtk_int32 res;
	rtl837x_hw_reset();
	rtl837x_switch_probe();
	res = rtk_switch_init();
	if(res){
		dev_err(gsw->dev, "rtk_switch_init Fail, erron:%d\n",res);
		return -1;
	}

	res = rtk_vlan_init();
    if (res)
    {
		dev_err(gsw->dev, "rtk_vlan_init failed, errno:%d\n",res);
		return -1;
    }

	rtk_vlan_cfg_t vlan_config = {
		.mbr = 0,
		.untag = 0,
		.fid = 0,
		.leaky = 0,
		.enable = 0,
	};

	res = rtl8372n_vlan_set(1u,&vlan_config);
    if (res)
    {
		dev_err(gsw->dev, "rtk_vlan_set failed, errno%d\n",res);
		return -1;
    }

	res = rtl8372n_vlan_portPvid_set(gsw->port_map[gsw->cpu_port], 0);
    if (res)
    {
		dev_err(gsw->dev, "rtk_vlan_portPvid_set set pvid for cpu port failed, errno %d\n",res);
		return -1;
    }

	rtl8372n_rma_t pRmacfg;
	res = rtl8372n_asicRma_get(2u, &pRmacfg);
	if ( res )
	{
		dev_err(gsw->dev, "rtk_rma_get get rma failed, errno %d\n", res);
	return -1;
	}

	pRmacfg.operation = 0;                   // 清零配置
	res = rtl8372n_asicRma_set(2u, &pRmacfg);
	if ( res )
	{
		dev_err(gsw->dev, "rtk_rma_get set rma failed, errno %d\n", res);
		return -1;
	}

	rtl8372n_eee_init();                   // 空函数？
	if(gsw->num_ports > 0){
		for(int port = 0;port < gsw->num_ports;port++){
			rtk_uint32 phy_port = gsw->port_map[port];
			res = rtl8372n_eee_portTxRxEn_set(phy_port, 0u, 0u);
			if (res)
			{
				dev_err(gsw->dev, "rtl8372n_eee_portTxRxEn_set failed, errno %d\n",res);
				return -1;
			}
		}
	}

	return 0;
}

ret_t init_rtl837x_gsw(void)
{
	ret_t res;
	struct rtk_gsw *gsw = _gsw;

	res = rtl8372n_hw_init();
	if (res)
	{
		dev_err(gsw->dev, "rtl8372n_hw_init failed, errno %d\n",res);
		return -19;
	}

	res = rtl8372n_setup_port_u();
	if (res)
	{
		dev_err(gsw->dev, "rtl837x_setup_port failed, errno %d\n",res);
		return -19;
	}

	// TODO
	// res = rtl8372n_igrAcl_init();
	// if (res != RT_ERR_OK){
	// 	dev_err(gsw->dev, "ACL init failed, ret=%d\n", res);
	// 	return res;
	// }

	// TODO
	// res = rtl837x_acl_add_u(a1);
	// if (res != RT_ERR_OK){
	// 	dev_err(gsw->dev, "rtl837x_acl_add failed, ret=%d\n", res);
	// 	return res;
	// }

	res = rtl8372n_switch_init(_gsw);
	if (res != RT_ERR_OK){
		dev_err(gsw->dev, "rtl8372n_switch_init failed, ret=%d\n", res);
		return res;
	}

	rtl837x_debug_proc_init();

	return RT_ERR_OK;
}

// below are platform driver
static const struct of_device_id rtk_gsw_match[] = {
	{ .compatible = "realtek,rtl837x" },
	{},
};

MODULE_DEVICE_TABLE(of, rtk_gsw_match);

static int rtl837x_gsw_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *mdio;
	struct mii_bus *mdio_bus;
	struct rtk_gsw *gsw;
	struct device_node *ethernet;
	int ret;
	dev_info(&pdev->dev,"start rtl837x_gsw_probe");
	mdio = of_parse_phandle(np, "mediatek,mdio", 0);

	if (!mdio)
		return -EINVAL;

	mdio_bus = of_mdio_find_bus(mdio);

	if (!mdio_bus)
		return -EPROBE_DEFER;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct rtk_gsw), GFP_KERNEL);

	if (!gsw)
		return -ENOMEM;

	gsw->dev = &pdev->dev;

	gsw->bus = mdio_bus;

	ethernet = of_parse_phandle(np, "ethernet", 0);
	if (!ethernet) 
		return -EINVAL;
	gsw->ethernet_master = of_find_net_device_by_node(ethernet);
	if (!gsw->ethernet_master)
		return -EPROBE_DEFER;

	of_property_read_u32(np, "smi-addr", &(gsw->smi_addr));
	of_property_read_u32(np, "cpu-port", &(gsw->cpu_port));
	of_property_read_string(np, "serdes-mode", &(gsw->serdes_mode)); //没有用到 后续修复

	gsw->reset_pin = of_get_named_gpio(np, "mediatek,reset-pin", 0);
	if (gsw->reset_pin >= 0) {
		ret = devm_gpio_request(gsw->dev, gsw->reset_pin, "mediatek,reset-pin");
		if (ret)
			printk("fail to devm_gpio_request mediatek,reset-pin\n");
	}

	gsw->irq_pin = of_get_named_gpio(np, "irq-gpios", 0);
	if (gsw->irq_pin >= 0) {
		ret = devm_gpio_request(gsw->dev, gsw->irq_pin, "irq-gpios");
		if (ret)
			printk("fail to devm_gpio_request irq-gpios\n");
	}

	gsw->irq = gpiod_to_irq(gpio_to_desc(gsw->irq_pin));
	if (gsw->irq < 0) {
		dev_err(gsw->dev, "Couldn't gpiod_to_irq\n");
		return gsw->irq;
	}

	gsw->mib_counters = rtl837x_mib_counters;
	gsw->num_mib_counters = ARRAY_SIZE(rtl837x_mib_counters);
	_gsw = gsw;

	printk("rtl837x dev info:smi-addr:%d\tcpu_port:%d\tserdes-mode:%s\n", gsw->smi_addr, gsw->cpu_port, gsw->serdes_mode);
	init_rtl837x_gsw();

	platform_set_drvdata(pdev, gsw);

	return 0;
}

static int rtl837x_gsw_remove(struct platform_device *pdev)
{
	struct rtk_gsw *gsw = platform_get_drvdata(pdev);

	unregister_switch(&gsw->sw_dev);
	rtl837x_debug_proc_deinit();
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver gsw_driver = {
	.probe = rtl837x_gsw_probe,
	.remove = rtl837x_gsw_remove,
	.driver = {
		.name = "rtk-gsw",
		.of_match_table = rtk_gsw_match,
	},
};

module_platform_driver(gsw_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("air jinkela <air_jinkela@163.com>");
MODULE_DESCRIPTION("rtl8372n switch driver for MT7988");
