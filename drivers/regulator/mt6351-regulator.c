/*
 * Copyright (c) 2017 MediaTek Inc.
 * Author: Jimmy-Yj Huang <jimmy-yj.huang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/mfd/mt6397/core.h>
#include <linux/mfd/mt6351/registers.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/mt6351-regulator.h>
#include <linux/regulator/of_regulator.h>

/*
 * MT6351 regulators' information
 *
 * @desc: standard fields of regulator description.
 * @qi: Mask for query enable signal status of regulators
 * @vselon_reg: Register sections for hardware control mode of bucks
 * @vselctrl_reg: Register for controlling the buck control mode.
 * @vselctrl_mask: Mask for query buck's voltage control mode.
 */
struct mt6351_regulator_info {
    struct regulator_desc desc;
    u32 qi;
    u32 vselon_reg;
    u32 vselctrl_reg;
    u32 vselctrl_mask;
};

/*
 * MTK regulators' init data
 *
 * @id: chip id
 * @size: num of regulators
 * @regulator_info: regulator info.
 */
struct mt_regulator_init_data {
    u32 id;
    u32 size;
    struct mt6351_regulator_info *regulator_info;
};

#define MT_BUCK(ver, match, vreg, min, max, step, volt_ranges, \
        enreg, vosel, vosel_mask, voselon, vosel_ctrl)        \
[ver##_ID_##vreg] = {                            \
    .desc = {                            \
        .name = #vreg,                        \
        .of_match = of_match_ptr(match),            \
        .ops = &mt6351_volt_range_ops,                \
        .type = REGULATOR_VOLTAGE,                \
        .id = ver##_ID_##vreg,                    \
        .owner = THIS_MODULE,                    \
        .n_voltages = (max - min) / step + 1,            \
        .linear_ranges = volt_ranges,                \
        .n_linear_ranges = ARRAY_SIZE(volt_ranges),        \
        .vsel_reg = vosel,                    \
        .vsel_mask = vosel_mask,                \
        .enable_reg = enreg,                    \
        .enable_mask = BIT(0),                    \
    },                                \
    .qi = BIT(13),                            \
    .vselon_reg = voselon,                        \
    .vselctrl_reg = vosel_ctrl,                    \
    .vselctrl_mask = BIT(1),                    \
}

#define MT_LDO(ver, match, vreg, ldo_volt_table, enreg, enbit, \
        vosel, vosel_mask)                    \
[ver##_ID_##vreg] = {                            \
    .desc = {                            \
        .name = #vreg,                        \
        .of_match = of_match_ptr(match),            \
        .ops = &mt6351_volt_table_ops,                \
        .type = REGULATOR_VOLTAGE,                \
        .id = ver##_ID_##vreg,                    \
        .owner = THIS_MODULE,                    \
        .n_voltages = ARRAY_SIZE(ldo_volt_table),        \
        .volt_table = ldo_volt_table,                \
        .vsel_reg = vosel,                    \
        .vsel_mask = vosel_mask,                \
        .enable_reg = enreg,                    \
        .enable_mask = BIT(enbit),                \
    },                                \
    .qi = BIT(15),                            \
}

#define MT_REG_FIXED(ver, match, vreg, enreg, enbit, volt)        \
[ver##_ID_##vreg] = {                            \
    .desc = {                            \
        .name = #vreg,                        \
        .of_match = of_match_ptr(match),            \
        .ops = &mt6351_volt_fixed_ops,                \
        .type = REGULATOR_VOLTAGE,                \
        .id = ver##_ID_##vreg,                    \
        .owner = THIS_MODULE,                    \
        .n_voltages = 1,                    \
        .enable_reg = enreg,                    \
        .enable_mask = BIT(enbit),                \
        .min_uV = volt,                        \
    },                                \
    .qi = BIT(15),                            \
}

static const struct regulator_linear_range buck_volt_range1[] = {
        REGULATOR_LINEAR_RANGE(600000, 0, 0x7f, 6250),
};

static const struct regulator_linear_range buck_volt_range2[] = {
        REGULATOR_LINEAR_RANGE(700000, 0, 0x7f, 6250),
};

