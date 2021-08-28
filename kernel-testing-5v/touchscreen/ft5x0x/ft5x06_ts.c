/* drivers/input/touchscreen/ft5x06_ts.c
 *
 * FocalTech ft5x0x TouchScreen driver.
 *
 * Copyright (c) 2010  Focal tech Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/input.h>
#include "earlysuspend.h"
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
//#include <mach/irqs.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/input/mt.h>
#include "intel_vlv2.h"
#include "ft5x06_ts.h"
#include <linux/timer.h>

#include "gpio.h"
#include "acpi.h"
#include "acpi_gpio.h"

/*
#include <mach/gpio.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
*/
//#define FT5336_DOWNLOAD
#define SYSFS_DEBUG
#define FTS_APK_DEBUG
#define FTS_CTL_IIC  

#ifdef FTS_CTL_IIC
#include "focaltech_ctl.h"
#endif

#ifdef FT5336_DOWNLOAD
#include "ft5336_download_lib.h"
static struct i2c_client *g_i2c_client = NULL;
static unsigned char CTPM_MAIN_FW[]=
{
	/* #include "ft5336_all.i" */
};
#endif

#ifdef SYSFS_DEBUG
#include "ft5x06_ex_fun.h"
#endif

//extern int s3c_gpio_cfgpin(unsigned int pin, unsigned int config);
struct ts_event {
	u16 au16_x[CFG_MAX_TOUCH_POINTS];	/*x coordinate */
	u16 au16_y[CFG_MAX_TOUCH_POINTS];	/*y coordinate */
	u8 au8_touch_event[CFG_MAX_TOUCH_POINTS];	/*touch event:
					0 -- down; 1-- contact; 2 -- contact */
	u8 au8_finger_id[CFG_MAX_TOUCH_POINTS];	/*touch ID */
	u16 pressure;
	u8 touch_point;
};

struct ft5x0x_ts_data {
	unsigned int irq;
	unsigned int x_max;
	unsigned int y_max;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct ts_event event;
	struct ft5x0x_platform_data *pdata;
	#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
	#endif
};
static struct ft5x0x_ts_data *ft5x0x_ts;
static struct ft5x0x_ts_data ft5x0x_data ;
static struct ft5x0x_platform_data ft5x0x_platform;
//extern int fts_ctpm_auto_upgrade(struct i2c_client * client);
static void ft5x0x_ts_release(struct ft5x0x_ts_data *data)
{
        int i = 0;
        for(i=0; i<5; i++) 
	{
    	    input_mt_slot(data->input_dev,i);
            input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,false);
	
        }
        input_report_key(data->input_dev, BTN_TOUCH, 0);
        input_sync(data->input_dev);
}


#define ANDROID_INPUT_PROTOCOL_B
/*change*/
#define FT5X0X_RESET_PIN        FT_RST_PORT /*S5PV210_GPB(2)*/
#define FT5X0X_RESET_PIN_NAME	"ft5x0x-reset"
#define FT5X0X_INT_PIN        FT_INT_PORT

#ifdef CONFIG_PM
static void ft5x0x_ts_suspend(struct early_suspend *h);
static void ft5x0x_ts_resume(struct early_suspend *handle);
#endif

