/**************************************************************************
*  aw9120_led.c
* 
*  Create Date :
* 
*  Modify Date : 
*
*  Create by   : AWINIC Technology CO., LTD
*
*  Version     : 1.0 , 2016/11/22
* 
**************************************************************************/
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/gameport.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/atomic.h>

//////////////////////////////////////
// Defines
//////////////////////////////////////
#define AW9120_LED_NAME		"aw9120_led"

// Registers
#define     RSTR       	 0x00
#define     GCR      	 0x01

#define     LER1   	     0x50
#define     LER2   	     0x51
#define     LCR   	     0x52
#define     PMR 	     0x53
#define     RMR  	     0x54
#define     CTRS1  	     0x55
#define     CTRS2  	     0x56
#define     IMAX1    	 0x57
#define     IMAX2    	 0x58
#define     IMAX3    	 0x59
#define     IMAX4    	 0x5a
#define     IMAX5    	 0x5b
#define     TIER     	 0x5c
#define     INTVEC   	 0x5d
#define     LISR2     	 0x5e
#define     SADDR	     0x5f

#define     PCR      	 0x60
#define     CMDR     	 0x61
#define     RA       	 0x62
#define     RB       	 0x63
#define     RC       	 0x64
#define     RD       	 0x65
#define     R1       	 0x66
#define     R2       	 0x67
#define     R3       	 0x68
#define     R4       	 0x69
#define     R5       	 0x6a
#define     R6       	 0x6b
#define     R7       	 0x6c
#define     R8       	 0x6d
#define     GRPMSK1  	 0x6e
#define     GRPMSK2  	 0x6f

#define     TCR      	 0x70
#define     TAR      	 0x71
#define     TDR      	 0x72
#define     TDATA      	 0x73
#define     TANA     	 0x74
#define     TKST     	 0x75
#define     FEXT   	     0x76
#define     WP       	 0x7d
#define     WADDR    	 0x7e
#define     WDATA    	 0x7f

//////////////////////////////////////
// i2c client
//////////////////////////////////////
static struct i2c_client *aw9120_i2c_client;


/* The definition of each time described as shown in figure.
 *        /-----------\
 *       /      |      \
 *      /|      |      |\
 *     / |      |      | \-----------
 *       |hold_time_ms |      |
 *       |             |      |
 * rise_time_ms  fall_time_ms |
 *                       off_time_ms
 */
#define ROM_CODE_MAX 255

/*
 * rise_time_ms = 1500
 * hold_time_ms = 500
 * fall_time_ms = 1500
 * off_time_ms = 1500
 */
int led_code_len = 7;
int led_code[ROM_CODE_MAX] = {
	0xbf00,0x9f05,0xfffa,0x3c7d,0xdffa,0x3cbb,0x2,
};


//////////////////////////////////////////////////////////////////////////////////////////
// PDN power control
//////////////////////////////////////////////////////////////////////////////////////////
struct pinctrl *aw9120ctrl = NULL;
struct pinctrl_state *aw9120_pdn_high = NULL;
struct pinctrl_state *aw9120_pdn_low = NULL;

int aw9120_gpio_init(struct platform_device *pdev)
{
	int ret = 0;

	aw9120ctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(aw9120ctrl)) {
		dev_err(&pdev->dev, "Cannot find aw9120 pinctrl!");
		ret = PTR_ERR(aw9120ctrl);
		dev_err(&pdev->dev, "[%s] devm_pinctrl_get fail!\n", __func__);
	}
	aw9120_pdn_high = pinctrl_lookup_state(aw9120ctrl, "aw9120_pdn_high");
	if (IS_ERR(aw9120_pdn_high)) {
		ret = PTR_ERR(aw9120_pdn_high);
		dev_err(&pdev->dev, "[%s] : pinctrl err, aw9120_pdn_high\n", __func__);
	}

	aw9120_pdn_low = pinctrl_lookup_state(aw9120ctrl, "aw9120_pdn_low");
	if (IS_ERR(aw9120_pdn_low)) {
		ret = PTR_ERR(aw9120_pdn_low);
		dev_err(&pdev->dev, "[%s] : pinctrl err, aw9120_pdn_low\n", __func__);
	}

	dev_info(&pdev->dev, "[%s] success\n", __func__);
	return ret;
}

