# SPDX-License-Identifier: GPL-2.0

ccflags-$(CONFIG_MT6797_DEBUG_FORCE_UART) += -DMT6797_DEBUG_FORCE_UART=1

obj-$(CONFIG_MT6797_DEBUG) += mt6797_debug.o
mt6797_debug-objs := main.o
mt6797_debug-objs += regdump.o
mt6797_debug-objs += debug_bus_protect_regs.o
mt6797_debug-objs += debug_gpio_regs.o
mt6797_debug-objs += debug_gpuldo_regs.o
mt6797_debug-objs += debug_emi_mpu.o
mt6797_debug-objs += debug_emi_regs.o
mt6797_debug-objs += debug_io_cfg_l_regs.o
mt6797_debug-objs += debug_m4u_regs.o
mt6797_debug-objs += debug_pericfg_regs.o
mt6797_debug-objs += debug_pll.o
mt6797_debug-objs += debug_psci.o
mt6797_debug-objs += debug_spm_regs.o
mt6797_debug-objs += debug_uart_regs.o
mt6797_debug-objs += debug_usb_sifslv_u2phy_regs.o
mt6797_debug-objs += force_uart.o