/*
*ft5x0x_i2c_Read-read data and write data by i2c
*@client: handle of i2c
*@writebuf: Data that will be written to the slave
*@writelen: How many bytes to write
*@readbuf: Where to store data read from slave
*@readlen: How many bytes to read
*
*Returns negative errno, else the number of messages executed
*
*
*/
int ft5x0x_i2c_Read(struct i2c_client *client, char *writebuf,
		    int writelen, char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&client->dev, "f%s: i2c read error=%d.\n",
				__func__,ret);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&client->dev, "%s:i2c read error=%d.\n", __func__,ret);
	}
	return ret;
}
/*write data by i2c*/
 int ft5x0x_i2c_Write(struct i2c_client *client, char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = client->addr,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&client->dev, "%s i2c write error.\n", __func__);

	return ret;
}
#ifdef FT5336_DOWNLOAD
int ft5x0x_download_i2c_Read(unsigned char *writebuf,
		    int writelen, unsigned char *readbuf, int readlen)
{
	int ret;

	if (writelen > 0) {
		struct i2c_msg msgs[] = {
			{
			 .addr = 0x38,
			 .flags = 0,
			 .len = writelen,
			 .buf = writebuf,
			 },
			{
			 .addr = 0x38,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(g_i2c_client->adapter, msgs, 2);
		if (ret < 0)
			dev_err(&g_i2c_client->dev, "f%s: i2c read error.\n",
				__func__);
	} else {
		struct i2c_msg msgs[] = {
			{
			 .addr = 0x38,
			 .flags = I2C_M_RD,
			 .len = readlen,
			 .buf = readbuf,
			 },
		};
		ret = i2c_transfer(g_i2c_client->adapter, msgs, 1);
		if (ret < 0)
			dev_err(&g_i2c_client->dev, "%s:i2c read error.\n", __func__);
	}
	return ret;
}
/*write data by i2c*/
int ft5x0x_download_i2c_Write(unsigned char *writebuf, int writelen)
{
	int ret;

	struct i2c_msg msg[] = {
		{
		 .addr = 0x38,
		 .flags = 0,
		 .len = writelen,
		 .buf = writebuf,
		 },
	};

	ret = i2c_transfer(g_i2c_client->adapter, msg, 1);
	if (ret < 0)
		dev_err(&g_i2c_client->dev, "%s i2c write error.\n", __func__);

	return ret;
}

#endif
/*Read touch point information when the interrupt  is asserted.*/
static int ft5x0x_read_Touchdata(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	u8 buf[POINT_READ_BUF] = { 0 };
	int ret = -1;
	int i = 0;
	u8 pointid = FT_MAX_ID;

	ret = ft5x0x_i2c_Read(data->client, buf, 1, buf, POINT_READ_BUF);
	if (ret < 0) {
		dev_err(&data->client->dev, "%s read touchdata failed.\n",
			__func__);
		return ret;
	}
	memset(event, 0, sizeof(struct ts_event));

	event->touch_point = 0;
	for (i = 0; i < CFG_MAX_TOUCH_POINTS; i++) {
		pointid = (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		if (pointid >= FT_MAX_ID)
			break;
		else
			event->touch_point++;
			event->au16_x[i] =
		    (s16) (buf[FT_TOUCH_X_H_POS + FT_TOUCH_STEP * i] & 0x0F) << 8 | (s16) buf[FT_TOUCH_X_L_POS + FT_TOUCH_STEP * i];
	//event->au16_x[i]= data->x_max - (event->au16_x[i]);
		event->au16_y[i] =
		    (s16) (buf[FT_TOUCH_Y_H_POS + FT_TOUCH_STEP * i] & 0x0F) << 8 | (s16) buf[FT_TOUCH_Y_L_POS + FT_TOUCH_STEP * i];
	//event->au16_y[i]= data->y_max - (event->au16_y[i]);	
	event->au8_touch_event[i] =
		buf[FT_TOUCH_EVENT_POS + FT_TOUCH_STEP * i] >> 6;
		event->au8_finger_id[i] =
		    (buf[FT_TOUCH_ID_POS + FT_TOUCH_STEP * i]) >> 4;
		#if 0
		pr_info("id=%d event=%d x=%d y=%d\n", event->au8_finger_id[i],
			event->au8_touch_event[i], event->au16_x[i], event->au16_y[i]);
		#endif
	}

	event->pressure = FT_PRESS;	

	return 0;
}

/*
*report the point information
*/
static void ft5x0x_report_value(struct ft5x0x_ts_data *data)
{
	struct ts_event *event = &data->event;
	int i;
	int uppoint = 0;

	/*protocol B*/	
	for (i = 0; i < event->touch_point; i++)
	{
		input_mt_slot(data->input_dev, event->au8_finger_id[i]);
		
		if (event->au8_touch_event[i]== 0 || event->au8_touch_event[i] == 2)
		{
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
				true);
			input_report_abs(data->input_dev, ABS_MT_TOUCH_MAJOR,
					event->pressure);
			input_report_abs(data->input_dev, ABS_MT_POSITION_X,
					event->au16_x[i]);
			input_report_abs(data->input_dev, ABS_MT_POSITION_Y,
					event->au16_y[i]);
		  //printk("Report:  x=%d , y=%d\n",event->au16_x[i],event->au16_y[i]);
		}
		else
		{
			uppoint++;
			input_mt_slot(data->input_dev,event->au8_finger_id[i]);
			input_mt_report_slot_state(data->input_dev, MT_TOOL_FINGER,
				false);
		}
	}
	if(event->touch_point == uppoint)
		input_report_key(data->input_dev, BTN_TOUCH, 0);
	else
		input_report_key(data->input_dev, BTN_TOUCH, event->touch_point > 0);
	input_sync(data->input_dev);


}

/*The ft5x0x device will signal the host about TRIGGER_FALLING.
*Processed when the interrupt is asserted.
*/
static irqreturn_t ft5x0x_ts_interrupt(int irq, void *dev_id)
{
	ft5x0x_ts =  dev_id;
	int ret = 0;
	disable_irq_nosync(ft5x0x_ts->irq);

	//pr_info("interrupt is coming\n");
	ret = ft5x0x_read_Touchdata(ft5x0x_ts);
	if (ret == 0)
		ft5x0x_report_value(ft5x0x_ts);

	enable_irq(ft5x0x_ts->irq);

	return IRQ_HANDLED;
}

void ft5x0x_reset_tp(int HighOrLow)
{
	pr_info("set tp reset pin to %d\n", HighOrLow);
//	gpio_set_value(FT5X0X_RESET_PIN, HighOrLow);
	gpio_set_value(ft5x0x_data.pdata->reset, HighOrLow);
}

void ft5x0x_Enable_IRQ(struct i2c_client *client, int enable)
{
	//if (FT5X0X_ENABLE_IRQ == enable)
	//if (FT5X0X_ENABLE_IRQ)
		//enable_irq(client->irq);
	//else
		//disable_irq_nosync(client->irq);
}

static int fts_init_gpio_hw(struct ft5x0x_ts_data *ft5x0x_ts)
{

	int ret = 0;
/*	int i = 0;     */
	
//	ret = gpio_request(FT5X0X_RESET_PIN, FT5X0X_RESET_PIN_NAME);
	ret = gpio_request(ft5x0x_ts->pdata->reset, FT5X0X_RESET_PIN_NAME);
	if (ret) {
		pr_err("%s: request GPIO %s for reset failed, ret = %d\n",
				__func__, FT5X0X_RESET_PIN_NAME, ret);
		return ret;
	}
        /*change*/
//        gpio_direction_output(FT5X0X_RESET_PIN,0);
		gpio_direction_output(ft5x0x_ts->pdata->reset,0);

//	gpio_set_value(FT5X0X_RESET_PIN, 1);
	gpio_set_value(ft5x0x_ts->pdata->reset,1);

	return ret;
}

static void fts_un_init_gpio_hw(struct ft5x0x_ts_data *ft5x0x_ts)
{
	//gpio_free(FT5X0X_RESET_PIN);
	gpio_free(ft5x0x_ts->pdata->reset);
}
#ifdef FT5336_DOWNLOAD

int ft5336_Enter_Debug(void)
{
	ft5x0x_reset_tp(0);
	msleep(4);
	ft5x0x_reset_tp(1);
	return ft5336_Lib_Enter_Download_Mode();
}
//if return 0, main flash is ok, else download.
int ft5336_IsDownloadMain(void)
{
	//add condition to check main flash
	return -1;
}
int ft5336_DownloadMain(void)
{
	unsigned short fwlen = 0;
	if (ft5336_Enter_Debug() < 0) {
		pr_err("-----enter debug mode failed\n");
		return -1;
	}
	fwlen = sizeof(CTPM_MAIN_FW);
	pr_info("----fwlen=%d\n", fwlen);

	//return ft6x06_Lib_DownloadMain(CTPM_MAIN_FW, fwlen);
	return ft5336_Lib_DownloadMain(CTPM_MAIN_FW, fwlen);
}
#endif


static int ft5x0x_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct ft5x0x_platform_data *pdata=&ft5x0x_platform;
//	    (struct ft5x0x_platform_data *)client->dev.platform_data;
	struct ft5x0x_ts_data *ft5x0x_ts;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char uc_reg_value;
	unsigned char uc_reg_addr;
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		goto exit_check_functionality_failed;
	}

	ft5x0x_ts = kzalloc(sizeof(struct ft5x0x_ts_data), GFP_KERNEL);

	if (!ft5x0x_ts) {
		err = -ENOMEM;
		goto exit_alloc_data_failed;
	}

	i2c_set_clientdata(client, ft5x0x_ts);

	uc_reg_addr = 0xa8;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);

	if((uc_reg_value != 0xa0) && (uc_reg_value != 0x59)) {
		printk("lee: wrong touch: 0x%x!\n", uc_reg_value);
		kfree(ft5x0x_ts);
		return -ENOMEM;
	}

	pdata->reset = acpi_get_gpio_by_index(&client->dev, 0, NULL);
	pdata->irq =acpi_get_gpio_by_index(&client->dev, 1, NULL);
	//printk("pdata->reset = %d\n", pdata->reset);