static void aw9120_hw_on(void)
{
	pr_err("[%s] enter\n", __func__);
	pinctrl_select_state(aw9120ctrl, aw9120_pdn_low);
	msleep(5);
	pinctrl_select_state(aw9120ctrl, aw9120_pdn_high);
	msleep(5);
	pr_err("[%s] out\n", __func__);
}

#if 0
static void aw9120_hw_off(void)
{
	pr_err("%s enter\n", __func__);
	pinctrl_select_state(aw9120ctrl, aw9120_pdn_low);
	msleep(5);
	pr_err("%s out\n", __func__);
}
#endif
//////////////////////////////////////////////////////////////////////////////////////////
// i2c write and read
//////////////////////////////////////////////////////////////////////////////////////////
static unsigned int i2c_write_reg(unsigned char addr, unsigned int reg_data)
{
	int ret;
	u8 wdbuf[512] = {0};

	struct i2c_msg msgs[] = {
		{
			.addr	= aw9120_i2c_client->addr,
			.flags	= 0,
			.len	= 3,
			.buf	= wdbuf,
		},
	};

	if(NULL == aw9120_i2c_client) {
		pr_err("msg %s aw9120_i2c_client is NULL\n", __func__);
		return -1;
	}

	wdbuf[0] = addr;
	wdbuf[2] = (unsigned char)(reg_data & 0x00ff);
	wdbuf[1] = (unsigned char)((reg_data & 0xff00)>>8);

	ret = i2c_transfer(aw9120_i2c_client->adapter, msgs, 1);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	return ret;
}


static unsigned int i2c_read_reg(unsigned char addr)
{
	int ret;
	u8 rdbuf[512] = {0};
	unsigned int getdata;

	struct i2c_msg msgs[] = {
		{
			.addr	= aw9120_i2c_client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= rdbuf,
		},
		{
			.addr	= aw9120_i2c_client->addr,
			.flags	= I2C_M_RD,
			.len	= 2,
			.buf	= rdbuf,
		},
	};

	if(NULL == aw9120_i2c_client) {
		pr_err("msg %s aw9120_i2c_client is NULL\n", __func__);
		return -1;
	}

	rdbuf[0] = addr;

	ret = i2c_transfer(aw9120_i2c_client->adapter, msgs, 2);
	if (ret < 0)
		pr_err("msg %s i2c read error: %d\n", __func__, ret);

	getdata=rdbuf[0] & 0x00ff;
	getdata<<= 8;
	getdata |=rdbuf[1];

	return getdata;
}


//////////////////////////////////////////////////////////////////////////////////////////
// aw9120 led 
//////////////////////////////////////////////////////////////////////////////////////////
unsigned int aw9210_disable_led_module(void)
{
	unsigned int reg;
	reg = i2c_read_reg(GCR);
	reg &= 0xFFFE;
	i2c_write_reg(GCR, reg);
	return reg;
}

void aw9210_enable_led_module(unsigned int reg) {
	reg |= 0x0001;
	i2c_write_reg(GCR,reg);
}

void aw9120_led_off(void)
{
	aw9210_disable_led_module();
}

void aw9120_led_on(void)
{
	unsigned int reg = aw9210_disable_led_module();

	//LED Config
	i2c_write_reg(IMAX1,0x1111); 	// IMAX1-LED1~LED4 Current
	i2c_write_reg(IMAX2,0x1111); 	// IMAX2-LED5~LED8 Current
	i2c_write_reg(IMAX3,0x1111); 	// IMAX3-LED9~LED12 Current
	i2c_write_reg(IMAX4,0x1111); 	// IMAX4-LED13~LED16 Current
	i2c_write_reg(IMAX5,0x1111); 	// IMAX5-LED17~LED20 Current
	i2c_write_reg(LER1,0x0FFF); 	// LER1-LED1~LED12 Enable
	i2c_write_reg(LER2,0x00FF); 	// LER2-LED13~LED20 Enable
	i2c_write_reg(CTRS1,0x0FFF); 	// CTRS1-LED1~LED12: i2c Control
	i2c_write_reg(CTRS2,0x00FF); 	// CTRS2-LED13~LED20: i2c Control
	//i2c_write_reg(CMDR,0xBFFF); 	// CMDR-LED1~LED20 PWM=0xFF

	aw9210_enable_led_module(reg);

	i2c_write_reg(CMDR,0xBFFF); 	// CMDR-LED1~LED20 PWM=0xFF
}

