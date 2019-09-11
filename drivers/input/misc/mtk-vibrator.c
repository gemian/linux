/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2016 MediaTek Inc.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/regmap.h>
#include <linux/mfd/mt6351/registers.h>
#include <linux/mfd/mt6397/core.h>

#include "timed_output.h"

#define VERSION				"v 0.1"
#define VIB_DEVICE			"mtk_vibrator"
#define OC_INTR_INIT_DELAY	(3)

//mt6763
//#define VIBR_MT6763_VOSEL_ADDR    MT6356_VIBR_ANA_CON0
//#define VIBR_MT6763_VOSEL_MASK    0xF
//#define VIBR_MT6763_VOSEL_SHIFT   8
//#define VIBR_MT6763_EN_ADDR       MT6356_LDO_VIBR_CON0
//#define VIBR_MT6763_EN_MASK       0x1
//#define VIBR_MT6763_EN_SHIFT      0

//mt6739
//#define VIBR_MT6739_VOSEL_ADDR    MT6357_VIBR_ANA_CON0
//#define VIBR_MT6739_VOSEL_MASK    0xF
//#define VIBR_MT6739_VOSEL_SHIFT   8
//#define VIBR_MT6739_EN_ADDR       MT6357_LDO_VIBR_CON0
//#define VIBR_MT6739_EN_MASK       0x1
//#define VIBR_MT6739_EN_SHIFT      0

//mt6771
//#define VIBR_MT6771_VOSEL_ADDR    MT6358_VIBR_ANA_CON0
//#define VIBR_MT6771_VOSEL_MASK    0xF
//#define VIBR_MT6771_VOSEL_SHIFT   8
//#define VIBR_MT6771_EN_ADDR       MT6358_LDO_VIBR_CON0
//#define VIBR_MT6771_EN_MASK       0x1
//#define VIBR_MT6771_EN_SHIFT      0

//mt6757,mt6797
#define VIBR_MT6351_VOSEL_ADDR    MT6351_VIBR_ANA_CON0
#define VIBR_MT6351_VOSEL_MASK    0x7
#define VIBR_MT6351_VOSEL_SHIFT   8
#define VIBR_MT6351_EN_ADDR       MT6351_LDO_VIBR_CON0
#define VIBR_MT6351_EN_MASK       0x1
#define VIBR_MT6351_EN_SHIFT      1

static struct workqueue_struct *vibrator_queue;
static struct work_struct vibrator_work;
static struct hrtimer vibe_timer;
static spinlock_t vibe_lock;
static int vibe_state;
static int ldo_state;
static int shutdown_flag;

