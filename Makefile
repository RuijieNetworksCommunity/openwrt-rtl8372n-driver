include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8372n_gsw
PKG_RELEASE:=1
PKG_MAINTAINER:=air jinkela (air_jinkela@163.com)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)
	SUBMENU:=Other modules
	TITLE:=$(PKG_NAME)
	FILES:=$(PKG_BUILD_DIR)/rtl8372n_gsw.ko
	AUTOLOAD:=$(call AutoLoad, 99, rtl8372n_gsw)
	DEPENDS:=+kmod-swconfig
endef

EXTRA_KCONFIG:= \
	CONFIG_RTL8372N_GSW=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \
	-DVERSION=$(PKG_RELEASE) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(PKG_BUILD_DIR)/rtl8372n/include

MAKE_OPTS:=$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" $(MAKE_OPTS) modules
endef

$(eval $(call KernelPackage,rtl8372n_gsw))
