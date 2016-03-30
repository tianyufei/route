# 
# Copyright (c) 2014 yaya, Inc.
# All Rights Reserved. 
# yaya Confidential and Proprietary.
#
# Author: Dajun Zheng (zhengdajun@yaya.cn)
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yaya
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/platform_config.mk
include $(TOPDIR)/platform_env.mk

# qman main Library
define Package/yaya
	CATEGORY:=YAYA
	TITLE:=yaya
	MAINTAINER:=zhengdajun@yaya.cn
	URL:=http://www.yaya.cn
	DEPENDS:= +libdbus +libexpat +libpthread +dbus +dbus-utils \
             +iptables +iptables-mod-conntrack +iptables-mod-nat +iptables-mod-ipopt \
             +kmod-ppp +kmod-pppoe +ppp +ppp-mod-pppoe \
             +udhcp +dproxy +ebtables \
             +kmod-ebtables-ipv4 \
             +kmod-ipt-nathelper-extra \
             +hotplug2 +udevtrigger +mtd \
             +athrs-wireless-tool +kmod-athrs_gmac \
             +libgsoapssl +libopenssl +libdbus-1 +libjson-c +libcrypto +libssl +libuci
ifeq ($(WITH_UCI),y)
	DEPENDS:=$(DEPENDS) +libuci
endif

endef

define Package/yaya/Description
	YAYA home gateway management system
endef

ifneq ($(strip $(CONFIG_YAYA_SRC_TREE)),"")
define Build/Prepare
	if [ -d $(PKG_BUILD_DIR) ]; then \
		rm -fR $(PKG_BUILD_DIR); \
	fi	
	ln -s $(CONFIG_YAYA_SRC_TREE) $(PKG_BUILD_DIR)
endef
endif

## set CONFIGURE_ARGS or CONFIGURE_VARS instead
YAYA_ARGS = CFLAGS="-Wwrite-strings -Wall -Wextra -O2 -fPIC" \
	CC="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-gcc" \
	LD="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-ld" \
	NM="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-nm" \
	STRIP="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-strip" \
	AR="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-ar" \
	CPP="$(CONFIG_ARCH)-openwrt-linux-$(CONFIG_TARGET_SUFFIX)-g++"

define Build/Configure
endef

define Build/Compile
	echo "compile yaya"
#	$(MAKE) -C "$(PKG_BUILD_DIR)" $(YAYA_ARGS) clean
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(YAYA_ARGS) 
	echo "compile yaya finished"
endef

define Build/Clean
	echo "clean yaya"
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(YAYA_ARGS) clean
endef

# install qman library
define Package/yaya/install
	echo "install yaya"
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/yaya.init $(1)/etc/init.d/yaya
	$(CP) -rf $(CONFIG_YAYA_OUT_TREE)/target/* $(1)
	$(CP) $(STAGING_DIR)/usr/lib/libdbus-1.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/usr/lib/libjson-c.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/usr/lib/libssl.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/usr/lib/libcrypto.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/usr/lib/libz.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/root-$(CONFIG_TARGET_BOARD)/lib/libpthread.so* $(1)/bin/
ifeq ($(SOFT_YOS_DB),y)
	$(CP) $(STAGING_DIR)/usr/lib/mysql/libmysqlclient.so.* $(1)/bin/
endif
#	$(CP) $(STAGING_DIR)/root-$(CONFIG_TARGET_BOARD)/lib/librt.so* $(1)/bin/
	$(CP) $(STAGING_DIR)/root-$(CONFIG_TARGET_BOARD)/usr/lib/libstdc++.so* $(1)/bin/
	rm -rf $(1)/obj
	ln -sf /tmp/resolv.conf $(1)/etc/resolv.conf
	echo "install yaya finished"
endef

$(eval $(call BuildPackage,yaya))

