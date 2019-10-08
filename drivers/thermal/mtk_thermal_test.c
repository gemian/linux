// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 Adam Boardman
 */

#include <kunit/test.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/nvmem-consumer.h>
#include <linux/nvmem-provider.h>

struct readable_value {
	u32 addr;
	u32 value;
};

struct readable_value readable[] = {
	{
		.addr = 0x98,
		.value = 0
	},
	{
		.addr = 0x9c,
		.value = 0
	}
};

#define readl readl
static u32 readl(const volatile void __iomem *addr) {
	int i;
	for (i=0; i < ARRAY_SIZE(readable); i++) {
		if (readable[i].addr == (u32)addr) {
			return readable[i].value;
		}
	}
	return 0;
}

#include <linux/dma-mapping.h>
#include "mtk_thermal.c"

void __iomem *nvram_dest_ptr = NULL;

static struct resource mtk_efuse_resources[] = {
	[0] = {
		.start	= 0x10206000,
		.end	= 0x10207000,
		.flags	= IORESOURCE_MEM,
	},
};

void __iomem *devm_platform_ioremap_resource(struct platform_device *pdev,
											 unsigned int index)
{
	if (!nvram_dest_ptr) {
		nvram_dest_ptr = devm_kzalloc(&pdev->dev, 0x1000, GFP_KERNEL);
	}

	return nvram_dest_ptr;
}

void __iomem *devm_ioremap_resource(struct device *dev,
									const struct resource *res)
{
	if (!nvram_dest_ptr) {
		nvram_dest_ptr = devm_kzalloc(dev, resource_size(res), GFP_KERNEL);
	}

	return nvram_dest_ptr;
}

struct platform_device *of_platform_device_create_with_res(
	struct device_node *np,
	const char *bus_id,
	struct device *parent,
	struct resource	*resource,
	int resource_size)
{
	struct platform_device *dev;

	if (!of_device_is_available(np) ||
		of_node_test_and_set_flag(np, OF_POPULATED))
		return NULL;

	dev = of_device_alloc(np, bus_id, parent);
	if (!dev)
		goto err_clear_flag;

	dev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	if (!dev->dev.dma_mask)
		dev->dev.dma_mask = &dev->dev.coherent_dma_mask;
	dev->dev.bus = &platform_bus_type;

	if (resource) {
		dev->resource = resource;
		dev->num_resources = resource_size;
	}

	if (of_device_add(dev) != 0) {
		platform_device_put(dev);
		goto err_clear_flag;
	}

	return dev;

	err_clear_flag:
	of_node_clear_flag(np, OF_POPULATED);
	return NULL;
}

static struct platform_device *mtk_thermal_calibration_init(struct kunit *test, uint64_t io_base_addr, uint32_t efuse_calib_addr, uint32_t efuse_calib_size)
{
	int ret;
	struct platform_device *base_pdev;
	struct platform_device *efuse_pdev;
	struct platform_device *thermal_pdev;
	struct device_node *base;
	struct device_node *efuse;
	struct device_node *thermal;
	struct device_node *thermal_calibration_data;
	struct property *efuse_compat;
	struct property *efuse_reg;
	struct property *nvmem_cells;
	struct property *nvmem_cell_names;
	struct property *thermal_calibration_data_reg;
	phandle phandle = 1;
	uint32_t nvmem_cells_value_32;
	uint64_t value_64_2[2];
	uint32_t calib_value_32[2];
	char* nvmem_cells_value_32_buf;
	char* value_64_2_buf;
	char* calib_value_32_buf;

	base = kunit_kzalloc(test, sizeof(*base), GFP_KERNEL);
	efuse = kunit_kzalloc(test, sizeof(*efuse), GFP_KERNEL);
	thermal = kunit_kzalloc(test, sizeof(*thermal), GFP_KERNEL);
	thermal_calibration_data = kunit_kzalloc(test, sizeof(*thermal_calibration_data), GFP_KERNEL);
	efuse_compat = kunit_kzalloc(test, sizeof(*efuse_compat), GFP_KERNEL);
	efuse_reg = kunit_kzalloc(test, sizeof(*efuse_reg), GFP_KERNEL);
	nvmem_cells = kunit_kzalloc(test, sizeof(*nvmem_cells), GFP_KERNEL);
	nvmem_cell_names = kunit_kzalloc(test, sizeof(*nvmem_cell_names), GFP_KERNEL);
	thermal_calibration_data_reg = kunit_kzalloc(test, sizeof(*thermal_calibration_data_reg), GFP_KERNEL);
	nvmem_cells_value_32_buf = kunit_kzalloc(test, sizeof(nvmem_cells_value_32), GFP_KERNEL);
	value_64_2_buf = kunit_kzalloc(test, sizeof(value_64_2), GFP_KERNEL);
	calib_value_32_buf = kunit_kzalloc(test, sizeof(calib_value_32), GFP_KERNEL);