//	printk("pdata->irq= %d\n", pdata->irq);

	/*get irq*/
	client->irq = gpio_to_irq(pdata->irq);

	/*change*/
//	ft5x0x_ts->irq = IRQ_EINT(14);//client->irq;	
        ft5x0x_ts->irq = client->irq;
	ft5x0x_ts->client = client;
	ft5x0x_ts->pdata = pdata;
	ft5x0x_ts->x_max = 600-1;//pdata->x_max - 1;
	ft5x0x_ts->y_max = 1024-1;//pdata->y_max - 1;
//	ft5x0x_ts->pdata->reset = 128; //acpi_get_gpio_by_index(&client->dev, 0, NULL);
//	ft5x0x_ts->pdata->irq =133; //acpi_get_gpio_by_index(&client->dev, 0, NULL);
    //    i2c_set_clientdata(client, ft5x0x_ts);
    
	ft5x0x_data=*ft5x0x_ts;
//	client->dev.platform_data = pdata;

	client->irq = ft5x0x_ts->irq;
//	printk("ft_irq = %d\n", client->irq);
	//pr_info("FT5X0X_INT_PIN = %d\n", FT5X0X_INT_PIN);
	gpio_free(ft5x0x_ts->pdata->reset);
        gpio_free(ft5x0x_ts->pdata->irq);


	if(fts_init_gpio_hw(ft5x0x_ts)<0)
		goto exit_init_gpio;	