#if 1
#define VIB_DEBUG(format, args...) \
do {	\
    pr_info(format, ##args); \
} while (0)
#else
#define VIB_DEBUG(fmt, args...) do {} while (0)
#endif

struct mtk_vibr_data {
    unsigned int volsel_addr;
    unsigned int volsel_mask;
    unsigned int volsel_shift;
    unsigned int en_addr;
    unsigned int en_mask;
    unsigned int en_shift;
};

struct vibrator_hw {
    u32 vib_timer;
    u32 vib_limit;
    u32 vib_vol;
    const struct mtk_vibr_data *data;
    struct regmap *regmap;
};

static const struct mtk_vibr_data mt6351_vibr_data = {
    .volsel_addr=VIBR_MT6351_VOSEL_ADDR,
    .volsel_mask=VIBR_MT6351_VOSEL_MASK,
    .volsel_shift=VIBR_MT6351_VOSEL_SHIFT,
    .en_addr=VIBR_MT6351_EN_ADDR,
    .en_mask=VIBR_MT6351_EN_MASK,
    .en_shift=VIBR_MT6351_EN_SHIFT,
};

//static const struct mtk_rtc_data mt6771_vibr_data = {
//    .volsel_addr = VIBR_MT6771_VOSEL_ADDR,
//    .volsel_mask = VIBR_MT6771_VOSEL_MASK,
//    .volsel_shif = VIBR_MT6771_VOSEL_SHIFT,
//    .en_addr = VIBR_MT6771_EN_ADDR,
//    .en_mask = VIBR_MT6771_EN_MASK,
//    .en_shift = VIBR_MT6771_EN_SHIFT,
//};

static const struct of_device_id vibr_of_ids[] = {
      { .compatible = "mediatek,mt6351-vibrator",
        .data = &mt6351_vibr_data, },
//      { .compatible = "mediatek,mt6771-vibrator",
//        .data = (void*)&mt6771_vibr_data, },
    {}
};
MODULE_DEVICE_TABLE(of, vibr_of_ids);

struct vibrator_hw *pvib_cust = NULL;

struct vibrator_hw *get_cust_vibrator_dtsi(void)
{
	if (pvib_cust == NULL)
		VIB_DEBUG("get_cust_vibrator_dtsi fail, pvib_cust is NULL\n");
	return pvib_cust;
}

void vibr_set_value(unsigned int value)
{
	struct vibrator_hw *hw = get_cust_vibrator_dtsi();
	VIB_DEBUG("[%s] regmap_update_bits(%p,%d,%d,%d)\n", __func__, hw->regmap, hw->data->en_addr,
              hw->data->en_mask << hw->data->en_shift, value << hw->data->en_shift);
	regmap_update_bits(hw->regmap, hw->data->en_addr,
		hw->data->en_mask << hw->data->en_shift, value << hw->data->en_shift);
}

//void init_vibr_oc_handler(void (*vibr_oc_func)(void))
//{
//	pmic_register_interrupt_callback(INT_VIBR_OC, vibr_oc_func);
//}

void init_cust_vibrator_dtsi(struct platform_device *pdev)
{
	int ret;
	const struct mtk_vibr_data *data;
	struct mt6397_chip *pmic;

	if (pvib_cust == NULL) {
		pvib_cust = kmalloc(sizeof(struct vibrator_hw), GFP_KERNEL);
		if (pvib_cust == NULL) {
			VIB_DEBUG("[%s] kmalloc fail\n", __func__);
			return;
		}
		ret = of_property_read_u32(pdev->dev.of_node, "vib_timer", &(pvib_cust->vib_timer));
		if (!ret)
			VIB_DEBUG("[%s] The vibrator timer from dts is : %d\n", __func__, pvib_cust->vib_timer);
		else
			pvib_cust->vib_timer = 25;

		ret = of_property_read_u32(pdev->dev.of_node, "vib_limit", &(pvib_cust->vib_limit));
		if (!ret)
			VIB_DEBUG("[%s] The vibrator limit from dts is : %d\n", __func__, pvib_cust->vib_limit);
		else
			pvib_cust->vib_limit = 9;

		ret = of_property_read_u32(pdev->dev.of_node, "vib_vol", &(pvib_cust->vib_vol));
		if (!ret)
			VIB_DEBUG("[%s] The vibrator vol from dts is : %d\n", __func__, pvib_cust->vib_vol);
		else
			pvib_cust->vib_vol = 0x05;

        data = (struct mtk_vibr_data*)of_device_get_match_data(&pdev->dev);
		pvib_cust->data = (struct mtk_vibr_data*)of_device_get_match_data(&pdev->dev);

		VIB_DEBUG("[%s] pvib_cust = %d, %d, %d\n", __func__, pvib_cust->vib_timer, pvib_cust->vib_limit, pvib_cust->vib_vol);

		pmic = platform_get_drvdata(pdev);
        if (!pmic)
          VIB_DEBUG("[%s] Failed to get pmic\n", __func__);
		pvib_cust->regmap = pmic->regmap;
		if (!pvib_cust->regmap)
			VIB_DEBUG("[%s] Failed to get regmap\n", __func__);

	}
}

void vibr_power_set(const struct platform_device *pdev)
{
	int error;
	struct vibrator_hw *hw = get_cust_vibrator_dtsi();

	if (hw != NULL) {
		VIB_DEBUG("[%s]: vibrator set voltage = %d\n", __func__, hw->vib_vol);
		error = regmap_update_bits(hw->regmap, hw->data->volsel_addr,
			hw->data->volsel_mask << hw->data->volsel_shift, (unsigned int) hw->vib_vol << hw->data->volsel_shift);
		if (error) {
			VIB_DEBUG("[%s]: regmap_update_bits failed (%d)\n", __func__, error);
		}
	} else {
		VIB_DEBUG("[%s]: can not get vibrator settings from dtsi!\n", __func__);
	}
}

static int vibr_enable(void)
{
    if (!ldo_state) {
        ldo_state = 1;
        vibr_set_value(ldo_state);
    }
    return 0;
}

static int vibr_disable(void)
{
    if (ldo_state) {
        ldo_state = 0;
        vibr_set_value(ldo_state);
    }
    return 0;
}

static void update_vibrator(struct work_struct *work)
{
    if (!vibe_state)
        vibr_disable();
    else
        vibr_enable();
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
    if (hrtimer_active(&vibe_timer)) {
        ktime_t r = hrtimer_get_remaining(&vibe_timer);

        return ktime_to_ms(r);
    } else
        return 0;
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{
    unsigned long flags;

    struct vibrator_hw *hw = get_cust_vibrator_dtsi();

	if (hrtimer_cancel(&vibe_timer))
		VIB_DEBUG("vibrator_enable: cancel existed hrtimer, cust timer: %d(ms), value: %d, shutdown: %d\n",
			hw->vib_timer, value, shutdown_flag);
	else
		VIB_DEBUG("vibrator_enable: no timer existed, cust timer: %d(ms), value: %d, shutdown: %d\n",
			hw->vib_timer, value, shutdown_flag);

    spin_lock_irqsave(&vibe_lock, flags);

    if (value == 0 || shutdown_flag == 1) {
        VIB_DEBUG("vibrator_enable: shutdown_flag = %d, cust_timer:%d\n",
              shutdown_flag, hw->vib_timer);
        vibe_state = 0;
    } else {
        VIB_DEBUG("vibrator_enable: vibrator cust timer: %d\n",
              hw->vib_timer);
        if (value > hw->vib_limit && value < hw->vib_timer)
            value = hw->vib_timer;

        value = (value > 15000 ? 15000 : value);
        vibe_state = 1;
        hrtimer_start(&vibe_timer,
                      ktime_set(value / 1000, (value % 1000) * 1000000),
                      HRTIMER_MODE_REL);
    }
    spin_unlock_irqrestore(&vibe_lock, flags);
    VIB_DEBUG("vibrator_enable: vibrator start: %d\n", value); 
    queue_work(vibrator_queue, &vibrator_work);
}

//static void vibrator_oc_handler(void)
//{
//	VIB_DEBUG("vibrator_oc_handler: disable vibr due to oc intr happened\n");
//	vibrator_enable(NULL, 0);
//}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	vibe_state = 0;
	VIB_DEBUG("vibrator_timer_func: vibrator will disable\n");
	queue_work(vibrator_queue, &vibrator_work);
	return HRTIMER_NORESTART;
}

static struct timed_output_dev mtk_vibrator = {
	.name = "vibrator",
	.get_time = vibrator_get_time,
	.enable = vibrator_enable,
};

static int vib_probe(struct platform_device *pdev)
{
//	init_vibr_oc_handler(vibrator_oc_handler);
	init_cust_vibrator_dtsi(pdev);
	vibr_power_set(pdev);
	return 0;
}

static int vib_remove(struct platform_device *pdev)
{
    return 0;
}

static void vib_shutdown(struct platform_device *pdev)
{
    unsigned long flags;

    VIB_DEBUG("vib_shutdown: enter!\n");
    spin_lock_irqsave(&vibe_lock, flags);
    shutdown_flag = 1;
    if (vibe_state) {
        VIB_DEBUG("vib_shutdown: vibrator will disable\n");
        vibe_state = 0;
        spin_unlock_irqrestore(&vibe_lock, flags);
        vibr_disable();
    } else {
        spin_unlock_irqrestore(&vibe_lock, flags);
    }
}

/******************************************************************************
* Device driver structure
*****************************************************************************/
static struct platform_driver vibrator_driver = {
	.probe = vib_probe,
	.remove = vib_remove,
	.shutdown = vib_shutdown,
	.driver = {
			.name = VIB_DEVICE,
			.owner = THIS_MODULE,
			.of_match_table = vibr_of_ids,
		   },
};

static struct platform_device vibrator_device = {
	.name = "mtk_vibrator",
	.id = -1,
};

static ssize_t store_vibr_on(struct device *dev, struct device_attribute *attr,
                             const char *buf, size_t size)
{
    if (buf != NULL && size != 0) {
        /* VIB_DEBUG("buf is %s and size is %d\n", buf, size); */
        if (buf[0] == '0')
            vibr_disable();
        else
            vibr_enable();
    }
    return size;
}
static DEVICE_ATTR(vibr_on, 0220, NULL, store_vibr_on);

static int vib_mod_init(void)
{
    s32 ret = 0;

    VIB_DEBUG("MediaTek MT6351 vibrator driver register, version %s\n",
              VERSION);

    ret = platform_device_register(&vibrator_device);
    if (ret != 0) {
        VIB_DEBUG("Unable to register vibrator device (%d)\n", ret);
        return ret;
    }

    vibrator_queue = create_singlethread_workqueue(VIB_DEVICE);
    if (!vibrator_queue) {
        VIB_DEBUG("Unable to create workqueue\n");
        return -ENODATA;
    }
    INIT_WORK(&vibrator_work, update_vibrator);

    spin_lock_init(&vibe_lock);
    shutdown_flag = 0;
    vibe_state = 0;
    hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    vibe_timer.function = vibrator_timer_func;

    timed_output_dev_register(&mtk_vibrator);/* timed_output driver model */

    ret = platform_driver_register(&vibrator_driver);

    if (ret) {
        VIB_DEBUG("Unable to register vibrator driver (%d)\n", ret);
        return ret;
    }

    ret = device_create_file(mtk_vibrator.dev, &dev_attr_vibr_on);
    if (ret)
        VIB_DEBUG("device_create_file vibr_on fail!\n");

    VIB_DEBUG("vib_mod_init Done\n");

    return ret;
}


static void vib_mod_exit(void)
{
    VIB_DEBUG("MediaTek MTK vibrator driver unregister, version %s\n", VERSION);
    if (vibrator_queue)
        destroy_workqueue(vibrator_queue);
    VIB_DEBUG("vib_mod_exit Done\n");
}

module_init(vib_mod_init);
module_exit(vib_mod_exit);


MODULE_AUTHOR("MediaTek Inc.");
MODULE_DESCRIPTION("MTK Vibrator Driver (VIB)");
MODULE_LICENSE("GPL v2");