	base->kobj = *kobject_create_and_add("base", firmware_kobj);
	efuse->kobj = *kobject_create_and_add("efuse", firmware_kobj);
	thermal->kobj = *kobject_create_and_add("thermal", firmware_kobj);
	thermal_calibration_data->kobj = *kobject_create_and_add("thermal_calibration_data", firmware_kobj);

	base_pdev = of_device_alloc(base, "base", NULL);
	of_device_add(base_pdev);
	ret = of_platform_populate(efuse, NULL, NULL, &base_pdev->dev);
	ret = of_platform_populate(thermal, NULL, NULL, &base_pdev->dev);
	ret = of_platform_populate(thermal_calibration_data, NULL, NULL, &base_pdev->dev);

	base->phandle = phandle++;

	efuse->name = "efuse";
	efuse->full_name = "efuse";
	efuse->phandle = phandle++;

	thermal->name = "thermal";
	thermal->full_name = "thermal";
	thermal->phandle = phandle++;

	thermal_calibration_data->phandle = phandle++;
	thermal_calibration_data->name = "thermal_calibration_data";
	thermal_calibration_data->full_name = "thermal_calibration_data";

	efuse_compat->name = "compatible";
	efuse_compat->value =  "mediatek,efuse";
	efuse_compat->length =  strlen(efuse_compat->value)+1;
	KUNIT_EXPECT_EQ(test, 0, of_add_property(efuse, efuse_compat));
	efuse_reg->name = "reg";
	value_64_2[0] = cpu_to_be64(io_base_addr);
	value_64_2[1] = cpu_to_be64(0x1000);
	memcpy(value_64_2_buf, &value_64_2, sizeof (value_64_2));
	efuse_reg->value = value_64_2_buf;
	efuse_reg->length =  sizeof (value_64_2_buf);
	KUNIT_EXPECT_EQ(test, 0, of_add_property(efuse, efuse_reg));

	nvmem_cells->name = "nvmem-cells";
	nvmem_cells_value_32 = cpu_to_be32(thermal_calibration_data->phandle);
	memcpy(nvmem_cells_value_32_buf, &nvmem_cells_value_32, sizeof (nvmem_cells_value_32));
	nvmem_cells->value = nvmem_cells_value_32_buf;
	nvmem_cells->length = sizeof (nvmem_cells_value_32);
	KUNIT_EXPECT_EQ(test, 0, of_add_property(thermal, nvmem_cells));
	nvmem_cell_names->name = "nvmem-cell-names";
	nvmem_cell_names->value =  "calibration-data";
	nvmem_cell_names->length =  strlen(nvmem_cell_names->value)+1;
	KUNIT_EXPECT_EQ(test, 0, of_add_property(thermal, nvmem_cell_names));

	thermal_calibration_data_reg->name = "reg";
	calib_value_32[0] = cpu_to_be32(efuse_calib_addr);
	calib_value_32[1] = cpu_to_be32(efuse_calib_size);
	memcpy(calib_value_32_buf, &calib_value_32, sizeof (calib_value_32));
	thermal_calibration_data_reg->value = calib_value_32_buf;
	thermal_calibration_data_reg->length = sizeof(calib_value_32);
	KUNIT_EXPECT_EQ(test, 0, of_add_property(thermal_calibration_data, thermal_calibration_data_reg));

	of_root = base;
	of_root->full_name = "/";
	base->child = thermal;
	thermal->parent = base;
	thermal->sibling = efuse;
	efuse->parent = base;
	efuse->child = thermal_calibration_data;
	thermal_calibration_data->parent = efuse;

	thermal_pdev = of_platform_device_create(thermal, "thermal", &base_pdev->dev);
	efuse_pdev = of_platform_device_create_with_res(efuse, "efuse", &base_pdev->dev, mtk_efuse_resources, ARRAY_SIZE(mtk_efuse_resources));
	of_platform_device_create(thermal_calibration_data, "thermal_calibration_data", &base_pdev->dev);

	return thermal_pdev;
}