static const struct regulator_linear_range buck_volt_range3[] = {
        REGULATOR_LINEAR_RANGE(1500000, 0, 0x1f, 20000),
};

static const u32 ldo_volt_table1[] = {
        1800000, 2200000, 2375000, 2800000,
};

static const u32 ldo_volt_table2[] = {
        1200000, 1300000, 1700000, 1800000, 1860000, 2760000, 3000000, 3100000,
};

static const u32 ldo_volt_table3[] = {
        3000000, 3300000,
};

static const u32 ldo_volt_table4[] = {
        1500000, 1800000, 2500000, 2800000,
};

static const u32 ldo_volt_table5[] = {
        3300000, 3400000, 3500000, 3600000,
};

static const u32 ldo_volt_table5_v2[] = {
        1200000, 1000000, 1500000, 1800000, 2500000, 2800000, 3000000, 3300000,
};

static const u32 ldo_volt_table6[] = {
        1200000, 1300000, 1500000, 1800000, 2000000, 2900000, 3000000, 3300000,
};

static const u32 ldo_volt_table6_v1[] = {
        2500000, 2900000, 3000000, 3300000,
};

static const u32 ldo_volt_table6_v2[] = {
        1300000, 1800000, 2900000, 3300000,
};

static const u32 ldo_volt_table7[] = {
        1200000, 1300000, 1500000, 1800000, 2000000, 2800000, 3000000, 3300000,
};

static const u32 ldo_volt_table8[] = {
        900000, 950000, 1000000, 1050000, 1100000, 1200000, 1210000,
};

static const u32 ldo_volt_table9[] = {
        900000, 950000, 1000000, 1050000, 1200000, 1500000, 1800000,
};

static const u32 ldo_volt_table10[] = {
        1000000, 1050000, 1100000, 1220000, 1300000, 1500000, 1800000, 1810000,
};

static const u32 ldo_volt_table10_v1[] = {
        1000000, 1200000, 1300000, 1500000, 1800000, 1810000,
};

static const u32 ldo_volt_table11[] = {
        1800000, 2900000, 3000000, 3300000,
};

static int mt6351_get_status(struct regulator_dev *rdev) {
    int ret;
    u32 regval;
    struct mt6351_regulator_info *info = rdev_get_drvdata(rdev);

    ret = regmap_read(rdev->regmap, info->desc.enable_reg, &regval);
    if (ret != 0) {
        dev_err(&rdev->dev, "Failed to get enable reg: %d\n", ret);
        return ret;
    }

    return (regval & info->qi) ? REGULATOR_STATUS_ON : REGULATOR_STATUS_OFF;
}

static struct regulator_ops mt6351_volt_range_ops = {
        .list_voltage = regulator_list_voltage_linear_range,
        .map_voltage = regulator_map_voltage_linear_range,
        .set_voltage_sel = regulator_set_voltage_sel_regmap,
        .get_voltage_sel = regulator_get_voltage_sel_regmap,
        .set_voltage_time_sel = regulator_set_voltage_time_sel,
        .enable = regulator_enable_regmap,
        .disable = regulator_disable_regmap,
        .is_enabled = regulator_is_enabled_regmap,
        .get_status = mt6351_get_status,
};

static struct regulator_ops mt6351_volt_table_ops = {
        .list_voltage = regulator_list_voltage_table,
        .map_voltage = regulator_map_voltage_iterate,
        .set_voltage_sel = regulator_set_voltage_sel_regmap,
        .get_voltage_sel = regulator_get_voltage_sel_regmap,
        .set_voltage_time_sel = regulator_set_voltage_time_sel,
        .enable = regulator_enable_regmap,
        .disable = regulator_disable_regmap,
        .is_enabled = regulator_is_enabled_regmap,
        .get_status = mt6351_get_status,
};

static struct regulator_ops mt6351_volt_fixed_ops = {
        .list_voltage = regulator_list_voltage_linear,
        .enable = regulator_enable_regmap,
        .disable = regulator_disable_regmap,
        .is_enabled = regulator_is_enabled_regmap,
        .get_status = mt6351_get_status,
};