#ifdef CONFIG_PM
#if 0
	err = gpio_request(pdata->reset, "ft5x0x reset");
	if (err < 0) {
		dev_err(&client->dev, "%s:failed to set gpio reset.\n",
			__func__);
		goto exit_request_reset;
	}
#endif
#endif
	input_dev = input_allocate_device();
	if (!input_dev) {
		err = -ENOMEM;
		dev_err(&client->dev, "failed to allocate input device\n");
		goto exit_input_dev_alloc_failed;
	}

	ft5x0x_ts->input_dev = input_dev;
#if 0
        set_bit(ABS_MT_WIDTH_MAJOR, input_dev->absbit);	
	set_bit(ABS_MT_TOUCH_MAJOR, input_dev->absbit);
        set_bit(ABS_MT_POSITION_X, input_dev->absbit);
        set_bit(ABS_MT_POSITION_Y, input_dev->absbit);
        set_bit(ABS_MT_PRESSURE, input_dev->absbit);
        set_bit(EV_KEY, input_dev->evbit);
        set_bit(EV_ABS, input_dev->evbit);
         __set_bit(BTN_TOUCH, input_dev->keybit);


        __set_bit(INPUT_PROP_DIRECT, ft5x0x_ts->input_dev->propbit);
        input_mt_init_slots(input_dev, CFG_MAX_TOUCH_POINTS,0);

        input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
        input_set_abs_params(input_dev, ABS_MT_POSITION_Y, 0, ft5x0x_ts->y_max, 0, 0);
        input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
        input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, 255, 0, 0);
//        input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
        input_set_abs_params(input_dev, ABS_MT_PRESSURE, 0, PRESS_MAX, 0, 0);
  //      input_set_abs_params(input_dev, ABS_MT_TRACKING_ID, 0, CFG_MAX_TOUCH_POINTS, 0, 0);


        pr_info("Ft5x06: x_max=%d y_max=%d\n", ft5x0x_ts->x_max, ft5x0x_ts->y_max);
