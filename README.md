# Kernel for lt80 aka b15q (does not build yet)

You have to make this repo avaible in your cm-build-system under kernel/mediatek/mt6582/.

Enable these lines in device/mediatek/mt6582/BoardConfig.mk:
#TARGET_KERNEL_CONFIG := mt6582_defconfig
#TARGET_KERNEL_SOURCE := kernel/mediatek/mt6582/kernel

by remove # in begin.

To start the build-progress, you can do

mka bootimage


