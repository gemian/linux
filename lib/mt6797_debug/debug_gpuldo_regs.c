// SPDX-License-Identifier: GPL-2.0

#include <linux/kernel.h>
#include <linux/module.h>
#include "regdump.h"

#define INFRACFG_BASE 0x10001000

DEFINE_REGISTER(GPULDO_CTRL_0, 0xFBC, "GPU LDO Control 0",
	REGISTER_BIT_ON(0, "RG_GPULDO_0_EN"),
	REGISTER_BIT_ON(1, "RG_GPULDO_1_EN"),
	REGISTER_BIT_ON(2, "RG_GPULDO_2_EN"),
	REGISTER_BIT_ON(3, "RG_GPULDO_3_EN"),
	REGISTER_BIT_ON(4, "RG_GPULDO_4_EN"),
	REGISTER_BIT_ON(5, "RG_GPULDO_5_EN"),
	REGISTER_BIT_ON(6, "RG_GPULDO_6_EN"),
	REGISTER_BIT_ON(7, "RG_GPULDO_7_EN"),
	REGISTER_BIT_ON(8, "RG_GPULDO_8_EN")
);

DEFINE_REGISTER(GPULDO_CTRL_1, 0xFC0, "GPU LDO Control 1",
	REGISTER_FIELD(0, 3, "RG_GPULDO_VOSE_0"),
	REGISTER_BIT_ENABLED(4, "RG_GPULDO_LP_0"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_VOSE_1"),
	REGISTER_BIT_ENABLED(12, "RG_GPULDO_LP_1"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_VOSE_2"),
	REGISTER_BIT_ENABLED(20, "RG_GPULDO_LP_2"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_VOSE_3"),
	REGISTER_BIT_ENABLED(28, "RG_GPULDO_LP_3")
);

DEFINE_REGISTER(GPULDO_CTRL_2, 0xFC4, "GPU LDO Control 2",
	REGISTER_FIELD(0, 3, "RG_GPULDO_VOSE_4"),
	REGISTER_BIT_ENABLED(4, "RG_GPULDO_LP_4"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_VOSE_5"),
	REGISTER_BIT_ENABLED(12, "RG_GPULDO_LP_5"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_VOSE_6"),
	REGISTER_BIT_ENABLED(20, "RG_GPULDO_LP_6"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_VOSE_7"),
	REGISTER_BIT_ENABLED(28, "RG_GPULDO_LP_7")
);

DEFINE_REGISTER(GPULDO_CTRL_3, 0xFC8, "GPU LDO Control 3",
	REGISTER_FIELD(0, 3, "RG_GPULDO_VOSE_8"),
	REGISTER_BIT_ENABLED(4, "RG_GPULDO_LP_8")
);

DEFINE_REGISTER(GPULDO_CTRL_4, 0xFCC, "GPU LDO Control 4",
	REGISTER_FIELD(0, 3, "RG_GPULDO_CAL_0"),
	REGISTER_FIELD(4, 7, "RG_GPULDO_CAL_1"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_CAL_2"),
	REGISTER_FIELD(12, 15, "RG_GPULDO_CAL_3"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_CAL_4"),
	REGISTER_FIELD(20, 21, "RG_GPULDO_CAL_5"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_CAL_6"),
	REGISTER_FIELD(28, 31, "RG_GPULDO_CAL_7")
);

DEFINE_REGISTER(GPULDO_CTRL_5, 0xFD0, "GPU LDO Control 5",
	REGISTER_BIT_ON(0, "RG_GPULDO_BYPASS_0"),
	REGISTER_BIT_ON(1, "RG_GPULDO_BYPASS_1"),
	REGISTER_BIT_ON(2, "RG_GPULDO_BYPASS_2"),
	REGISTER_BIT_ON(3, "RG_GPULDO_BYPASS_3"),
	REGISTER_BIT_ON(4, "RG_GPULDO_BYPASS_4"),
	REGISTER_BIT_ON(5, "RG_GPULDO_BYPASS_5"),
	REGISTER_BIT_ON(6, "RG_GPULDO_BYPASS_6"),
	REGISTER_BIT_ON(7, "RG_GPULDO_BYPASS_7"),
	REGISTER_BIT_ON(8, "RG_GPULDO_STB_0"),
	REGISTER_BIT_ON(9, "RG_GPULDO_STB_1"),
	REGISTER_BIT_ON(10, "RG_GPULDO_STB_2"),
	REGISTER_BIT_ON(11, "RG_GPULDO_STB_3"),
	REGISTER_BIT_ON(12, "RG_GPULDO_STB_4"),
	REGISTER_BIT_ON(13, "RG_GPULDO_STB_5"),
	REGISTER_BIT_ON(14, "RG_GPULDO_STB_6"),
	REGISTER_BIT_ON(15, "RG_GPULDO_STB_7"),
	REGISTER_BIT_ON(16, "RG_GPULDO_NDIS_0"),
	REGISTER_BIT_ON(17, "RG_GPULDO_NDIS_1"),
	REGISTER_BIT_ON(18, "RG_GPULDO_NDIS_2"),
	REGISTER_BIT_ON(19, "RG_GPULDO_NDIS_3"),
	REGISTER_BIT_ON(20, "RG_GPULDO_NDIS_4"),
	REGISTER_BIT_ON(21, "RG_GPULDO_NDIS_5"),
	REGISTER_BIT_ON(22, "RG_GPULDO_NDIS_6"),
	REGISTER_BIT_ON(23, "RG_GPULDO_NDIS_7"),
	REGISTER_BIT_ON(24, "RG_GPULDO_BYPASS_8"),
	REGISTER_BIT_ON(25, "RG_GPULDO_STB_8"),
	REGISTER_BIT_ON(26, "RG_GPULDO_NDIS_8")
);

DEFINE_REGISTER(GPULDO_CTRL_6, 0xFD4, "GPU LDO Control 6",
	REGISTER_BIT_ON(0, "RG_GPULDO_PROBE_ENB_0"),
	REGISTER_BIT_ON(1, "RG_GPULDO_PROBE_ENB_1"),
	REGISTER_BIT_ON(2, "RG_GPULDO_PROBE_ENB_2"),
	REGISTER_BIT_ON(3, "RG_GPULDO_PROBE_ENB_3"),
	REGISTER_BIT_ON(4, "RG_GPULDO_PROBE_ENB_4"),
	REGISTER_BIT_ON(5, "RG_GPULDO_PROBE_ENB_5"),
	REGISTER_BIT_ON(6, "RG_GPULDO_PROBE_ENB_6"),
	REGISTER_BIT_ON(7, "RG_GPULDO_PROBE_ENB_7"),
	REGISTER_BIT_ON(8, "RG_GPULDO_PROBE_EN_V_0"),
	REGISTER_BIT_ON(9, "RG_GPULDO_PROBE_EN_V_1"),
	REGISTER_BIT_ON(10, "RG_GPULDO_PROBE_EN_V_2"),
	REGISTER_BIT_ON(11, "RG_GPULDO_PROBE_EN_V_3"),
	REGISTER_BIT_ON(12, "RG_GPULDO_PROBE_EN_V_4"),
	REGISTER_BIT_ON(13, "RG_GPULDO_PROBE_EN_V_5"),
	REGISTER_BIT_ON(14, "RG_GPULDO_PROBE_EN_V_6"),
	REGISTER_BIT_ON(15, "RG_GPULDO_PROBE_EN_V_7"),
	REGISTER_BIT_ON(16, "RG_GPULDO_PROBE_EN_I_0"),
	REGISTER_BIT_ON(17, "RG_GPULDO_PROBE_EN_I_1"),
	REGISTER_BIT_ON(18, "RG_GPULDO_PROBE_EN_I_2"),
	REGISTER_BIT_ON(19, "RG_GPULDO_PROBE_EN_I_3"),
	REGISTER_BIT_ON(20, "RG_GPULDO_PROBE_EN_I_4"),
	REGISTER_BIT_ON(21, "RG_GPULDO_PROBE_EN_I_5"),
	REGISTER_BIT_ON(22, "RG_GPULDO_PROBE_EN_I_6"),
	REGISTER_BIT_ON(23, "RG_GPULDO_PROBE_EN_I_7"),
	REGISTER_BIT_ON(24, "RG_GPULDO_PROBE_EN_8"),
	REGISTER_BIT_ON(25, "RG_GPULDO_PROBE_EN_V_8"),
	REGISTER_BIT_ON(26, "RG_GPULDO_PROBE_EN_I_8")
);

DEFINE_REGISTER(GPULDO_CTRL_7, 0xFD8, "GPU LDO Control 7",
	REGISTER_FIELD(0, 3, "RG_GPULDO_RSV_H_0"),
	REGISTER_FIELD(4, 7, "RG_GPULDO_RSV_H_1"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_RSV_H_2"),
	REGISTER_FIELD(12, 15, "RG_GPULDO_RSV_H_3"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_RSV_H_4"),
	REGISTER_FIELD(20, 23, "RG_GPULDO_RSV_H_5"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_RSV_H_6"),
	REGISTER_FIELD(28, 31, "RG_GPULDO_RSV_H_7")
);

DEFINE_REGISTER(GPULDO_CTRL_8, 0xFDC, "GPU LDO Control 8",
	REGISTER_FIELD(0, 3, "RG_GPULDO_RSV_M_0"),
	REGISTER_FIELD(4, 7, "RG_GPULDO_RSV_M_1"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_RSV_M_2"),
	REGISTER_FIELD(12, 15, "RG_GPULDO_RSV_M_3"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_RSV_M_4"),
	REGISTER_FIELD(20, 23, "RG_GPULDO_RSV_M_5"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_RSV_M_6"),
	REGISTER_FIELD(28, 31, "RG_GPULDO_RSV_M_7")
);
DEFINE_REGISTER(GPULDO_CTRL_9, 0xFE0, "GPU LDO Control 9",
	REGISTER_FIELD(0, 3, "RG_GPULDO_RSV_L_0"),
	REGISTER_FIELD(4, 7, "RG_GPULDO_RSV_L_1"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_RSV_L_2"),
	REGISTER_FIELD(12, 15, "RG_GPULDO_RSV_L_3"),
	REGISTER_FIELD(16, 19, "RG_GPULDO_RSV_L_4"),
	REGISTER_FIELD(20, 23, "RG_GPULDO_RSV_L_5"),
	REGISTER_FIELD(24, 27, "RG_GPULDO_RSV_L_6"),
	REGISTER_FIELD(28, 31, "RG_GPULDO_RSV_L_7")
);
DEFINE_REGISTER(GPULDO_CTRL_10, 0xFE4, "GPU LDO Control 10",
	REGISTER_FIELD(0, 3, "RG_GPULDO_RSV_H_8"),
	REGISTER_FIELD(4, 7, "RG_GPULDO_RSV_M_8"),
	REGISTER_FIELD(8, 11, "RG_GPULDO_RSV_L_8"),
	REGISTER_FIELD(12, 15, "RG_GPULDO_CAL_8"),
	REGISTER_FIELD(24, 31, "GPU_LDO_CAL_EN")
);

void __init mt6797_debug_gpuldo_regs_init(struct dentry *regs_dir)
{
	struct dentry *dir = debugfs_create_dir("gpuldo", regs_dir);
	PRINT_IF_ERROR(dir);

	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_0));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_1));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_2));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_3));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_4));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_5));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_6));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_7));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_8));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_9));
	PRINT_IF_ERROR(REGISTER_FILE(dir, INFRACFG_BASE, GPULDO_CTRL_10));
}