#endif
#if 1
	__set_bit(EV_ABS, input_dev->evbit);
	__set_bit(EV_KEY, input_dev->evbit);
	__set_bit(BTN_TOUCH, input_dev->keybit);
	/*change*/

	__set_bit(INPUT_PROP_DIRECT, input_dev->propbit);
	input_mt_init_slots(input_dev, CFG_MAX_TOUCH_POINTS,0);
	input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, PRESS_MAX, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_X, 0, ft5x0x_ts->x_max, 0, 0);
	input_set_abs_params(input_dev, ABS_MT_POSITION_Y,0, ft5x0x_ts->y_max, 0, 0);
	
	//pr_info("Ft5x06: x_max=%d y_max=%d\n", ft5x0x_ts->x_max, ft5x0x_ts->y_max);

	#endif
	input_dev->name = FT5X0X_NAME;
	err = input_register_device(input_dev);
	if (err) {
		dev_err(&client->dev,
			"ft5x0x_ts_probe: failed to register input device: %s\n",
			dev_name(&client->dev));
		goto exit_input_register_device_failed;
	}
	
	/*make sure CTP already finish startup process */
	msleep(150);
#ifdef SYSFS_DEBUG
		ft5x0x_create_sysfs(client);
#endif

#ifdef FTS_CTL_IIC
		if (ft_rw_iic_drv_init(client) < 0)
			dev_err(&client->dev, "%s:[FTS] create fts control iic driver failed\n",
					__func__);
#endif
#ifdef FTS_APK_DEBUG
	ft5x0x_create_apk_debug_channel(client);
#endif

#ifdef FT5336_DOWNLOAD
			g_i2c_client = client;
			FTS_I2c_Read_Function fun_i2c_read = ft5x0x_download_i2c_Read;
			FTS_I2c_Write_Function fun_i2c_write = ft5x0x_download_i2c_Write;
			Init_I2C_Read_Func(fun_i2c_read);
			Init_I2C_Write_Func(fun_i2c_write);
			 if(ft5336_IsDownloadMain() < 0) {
	 	#if 1
				pr_info("--------FTS---------download main\n");
				if(ft5336_DownloadMain()<0)
				{
					pr_err("---------FTS---------Download main failed\n");
				}
		#endif
			 } else
				pr_info("--------FTS---------no download main\n");
#endif
       // fts_ctpm_auto_upgrade(client);
        uc_reg_value = 0;

	/*get some register information */
	uc_reg_addr = FT5x0x_REG_FW_VER;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	//dev_dbg(&client->dev, "[FTS] Firmware version = 0x%x\n", uc_reg_value);
//	pr_info( "[FTS] Firmware version = 0x%x\n", uc_reg_value);

	uc_reg_addr = FT5x0x_REG_POINT_RATE;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	//dev_dbg(&client->dev, "[FTS] report rate is %dHz.\n",
	//	uc_reg_value * 10);
//	pr_info("[FTS] report rate is %dHz.\n", uc_reg_value * 10);

	uc_reg_addr = FT5X0X_REG_THGROUP;
	ft5x0x_i2c_Read(client, &uc_reg_addr, 1, &uc_reg_value, 1);
	//dev_dbg(&client->dev, "[FTS] touch threshold is %d.\n",
	//	uc_reg_value * 4);
