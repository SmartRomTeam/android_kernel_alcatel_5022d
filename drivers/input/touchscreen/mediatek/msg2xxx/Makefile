include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/msg2xxx/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/mt6580/$(ARCH_MTK_PROJECT)/touchpanel/msg2xxx/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/msg2xxx/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/

obj-y	+=  mstar_drv_common.o
obj-y	+=  mstar_drv_ic_fw_porting_layer.o
obj-y	+=  mstar_drv_main.o
obj-y	+=  mstar_drv_mtk.o
obj-y	+=  mstar_drv_mutual_fw_control.o
obj-y	+=  mstar_drv_mutual_mp_test.o
obj-y	+=  mstar_drv_platform_interface.o
obj-y	+=  mstar_drv_platform_porting_layer.o
obj-y	+=  mstar_drv_utility_adaption.o
obj-y	+=  mstar_drv_hotknot.o
obj-y	+=  mstar_drv_hotknot_queue.o
obj-y	+=  mstar_drv_jni_interface.o