void aw9120_led_breath(void)
{
	int i;

	unsigned int reg = aw9210_disable_led_module();

	//LED Config
	i2c_write_reg(IMAX1,0x1111); 	// IMAX1-LED1~LED4 Current
	i2c_write_reg(IMAX2,0x1111); 	// IMAX2-LED5~LED8 Current
	i2c_write_reg(IMAX3,0x1111); 	// IMAX3-LED9~LED12 Current
	i2c_write_reg(IMAX4,0x1111); 	// IMAX4-LED13~LED16 Current
	i2c_write_reg(IMAX5,0x1111); 	// IMAX5-LED17~LED20 Current
	i2c_write_reg(LER1,0x0FFF); 	// LER1-LED1~LED12 Enable
	i2c_write_reg(LER2,0x00FF); 	// LER2-LED13~LED20 Enable
	i2c_write_reg(CTRS1,0x0000); 	// CTRS1-LED1~LED12: SRAM Control
	i2c_write_reg(CTRS2,0x0000); 	// CTRS2-LED13~LED20: SRAM Control

	aw9210_enable_led_module(reg);

	// LED SRAM Hold Mode
	i2c_write_reg(PMR,0x0000);		// PMR-Load SRAM with I2C
	i2c_write_reg(RMR,0x0000);		// RMR-Hold Mode

	// Load LED SRAM
	i2c_write_reg(WADDR,0x0000);	// WADDR-SRAM Load Addr
	for(i=0; i<led_code_len; i++)
	{
		i2c_write_reg(WDATA,led_code[i]);
	}

	// LED SRAM Run
	i2c_write_reg(SADDR,0x0000);	// SADDR-SRAM Run Start Addr:0
	i2c_write_reg(PMR,0x0001);		// PMR-Reload and Excute SRAM
	i2c_write_reg(RMR,0x0002);		// RMR-Run
}

static int aw9120_led_id_from_name(unsigned int led_name)
{
	if (led_name == 2) {
		return 4;
	} else if (led_name > 2 && led_name < 6) {
		return led_name - 2;
	} else if (led_name > 0 && led_name < 8) {
		return led_name - 1;
	}

	pr_err("[aw9120] invalid LED %d\n", led_name);
	return -1;
}

// set the maximum current to run a given RGB LED
static void aw9120_set_current_rgb(int led_id, u8 rgb[3])
{
	u16 LER1reg, LER2reg, CTRS1reg, CTRS2reg;

	u16 low_mask, high_mask;
	u32 value_bits;
	u16 low_bits, high_bits;
	u32 enable_nmask, enable_bits;

	u16 old_regval;

	unsigned int red_subled_offset = led_id * 3; // 3 subLEDs per LED
	unsigned int low_regidx = red_subled_offset / 4;
	unsigned int pos_in_low = red_subled_offset % 4;

	unsigned int reg = aw9210_disable_led_module();

	LER1reg = i2c_read_reg(LER1);
	LER2reg = i2c_read_reg(LER2);
	CTRS1reg = i2c_read_reg(CTRS1);
	CTRS2reg = i2c_read_reg(CTRS2);

	// `red_subled_offset` indicates which nibble in the IMAX registers
	// the first (R) of the three (R, G, and B) LEDs is

	low_mask = ~GENMASK(pos_in_low * 4 + 11, pos_in_low * 4);
	high_mask = ~GENMASK(pos_in_low * 4 - 5, 0);

	//combine rgb into bits
	value_bits = (rgb[2] << 8 | rgb[1] << 4 | rgb[0]) << (pos_in_low * 4);
	low_bits = value_bits & 0xffff;
	high_bits = value_bits >> 16;

	enable_nmask = GENMASK(red_subled_offset+2, red_subled_offset);
	enable_bits = (u32)(((rgb[2] != 0) << 2) | ((rgb[1] != 0) << 1) | (rgb[0] != 0))
		<< red_subled_offset;
	LER1reg = (LER1reg & ~enable_nmask) | (enable_bits & 0x0fff);
	LER2reg = (LER2reg & ~(enable_nmask >> 12)) | (enable_bits >> 12);

	CTRS1reg = (CTRS1reg & ~enable_nmask) | (enable_bits & 0x0fff);
	CTRS2reg = (CTRS2reg & ~(enable_nmask >> 12)) | (enable_bits >> 12);

	old_regval = i2c_read_reg(IMAX1+low_regidx);
	i2c_write_reg(IMAX1+low_regidx, (old_regval & low_mask) | low_bits);

	if (high_bits != 0)
	{
		old_regval = i2c_read_reg(IMAX1+low_regidx+1);
		i2c_write_reg(IMAX1+low_regidx+1, (old_regval & high_mask) | high_bits);
	}

	// enable LEDs
	i2c_write_reg(LER1, LER1reg);
	i2c_write_reg(LER2, LER2reg);
	i2c_write_reg(CTRS1, CTRS1reg);
	i2c_write_reg(CTRS2, CTRS2reg);

	aw9210_enable_led_module(reg);

	i2c_write_reg(CMDR, 0xbfff);
}