/* The array is indexed by id(MT6351_ID_XXX) */
static struct mt6351_regulator_info mt6351_regulators[] = {
        MT_BUCK(MT6351, "buck_vcore", VCORE, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VCORE_CON2,
                MT6351_BUCK_VCORE_CON4, 0x7f,
                MT6351_BUCK_VCORE_CON5, MT6351_BUCK_VCORE_CON0),
        MT_BUCK(MT6351, "buck_vgpu", VGPU, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VGPU_CON2,
                MT6351_BUCK_VGPU_CON4, 0x7f,
                MT6351_BUCK_VGPU_CON5, MT6351_BUCK_VGPU_CON0),
        MT_BUCK(MT6351, "buck_vmodem", VMODEM, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VMODEM_CON2,
                MT6351_BUCK_VMODEM_CON4, 0x7f,
                MT6351_BUCK_VMODEM_CON5, MT6351_BUCK_VMODEM_CON0),
        MT_BUCK(MT6351, "buck_vmd1", VMD1, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VMD1_CON2,
                MT6351_BUCK_VMD1_CON4, 0x7f,
                MT6351_BUCK_VMD1_CON5, MT6351_BUCK_VMD1_CON0),
        MT_BUCK(MT6351, "buck_vsram_md", VSRAM_MD, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VSRAM_MD_CON2,
                MT6351_BUCK_VSRAM_MD_CON4, 0x7f,
                MT6351_BUCK_VSRAM_MD_CON5, MT6351_BUCK_VSRAM_MD_CON0),
        MT_BUCK(MT6351, "buck_vs1", VS1, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VS1_CON2,
                MT6351_BUCK_VS1_CON4, 0x7f,
                MT6351_BUCK_VS1_CON5, MT6351_BUCK_VS1_CON0),
        MT_BUCK(MT6351, "buck_vs2", VS2, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VS2_CON2,
                MT6351_BUCK_VS2_CON4, 0x7f,
                MT6351_BUCK_VS2_CON5, MT6351_BUCK_VS2_CON0),
        MT_BUCK(MT6351, "buck_vpa", VPA, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VPA_CON2,
                MT6351_BUCK_VPA_CON4, 0x7f,
                MT6351_BUCK_VPA_CON5, MT6351_BUCK_VPA_CON0),
        MT_BUCK(MT6351, "buck_vsram_proc", VSRAM_PROC, 600000, 1393750, 6250,
                buck_volt_range1, MT6351_BUCK_VSRAM_PROC_CON2,
                MT6351_BUCK_VSRAM_PROC_CON4, 0x7f,
                MT6351_BUCK_VSRAM_PROC_CON5, MT6351_BUCK_VSRAM_PROC_CON0),
        MT_LDO(MT6351, "ldo_va18", VA18, ldo_volt_table1,
                MT6351_VA18_ANA_CON0, 1, MT6351_LDO_VA18_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vtcxo24", VTCXO24, ldo_volt_table1,
                MT6351_VTCXO24_ANA_CON0, 1, MT6351_LDO_VTCXO24_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vtcxo28", VTCXO28, ldo_volt_table1,
                MT6351_VTCXO28_ANA_CON0, 1, MT6351_LDO_VTCXO28_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcn28", VCN28, ldo_volt_table1,
                MT6351_VCN28_ANA_CON0, 1, MT6351_LDO_VCN28_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcama", VCAMA, ldo_volt_table4,
                MT6351_VCAMA_ANA_CON0, 1, MT6351_LDO_VCAMA_CON0, 0x300),
        MT_REG_FIXED(MT6351, "ldo_vusb33", VUSB33,
                MT6351_LDO_VUSB33_CON0, 1, 3300000),
        MT_LDO(MT6351, "ldo_vsim1", VSIM1, ldo_volt_table2,
                MT6351_VSIM1_ANA_CON0, 1, MT6351_LDO_VSIM1_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vsim2", VSIM2, ldo_volt_table2,
                MT6351_VSIM2_ANA_CON0, 1, MT6351_LDO_VSIM2_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vemc_3v3", VEMC_3V3, ldo_volt_table3,
                MT6351_VEMC_ANA_CON0, 1, MT6351_LDO_VEMC_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vmch", VMCH, ldo_volt_table3,
                MT6351_VMCH_ANA_CON0, 1, MT6351_LDO_VMCH_CON0, 0x300),
        MT_REG_FIXED(MT6351, "ldo_vio28", VIO28,
                MT6351_LDO_VIO28_CON0, 1, 2800000),
        MT_LDO(MT6351, "ldo_vibr", VIBR, ldo_volt_table7,
                MT6351_VIBR_ANA_CON0, 1, MT6351_LDO_VIBR_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcamd", VCAMD, ldo_volt_table8,
                MT6351_VCAMD_ANA_CON0, 1, MT6351_LDO_VCAMD_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vrf18", VRF18, ldo_volt_table10,
                MT6351_VRF18_ANA_CON0, 1, MT6351_LDO_VRF18_CON0, 0x300),
        MT_REG_FIXED(MT6351, "ldo_vio18", VIO18,
                MT6351_LDO_VIO18_CON0, 1, 1800000),
        MT_LDO(MT6351, "ldo_vcn18", VCN18, ldo_volt_table9,
                MT6351_VCN18_ANA_CON0, 1, MT6351_LDO_VCN18_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcamio", VCAMIO, ldo_volt_table9,
                MT6351_VCAMIO_ANA_CON0, 1, MT6351_LDO_VCAMIO_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vxo22", VXO22, ldo_volt_table1,
                MT6351_VXO22_ANA_CON0, 1, MT6351_LDO_VXO22_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vrf12", VRF12, ldo_volt_table9,
                MT6351_VRF12_ANA_CON0, 1, MT6351_LDO_VRF12_CON0, 0x300),
        MT_LDO(MT6351, "ldo_va10", VA10, ldo_volt_table9,
                MT6351_VA10_ANA_CON0, 1, MT6351_LDO_VA10_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vdram", VDRAM, ldo_volt_table8,
                MT6351_VDRAM_ANA_CON0, 1, MT6351_LDO_VDRAM_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vmipi", VMIPI, ldo_volt_table9,
                MT6351_VMIPI_ANA_CON0, 1, MT6351_LDO_VMIPI_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vgp3", VGP3, ldo_volt_table10,
                MT6351_VGP3_ANA_CON0, 1, MT6351_LDO_VGP3_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vbif28", VBIF28, ldo_volt_table1,
                MT6351_VBIF28_ANA_CON0, 1, MT6351_LDO_VBIF28_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vefuse", VEFUSE, ldo_volt_table2,
                MT6351_VEFUSE_ANA_CON0, 1, MT6351_LDO_VEFUSE_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcn33_bt", VCN33_BT, ldo_volt_table5,
                MT6351_VCN33_ANA_CON0, 1, MT6351_LDO_VCN33_CON0, 0x300),
        MT_LDO(MT6351, "ldo_vcn33_wifi", VCN33_WIFI, ldo_volt_table5,
                MT6351_VCN33_ANA_CON0, 1, MT6351_LDO_VCN33_CON0, 0x300),
        MT_REG_FIXED(MT6351, "ldo_vldo28", VLDO28,
                MT6351_LDO_VLDO28_CON3, 1, 2800000),
        MT_LDO(MT6351, "ldo_vmc", VMC, ldo_volt_table6,
                MT6351_VMC_ANA_CON0, 1, MT6351_LDO_VMC_CON0, 0x300),
        MT_REG_FIXED(MT6351, "ldo_vldo28_0", VLDO28_0,
                MT6351_LDO_VLDO28_CON3, 1, 2800000),
        MT_REG_FIXED(MT6351, "ldo_vldo28_1", VLDO28_1,
                MT6351_LDO_VLDO28_CON4, 1, 2800000),
};

