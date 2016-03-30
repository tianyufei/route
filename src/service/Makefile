#make files.
export SRV_PATH=$(ROOT_PATH)/src/service/inc

SUBDIRS=logmgr apmgr usrmgr

ifeq ($(Y_SOFTSRV_PORTAL),y)
SUBDIRS+=portal
endif

ifeq ($(Y_SOFTSRV_CFM),y)
SUBDIRS+=cfm
endif

include $(BUILD_PATH_MK)