/*************************************proc start***********************************************/
static ssize_t aw9120_value_proc_write(struct file *filp, const char __user *buff, size_t count, loff_t *ppos)
{
	char str_buf[16] = {0};

	if (copy_from_user(str_buf, buff, count) ){
		pr_err("copy_from_user---error\n");
		return -EFAULT;
	}

	if(str_buf[0]== '0') {// OFF
		aw9120_led_off();
	} else if(str_buf[0]== '1') {// ON
		aw9120_led_on();
	} else if(str_buf[0]== '2') {// Breath
		aw9120_led_breath();
	} else {
		aw9120_led_off();
	}

	return count;
}

static const struct proc_ops aw9120_value_proc_fops = {
	.proc_write = aw9120_value_proc_write,
};

static ssize_t aw9120_reg_proc_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	unsigned int reg_val[1];
	ssize_t len = 0;
	unsigned char i;
	for(i=1;i<0x7F;i++) {
		reg_val[0] = i2c_read_reg(i);
		len += snprintf(buffer+len, PAGE_SIZE-len, "reg%2X = 0x%4X, ", i,reg_val[0]);
	}
	len += snprintf(buffer+len, PAGE_SIZE-len, "\n");
	return len;
}

static ssize_t aw9120_reg_proc_write(struct file *filp, const char __user *buff, size_t count, loff_t *ppos)
{
	unsigned int databuf[2];
	if(2 == sscanf(buff,"%x %x",&databuf[0], &databuf[1])) {
		i2c_write_reg((u8)databuf[0],databuf[1]);
	}
	return count;
}

static const struct proc_ops aw9120_reg_proc_fops = {
	.proc_read = aw9120_reg_proc_read,
	.proc_write = aw9120_reg_proc_write,
};

static ssize_t aw9120_operation_proc_write(struct file *filp, const char __user *buff, size_t count, loff_t *ppos)
{
	unsigned int led_name;
	u8 rgb[3];
	int nread = 0;
	if(4 == sscanf(buff,"%x %hhx %hhx %hhx%n", &led_name, &rgb[0], &rgb[1], &rgb[2], &nread)) {
		int led_id = aw9120_led_id_from_name(led_name);
		if(led_id < 0)
			return -EINVAL;

		aw9120_set_current_rgb(led_id, rgb);
	}
	return count;
}

static const struct proc_ops aw9120_operation_proc_fops = {
	.proc_write = aw9120_operation_proc_write,
};

/*************************proc end*********************************/