static const struct mt_regulator_init_data mt6351_regulator_init_data = {
        .id = MT6351_HWCID,
        .size = MT6351_MAX_REGULATOR,
        .regulator_info = &mt6351_regulators[0],
};

static const struct platform_device_id mt6351_platform_ids[] = {
        {"mt6351-regulator", 0},
        { /* sentinel */ },
};
MODULE_DEVICE_TABLE(platform, mt6351_platform_ids
);

static const struct of_device_id mt6351_of_match[] = {
        {
                .compatible = "mediatek,mt6351-regulator",
                .data = &mt6351_regulator_init_data,
        },
        {
                /* sentinel */
        }
};
MODULE_DEVICE_TABLE(of, mt6351_of_match
);

static int mt6351_set_buck_vosel_reg(struct platform_device *pdev) {
    const struct of_device_id *of_id;
    struct mt6397_chip *mt6351 = dev_get_drvdata(pdev->dev.parent);
    struct mt6351_regulator_info *mt_regulators;
    struct mt_regulator_init_data *regulator_init_data;
    int i;
    u32 regval, reg;

    of_id = of_match_device(mt6351_of_match, &pdev->dev);
    if (!of_id || !of_id->data)
        return -ENODEV;

    regulator_init_data = (struct mt_regulator_init_data *) of_id->data;
    mt_regulators = regulator_init_data->regulator_info;

    for (i = 0; i < regulator_init_data->size; i++) {
        if ((mt_regulators + i)->vselctrl_reg) {
            reg = (mt_regulators + i)->vselctrl_reg;
            if (regmap_read(mt6351->regmap, reg, &regval) < 0) {
                dev_err(&pdev->dev,
                        "Failed to read buck ctrl\n");
                return -EIO;
            }

            if (regval & (mt_regulators + i)->vselctrl_mask) {
                (mt_regulators + i)->desc.vsel_reg =
                        (mt_regulators + i)->vselon_reg;
            }
        }
    }

    return 0;
}