static void mtk_thermal_mt6797_calibration(struct kunit *test)
{
	int ret;
	struct mtk_thermal mt;
	struct platform_device *thermal_pdev;
	u32 efuse_cal_buf[]={0x1f8e66d, 0x9963d0e5, 0x7237c000};

	thermal_pdev = mtk_thermal_calibration_init(test, 0x102060000, 0x180, 0xc);

	/*
	 * Calibration data from 3.18 kernel
	 * [calibration] temp0=0x9963d0e5, temp1=0x1f8e66d, temp2=0x7237c000
	 * [CPU_Thermal][cal] g_adc_ge_t      = 613
	 * [CPU_Thermal][cal] g_adc_oe_t      = 573
	 * [CPU_Thermal][cal] g_degc_cali     = 54
	 * [CPU_Thermal][cal] g_adc_cali_en_t = 1
	 * [CPU_Thermal][cal] g_o_slope       = 0
	 * [CPU_Thermal][cal] g_o_slope_sign  = 0
	 * [CPU_Thermal][cal] g_id            = 0
	 * [CPU_Thermal][cal] g_o_vtsmcu1     = 252
	 * [CPU_Thermal][cal] g_o_vtsmcu2     = 230
	 * [CPU_Thermal][cal] g_o_vtsmcu3     = 229
	 * [CPU_Thermal][cal] g_o_vtsmcu4     = 228
	 * [CPU_Thermal][cal] g_o_vtsabb     = 223
	 */
	memcpy(nvram_dest_ptr + 0x180, efuse_cal_buf, sizeof (efuse_cal_buf));
	mt.conf = (void *)&mt6797_thermal_data;

	ret = mtk_thermal_get_calibration_data(&thermal_pdev->dev, &mt);

	KUNIT_EXPECT_EQ(test, 613, mt.adc_ge);
	KUNIT_EXPECT_EQ(test, 54, mt.degc_cali);
	KUNIT_EXPECT_EQ(test, 0, mt.o_slope);
	KUNIT_EXPECT_EQ(test, 252, mt.vts[0]);
	KUNIT_EXPECT_EQ(test, 230, mt.vts[1]);
	KUNIT_EXPECT_EQ(test, 229, mt.vts[2]);
	KUNIT_EXPECT_EQ(test, 228, mt.vts[3]);
	KUNIT_EXPECT_EQ(test, 260, mt.vts[4]);
	// VTSABB - even though we have a calibration value for this its not used on 6797 so doesn't get loaded so expect the default
	KUNIT_EXPECT_EQ(test, 260, mt.vts[5]);
}

