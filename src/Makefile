obj-$(CONFIG_RTL8372N_GSW) += rtl8372n_gsw.o
rtl8372n_gsw-objs := rtl8372n_mdio.o
rtl8372n_gsw-objs += rtl8372n.o
rtl8372n_gsw-objs += rtl8372n_dbg.o

rtl8372n_gsw-objs += rtl8372n/rtl8372n_smi.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_phy.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_vlan.o
# rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_svlan.o //TODO
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_sds.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_rma.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_eee.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_cputag.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_igracl.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_asicdrv_port.o
rtl8372n_gsw-objs += rtl8372n/rtl8372n_switch.o

ccflags-y += -Werror -D_LITTLE_ENDIAN -DMDC_MDIO_OPERATION

ccflags-y += -Idrivers/net/phy/rtl837x/include