//	pr_info("[FTS] touch threshold is %d.\n", uc_reg_value * 4);

	err = request_threaded_irq(client->irq, NULL, ft5x0x_ts_interrupt, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, client->dev.driver->name, ft5x0x_ts);
	if (err < 0) {
	//	dev_err(&client->dev, "ft5x0x_probe: request irq failed\n");
		goto exit_irq_request_failed;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ft5x0x_data.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ft5x0x_data.early_suspend.suspend = ft5x0x_ts_suspend;
	ft5x0x_data.early_suspend.resume = ft5x0x_ts_resume;
	register_early_suspend(&ft5x0x_data.early_suspend);
#endif
       // printk("**********end_ft5x0x_probe************\n");
	return 0;

exit_irq_request_failed:
	i2c_set_clientdata(client, NULL);
	free_irq(client->irq, ft5x0x_ts);

exit_input_register_device_failed:
	input_free_device(input_dev);

exit_input_dev_alloc_failed:
#ifdef CONFIG_PM
#if 0
exit_request_reset:
	gpio_free(ft5x0x_ts->pdata->reset);
#endif
#endif

exit_init_gpio:
	fts_un_init_gpio_hw(ft5x0x_ts);
	kfree(ft5x0x_ts);

exit_alloc_data_failed:
exit_check_functionality_failed:
	return err;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void ft5x0x_ts_suspend(struct early_suspend *h)
{
         /*change*/
	struct ft5x0x_ts_data *ts;
        ts = container_of(h, struct ft5x0x_ts_data, early_suspend);

	dev_dbg(&ts->client->dev, "[FTS]ft5x0x suspend\n");
	disable_irq(ts->pdata->irq);

	ft5x0x_write_reg(ft5x0x_data.client, 0xa5, 0x03);
      //  printk("Holy ft5x0x_ts_suspend \n");
}

static void ft5x0x_ts_resume(struct early_suspend *handler)
{
        /*change*/
	struct ft5x0x_ts_data *ts;
        ts = container_of(handler, struct ft5x0x_ts_data, early_suspend);
      //  printk("Holy ft5x0x_ts_resume \n");
	dev_dbg(&ts->client->dev, "[FTS]ft5x0x resume.\n");
	gpio_set_value(ts->pdata->reset, 0);
	msleep(20);
	gpio_set_value(ts->pdata->reset, 1);
	enable_irq(ts->pdata->irq);
	ft5x0x_ts_release(ft5x0x_ts);
}
#else
#define ft5x0x_ts_suspend	NULL
#define ft5x0x_ts_resume	NULL
#endif
/*change*/
//static int __devexit ft5x0x_ts_remove(struct i2c_client *client)
static int  ft5x0x_ts_remove(struct i2c_client *client)
{
	struct ft5x0x_ts_data *ft5x0x_ts;
	ft5x0x_ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ft5x0x_data.early_suspend);
	input_unregister_device(ft5x0x_ts->input_dev);
	#ifdef CONFIG_PM
	gpio_free(ft5x0x_ts->pdata->reset);
	#endif

	#ifdef FTS_CTL_IIC
	ft_rw_iic_drv_exit();
	#endif
	#ifdef SYSFS_DEBUG
		ft5x0x_remove_sysfs(client);
	#endif

	#ifdef FTS_APK_DEBUG
		ft5x0x_release_apk_debug_channel();
	#endif

	fts_un_init_gpio_hw(ft5x0x_ts);

	free_irq(client->irq, ft5x0x_ts);

	kfree(ft5x0x_ts);
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const struct i2c_device_id ft5x0x_ts_id[] = {
	{FT5X0X_NAME, 0},
	{ "FTTH5406:00", 0 },
	{ "FTTH5406", 0 },
	{}
};

MODULE_DEVICE_TABLE(i2c, ft5x0x_ts_id);

#ifdef CONFIG_ACPI

static const struct acpi_device_id ft5x0x_acpi_match[] = {
	{ FT5X0X_NAME, 0 },
	{ "FTTH5406:00", 0 },
	{ "FTTH5406", 0 },
	{},
};

MODULE_DEVICE_TABLE(acpi, ft5x0x_acpi_match);
#endif


static struct i2c_driver ft5x0x_ts_driver = {
	.probe = ft5x0x_ts_probe,
	/*change*/
	//.remove = __devexit_p(ft5x0x_ts_remove),
	.remove = ft5x0x_ts_remove,
	.id_table = ft5x0x_ts_id,
	 #if 0 //def CONFIG_PM
	.suspend = ft5x0x_ts_suspend,
	.resume = ft5x0x_ts_resume,
         #endif
	.driver = {
		   .name = FT5X0X_NAME,
		   .owner = THIS_MODULE,
#ifdef CONFIG_ACPI
	.acpi_match_table = ACPI_PTR(ft5x0x_acpi_match),
#endif
		   },
};

static int __init ft5x0x_ts_init(void)
{
	int ret;
	ret = i2c_add_driver(&ft5x0x_ts_driver);
	if (ret) {
		printk(KERN_WARNING "Adding ft5x0x driver failed "
		       "(errno = %d)\n", ret);
	} else {
	//	pr_info("Successfully added driver %s\n",
	//		ft5x0x_ts_driver.driver.name);
	}
	return ret;
}

static void __exit ft5x0x_ts_exit(void)
{
	i2c_del_driver(&ft5x0x_ts_driver);
}

module_init(ft5x0x_ts_init);
module_exit(ft5x0x_ts_exit);

MODULE_AUTHOR("<luowj>");
MODULE_DESCRIPTION("FocalTech ft5x0x TouchScreen driver");
MODULE_LICENSE("GPL");