static int mt6351_regulator_probe(struct platform_device *pdev) {
    const struct of_device_id *of_id;
    struct mt6397_chip *mt6351 = dev_get_drvdata(pdev->dev.parent);
    struct regulator_config config = {};
    struct regulator_dev *rdev;
    int i;
    u32 reg_value, version;

    of_id = of_match_device(mt6351_of_match, &pdev->dev);
    if (!of_id || !of_id->data)
        return -ENODEV;

    regulator_init_data = (struct mt_regulator_init_data *) of_id->data;
    mt_regulators = regulator_init_data->regulator_info;

    /* Query buck controller to select activated voltage register part */
    if (mt6351_set_buck_vosel_reg(pdev))
        return -EIO;

    /* Read PMIC chip revision to update constraints and voltage table */
    if (regmap_read(mt6351->regmap, MT6351_HWCID, &reg_value) < 0) {
        dev_err(&pdev->dev, "Failed to read Chip ID\n");
        return -EIO;
    }
    dev_info(&pdev->dev, "Chip ID = 0x%x\n", reg_value);

    version = (reg_value & 0xFF);
    switch (version) {
        case MT6351_REGULATOR_ID12:
            mt6351_regulators[MT6351_ID_VMC].desc.volt_table =
                    ldo_volt_table5_v2;
            break;
        default:
            break;
    }

    for (i = 0; i < regulator_init_data->size; i++) {
        config.dev = &pdev->dev;
        config.driver_data = (mt_regulators + i);
        config.regmap = mt6351->regmap;
        rdev = devm_regulator_register(&pdev->dev,
                                       &(mt_regulators + i)->desc,
                                       &config);
        if (IS_ERR(rdev)) {
            dev_err(&pdev->dev, "failed to register %s\n",
                    (mt_regulators + i)->desc.name);
            return PTR_ERR(rdev);
        }
    }

    return 0;
}

static struct platform_driver mt6351_regulator_driver = {
        .driver = {
                .name = "mt6351-regulator",
                .of_match_table = of_match_ptr(mt6351_of_match),
        },
        .probe = mt6351_regulator_probe,
        .id_table = mt6351_platform_ids,
};

module_platform_driver(mt6351_regulator_driver);

MODULE_AUTHOR("Jimmy-Yj Huang <Jimmy-YJ.Hunag@mediatek.com>");
MODULE_DESCRIPTION("Regulator Driver for MediaTek MT6351 PMIC");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:mt6351-regulator");