static void mtk_thermal_mt6797_temp_calculation(struct kunit *test)
{
	struct mtk_thermal mt;
	struct mtk_thermal_bank mtb;
	mt.conf = (void *)&mt6797_thermal_data;
	mt.adc_ge = 613;
	mt.degc_cali = 54;
	mt.o_slope = 0;
	mt.vts[0] = 252;
	mt.vts[1] = 230;
	mt.vts[2] = 229;
	mt.vts[3] = 228;
	mt.vts[4] = 260;
	mt.vts[5] = 223;
	/*
	 * Values from mainline code
	 * Bank: 0, Sensor: 0, Raw: 36347, Temp: 29737
	 * Bank: 1, Sensor: 0, Raw: 36343, Temp: 30213
	 * Bank: 2, Sensor: 0, Raw: 36344, Temp: 30094
	 * Bank: 2, Sensor: 1, Raw: 36333, Temp: 28785
	 * Bank: 3, Sensor: 0, Raw: 36346, Temp: 29856
	 * Bank: 4, Sensor: 0, Raw: 36347, Temp: 29737
	 * Bank: 5, Sensor: 0, Raw: 36343, Temp: 30213
	 */
	KUNIT_EXPECT_EQ(test, 29737, raw_to_mcelsius(&mt, 0, 36347));
	KUNIT_EXPECT_EQ(test, 30213, raw_to_mcelsius(&mt, 0, 36343));
	KUNIT_EXPECT_EQ(test, 30094, raw_to_mcelsius(&mt, 0, 36344));
	KUNIT_EXPECT_EQ(test, 28785, raw_to_mcelsius(&mt, 1, 36333));
	KUNIT_EXPECT_EQ(test, 29856, raw_to_mcelsius(&mt, 0, 36346));
	KUNIT_EXPECT_EQ(test, 29737, raw_to_mcelsius(&mt, 0, 36347));
	KUNIT_EXPECT_EQ(test, 30213, raw_to_mcelsius(&mt, 0, 36343));

	/*
	 * Values from 3.18 GeminiPDA after enabling logging:
	 * echo 1 > /proc/driver/thermal/tzcpu_log
	 */
	// tscpu_thermal_read_bank_temp order 0 bank 0 type 0 tscpu_bank_ts 31400 tscpu_bank_ts_r 3565
	KUNIT_EXPECT_LE(test, 31400, raw_to_mcelsius(&mt, mt.conf->vts_index[0], 3565));
	KUNIT_EXPECT_GE(test, 31405, raw_to_mcelsius(&mt, mt.conf->vts_index[0], 3565));
	KUNIT_EXPECT_EQ(test, 31403, raw_to_mcelsius(&mt, mt.conf->vts_index[0], 3565));

	// tscpu_thermal_read_bank_temp order 0 bank 1 type 3 tscpu_bank_ts 27300 tscpu_bank_ts_r 3575
	KUNIT_EXPECT_LE(test, 27300, raw_to_mcelsius(&mt, mt.conf->vts_index[1], 3575));
	KUNIT_EXPECT_GE(test, 27400, raw_to_mcelsius(&mt, mt.conf->vts_index[1], 3575));
	KUNIT_EXPECT_EQ(test, 27357, raw_to_mcelsius(&mt, mt.conf->vts_index[1], 3575));

	//tscpu_thermal_read_bank_temp order 0 bank 2 type 1 tscpu_bank_ts 31900 tscpu_bank_ts_r 3538
	KUNIT_EXPECT_LE(test, 31900, raw_to_mcelsius(&mt, mt.conf->vts_index[2], 3538));
	KUNIT_EXPECT_GE(test, 32000, raw_to_mcelsius(&mt, mt.conf->vts_index[2], 3538));
	KUNIT_EXPECT_EQ(test, 31998, raw_to_mcelsius(&mt, mt.conf->vts_index[2], 3538));

	////tscpu_thermal_read_bank_temp order 1 bank 2 type 2 tscpu_bank_ts 27500 tscpu_bank_ts_r 3574
	KUNIT_EXPECT_LE(test, 27500, raw_to_mcelsius(&mt, mt.conf->vts_index[3], 3574));
	KUNIT_EXPECT_GE(test, 27600, raw_to_mcelsius(&mt, mt.conf->vts_index[3], 3574));
	KUNIT_EXPECT_EQ(test, 27595, raw_to_mcelsius(&mt, mt.conf->vts_index[3], 3574));

	//tscpu_thermal_read_bank_temp order 0 bank 3 type 1 tscpu_bank_ts 32100 tscpu_bank_ts_r 3537
	KUNIT_EXPECT_EQ(test, 32117, raw_to_mcelsius(&mt, mt.conf->vts_index[4], 3537));
	//tscpu_thermal_read_bank_temp order 0 bank 4 type 1 tscpu_bank_ts 31900 tscpu_bank_ts_r 3538
	KUNIT_EXPECT_EQ(test, 31998, raw_to_mcelsius(&mt, mt.conf->vts_index[5], 3538));
	//tscpu_thermal_read_bank_temp order 0 bank 5 type 1 tscpu_bank_ts 32300 tscpu_bank_ts_r 3535
	KUNIT_EXPECT_EQ(test, 32355, raw_to_mcelsius(&mt, mt.conf->vts_index[6], 3535));

	mtb.mt = &mt;
	mtb.id = 0;
	readable[0].value = 3565;
	KUNIT_EXPECT_EQ(test, 31403, mtk_thermal_bank_temperature(&mtb));

	mtb.id = 1;
	readable[0].value = 3565;
	KUNIT_EXPECT_EQ(test, 28547, mtk_thermal_bank_temperature(&mtb));

	mtb.id = 2;
	readable[0].value = 3565;
	readable[1].value = 3565;
	KUNIT_EXPECT_EQ(test, 28785, mtk_thermal_bank_temperature(&mtb));

	mtb.id = 3;
	readable[0].value = 3565;
	KUNIT_EXPECT_EQ(test, 28785, mtk_thermal_bank_temperature(&mtb));

	mtb.id = 4;
	readable[0].value = 3565;
	KUNIT_EXPECT_EQ(test, 30213, mtk_thermal_bank_temperature(&mtb));

	mtb.id = 5;
	readable[0].value = 3565;
	KUNIT_EXPECT_EQ(test, 28785, mtk_thermal_bank_temperature(&mtb));
}

static struct kunit_case mtk_thermal_test_cases[] = {
	KUNIT_CASE(mtk_thermal_mt6797_calibration),
	KUNIT_CASE(mtk_thermal_mt6797_temp_calculation),
	{}
};

static struct kunit_suite mtk_thermal_test_suite = {
	.name = "mtk-thermal",
	.test_cases = mtk_thermal_test_cases,
};
kunit_test_suite(mtk_thermal_test_suite);