//////////////////////////////////////////////////////////////////////////////////////////
// i2c driver
//////////////////////////////////////////////////////////////////////////////////////////
static int aw9120_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err = 0;
	int count = 0;
	unsigned int reg_value;

	pr_err("[%s] Enter\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	aw9120_i2c_client = client;
	pr_err("[%s]: i2c addr=%x\n", __func__, client->addr);

	aw9120_hw_on();

	for(count = 0; count < 5; count++) {
		reg_value = i2c_read_reg(0x00); //read chip ID
		pr_err("%s: aw9120 chip ID = 0x%4x\n", __func__, reg_value);
		if (reg_value == 0xb223)
			break;
		msleep(5);
		if(count == 4) {
			pr_err("msg %s read id error\n", __func__);
			err = -ENODEV;
			goto exit_create_singlethread;
		}
	}

	proc_create("aw9120_value", 0777, NULL, &aw9120_value_proc_fops);
	proc_create("aw9120_reg", 0777, NULL, &aw9120_reg_proc_fops);
	proc_create("aw9120_operation", 0777, NULL, &aw9120_operation_proc_fops);

	return 0;

exit_create_singlethread:
	pr_err("==singlethread error =\n");
	i2c_set_clientdata(client, NULL);
	aw9120_i2c_client = NULL;
exit_check_functionality_failed:
	return err;
}

static int aw9120_i2c_remove(struct i2c_client *client)
{
	pr_err("%s enter\n", __func__);

	i2c_set_clientdata(client, NULL);
	aw9120_i2c_client = NULL;

	return 0;
}

static const struct i2c_device_id aw9120_i2c_id[] = {
	{ AW9120_LED_NAME, 0 },{ }
};
MODULE_DEVICE_TABLE(i2c, aw9120_ts_id);


static const struct of_device_id aw9120_i2c_of_match[] = {
	{.compatible = "mediatek,aw9120_led"},
	{},
};
static struct i2c_driver aw9120_i2c_driver = {
	.probe		= aw9120_i2c_probe,
	.remove		= aw9120_i2c_remove,
	.id_table	= aw9120_i2c_id,
	.driver	= {
		.name	= AW9120_LED_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = aw9120_i2c_of_match,
	},
};

//////////////////////////////////////////////////////////////////////////////////////////
// platform driver
//////////////////////////////////////////////////////////////////////////////////////////
static int aw9120_led_remove(struct platform_device *pdev)
{
	pr_err("[%s] start!\n", __func__);
	i2c_del_driver(&aw9120_i2c_driver);
	return 0;
}

static int aw9120_led_probe(struct platform_device *pdev)
{
	int ret;

	dev_err(&pdev->dev, "[%s] start!\n", __func__);
	
	ret = aw9120_gpio_init(pdev);
	if (ret != 0) {
		dev_err(&pdev->dev, "[%s] failed to init aw9120 pinctrl.\n", __func__);
		return ret;
	} else {
		dev_err(&pdev->dev, "[%s] Success to init aw9120 pinctrl.\n", __func__);
	}
	
	ret = i2c_add_driver(&aw9120_i2c_driver);
	if (ret != 0) {
		dev_err(&pdev->dev, "[%s] failed to register aw9120 i2c driver.\n", __func__);
		return ret;
	} else {
		dev_err(&pdev->dev, "[%s] Success to register aw9120 i2c driver.\n", __func__);
	}

	dev_err(&pdev->dev, "[%s] exit!\n", __func__);
	
	return 0;
}

static const struct of_device_id aw9120_led_of_match[] = {
	{.compatible = "mediatek,aw9120_led"},
	{},
};

static struct platform_driver aw9120_led_driver = {
	.probe	 = aw9120_led_probe,
	.remove	 = aw9120_led_remove,
	.driver = {
		.name   = "aw9120_led",
		.of_match_table = aw9120_led_of_match,
	}
};

static int __init aw9120_led_init(void)
{
	int ret;
	pr_err("[%s] Enter\n", __func__);

	ret = platform_driver_register(&aw9120_led_driver);
	if (ret) {
		pr_err("****[%s] Unable to register driver (%d)\n", __func__, ret);
		return ret;
	}

	pr_err("%s Exit\n", __func__);

	return ret;
}

static void __exit aw9120_led_exit(void)
{
	pr_err("[%s] Enter\n", __func__);
	platform_driver_unregister(&aw9120_led_driver);
	pr_err("[%s] Exit\n", __func__);
}

module_init(aw9120_led_init);
module_exit(aw9120_led_exit);

MODULE_AUTHOR("<liweilei@awinic.com.cn>");
MODULE_DESCRIPTION("AWINIC AW9120 LED Driver");
MODULE_LICENSE("GPL v2");
