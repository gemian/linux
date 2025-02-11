// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include "regdump.h"

void mt6797_debug_force_uart(void);

void mt6797_debug_bus_protect_regs_init(struct dentry *regs_dir);
void mt6797_debug_gpio_regs_init(struct dentry *regs_dir);
void mt6797_debug_gpuldo_regs_init(struct dentry *regs_dir);
void mt6797_debug_emi_mpu_init(struct dentry *debug_dir);
void mt6797_debug_emi_regs_init(struct dentry *regs_dir);
void mt6797_debug_io_cfg_l_regs_init(struct dentry *regs_dir);
void mt6797_debug_m4u_regs_init(struct dentry *regs_dir);
void mt6797_debug_pericfg_regs_init(struct dentry *regs_dir);
void mt6797_debug_pll_init(struct dentry *debug_dir);
void mt6797_debug_psci_init(struct dentry *debug_dir);
void mt6797_debug_spm_regs_init(struct dentry *debug_dir);
void mt6797_debug_uart_regs_init(struct dentry *regs_dir);
void mt6797_debug_usb_sifslv_u2phy_init(struct dentry *regs_dir);

static struct dentry *debug_dir;

static int __init mt6797_debug_init(void)
{
	struct dentry * regs_dir;

#ifdef MT6797_DEBUG_FORCE_UART
	mt6797_debug_force_uart();
#endif

	debug_dir = debugfs_create_dir("mt6797", NULL);
	PRINT_IF_ERROR(debug_dir);
	regs_dir = debugfs_create_dir("regs", debug_dir);
	PRINT_IF_ERROR(regs_dir);
 	pr_info("[%s] created debugfs",__func__);
	mt6797_debug_bus_protect_regs_init(regs_dir);
	mt6797_debug_gpio_regs_init(regs_dir);
	mt6797_debug_gpuldo_regs_init(regs_dir);
	mt6797_debug_emi_mpu_init(debug_dir);
	mt6797_debug_emi_regs_init(regs_dir);
	mt6797_debug_io_cfg_l_regs_init(regs_dir);
	mt6797_debug_m4u_regs_init(regs_dir);
	mt6797_debug_pericfg_regs_init(regs_dir);
	mt6797_debug_pll_init(debug_dir);
	mt6797_debug_psci_init(debug_dir);
	mt6797_debug_spm_regs_init(regs_dir);
	mt6797_debug_uart_regs_init(regs_dir);
	mt6797_debug_usb_sifslv_u2phy_init(regs_dir);
	pr_info("[%s] inits finished",__func__);

	return 0;
}

static void __exit mt6797_debug_exit(void)
{
	debugfs_remove_recursive(debug_dir);
	pr_info("[%s] exited",__func__);
}

module_init(mt6797_debug_init);
module_exit(mt6797_debug_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jasper Mattsson <jasu@njomotys.info>");
MODULE_DESCRIPTION("MediaTek MT6797 debug functionality");
