/* 
 * drivers/input/touchscreen/ft5x06_ex_fun.c
 *
 * FocalTech ft5x0x expand function for debug. 
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
 *Note:the error code of EIO is the general error in this file.
 */


#include "ft5x06_ex_fun.h"
#include "ft5x06_ts.h"
#include <linux/mount.h>
#include <linux/netdevice.h>

struct Upgrade_Info{
	u16		delay_aa;		/*delay of write FT_UPGRADE_AA*/
	u16		delay_55;		/*delay of write FT_UPGRADE_55*/
	u8		upgrade_id_1;	/*upgrade id 1*/
	u8		upgrade_id_2;	/*upgrade id 2*/
	u16		delay_readid;	/*delay of read id*/
};

int  fts_ctpm_fw_upgrade(struct i2c_client * client, u8* pbt_buf, u32 dw_lenth);
static unsigned char fts_ctpm_VidFWid_get_from_boot(struct i2c_client *client);

static unsigned char CTPM_FW_BYD4[]=
{
	#include "Ft5x06_T7GD2_BYD4_ID0x59_VER0xA5_20141010_app.i"
};

static unsigned char CTPM_FW_SHENYUE[]=
{
        #include "Ft5x06_T7GD2_SHENYUE_ID0xA0_VER0xA9_20141014_app.i"
};


static struct mutex g_device_mutex;

int ft5x0x_write_reg(struct i2c_client * client, u8 regaddr, u8 regvalue)
{
	unsigned char buf[2] = {0};
	buf[0] = regaddr;
	buf[1] = regvalue;
	
	return ft5x0x_i2c_Write(client, buf, sizeof(buf));
}

int ft5x0x_read_reg(struct i2c_client * client, u8 regaddr, u8 * regvalue)
{
	return ft5x0x_i2c_Read(client, &regaddr, 1, regvalue, 1);
}
int fts_ctpm_auto_clb(struct i2c_client * client)
{
	unsigned char uc_temp;
	unsigned char i ;

	/*start auto CLB*/
	msleep(200);
	ft5x0x_write_reg(client, 0, 0x40);  
	msleep(100);   /*make sure already enter factory mode*/
	ft5x0x_write_reg(client, 2, 0x4);  /*write command to start calibration*/
	msleep(300);
	if (DEVICE_IC_TYPE == IC_FT5X36) {
		for(i=0;i<100;i++)
		{
			ft5x0x_read_reg(client, 0x02, &uc_temp);
			if (0x02 == uc_temp ||
				0xFF == uc_temp)  /*return to normal mode, calibration finish*/
			{
			    break;
			}
			msleep(20);	    
		}
	} else {
		for(i=0;i<100;i++)
		{
			ft5x0x_read_reg(client, 0, &uc_temp);
			if (0x0 == ((uc_temp&0x70)>>4))  /*return to normal mode, calibration finish*/
			{
			    break;
			}
			msleep(20);	    
		}
	}
	/*calibration OK*/

	msleep(300);
	ft5x0x_write_reg(client, 0, 0x40);  /*goto factory mode for store*/
	msleep(100);   /*make sure already enter factory mode*/
	ft5x0x_write_reg(client, 2, 0x5);  /*store CLB result*/
	msleep(300);
	ft5x0x_write_reg(client, 0, 0x0); /*return to normal mode*/ 
	msleep(300);
	/*store CLB result OK*/
	
	return 0;
}

/*
upgrade with *.i file
*/
int fts_ctpm_fw_upgrade_with_i_file(struct i2c_client * client)
{
	u8 * pbt_buf = NULL;
	int i_ret;
	u8 module_id;
	int fw_len;
	ft5x0x_read_reg(client, FT5x0x_REG_VENDOR_ID, &module_id);
	

	/*judge the fw that will be upgraded
	 * if illegal, then stop upgrade and return.
	*/


	if(module_id==0x59)
		fw_len= sizeof(CTPM_FW_BYD4);
	else
		fw_len= sizeof(CTPM_FW_SHENYUE);
	if (fw_len < 8 || fw_len > 32 * 1024) 
	{
		dev_err(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}

	if(module_id==0x59)
	{
		if((CTPM_FW_BYD4[fw_len-8]^CTPM_FW_BYD4[fw_len-6])==0xFF
			&& (CTPM_FW_BYD4[fw_len-7]^CTPM_FW_BYD4[fw_len-5])==0xFF
			&& (CTPM_FW_BYD4[fw_len-3]^CTPM_FW_BYD4[fw_len-4])==0xFF)
		{
			/*FW upgrade*/
			pbt_buf = CTPM_FW_BYD4;
			/*call the upgrade function*/
			i_ret =  fts_ctpm_fw_upgrade(client, pbt_buf, sizeof(CTPM_FW_BYD4));
			if (i_ret != 0)
			{
				dev_err(&client->dev, "[FTS] upgrade failed. err=%d.\n", i_ret);
			}
			else
			{
				#ifdef AUTO_CLB
				fts_ctpm_auto_clb(client);  /*start auto CLB*/
				#endif
			}
		}
		else
		{
			dev_err(&client->dev, "FW format error\n");
			return -EBADFD;
		}
	}
        else if(module_id==0xA0)
        {
                if((CTPM_FW_SHENYUE[fw_len-8]^CTPM_FW_SHENYUE[fw_len-6])==0xFF
                        && (CTPM_FW_SHENYUE[fw_len-7]^CTPM_FW_SHENYUE[fw_len-5])==0xFF
                        && (CTPM_FW_SHENYUE[fw_len-3]^CTPM_FW_SHENYUE[fw_len-4])==0xFF)
                {
                        /*FW upgrade*/
                        pbt_buf = CTPM_FW_SHENYUE;
                        /*call the upgrade function*/
                        i_ret =  fts_ctpm_fw_upgrade(client, pbt_buf, sizeof(CTPM_FW_SHENYUE));
                        if (i_ret != 0)
                        {
                                dev_err(&client->dev, "[FTS] upgrade failed. err=%d.\n", i_ret);
                        }
                        else
                        {
                                #ifdef AUTO_CLB
                                fts_ctpm_auto_clb(client);  /*start auto CLB*/
                                #endif
                        }
                }
                else
                {
                        dev_err(&client->dev, "FW format error\n");
                        return -EBADFD;
                }
        }
	
	return i_ret;
	
}

u8 fts_ctpm_get_i_file_ver(struct i2c_client *client)
{
    u16 ui_sz;
    u8 module_id;
    ft5x0x_read_reg(client, FT5x0x_REG_VENDOR_ID, &module_id);
    if(module_id==0x59)
    {
        ui_sz = sizeof(CTPM_FW_BYD4);
        if (ui_sz > 2)
		return CTPM_FW_BYD4[ui_sz - 2];
    }
    else
    {
	ui_sz = sizeof(CTPM_FW_SHENYUE);
	if (ui_sz > 2)
		return CTPM_FW_SHENYUE[ui_sz - 2];
    } 

       return 0x00; /*default value*/
}

int fts_ctpm_fw_upgrade_with_i_file_auto(struct i2c_client *client,u8 vendor_id)
{
	u8 *pbt_buf = NULL;
	int i_ret;
	int fw_len = 0; 
	/*judge the fw that will be upgraded
	* if illegal, then stop upgrade and return.
	*/
	if(vendor_id==0x59)
	{
		fw_len= sizeof(CTPM_FW_BYD4);
		pbt_buf = CTPM_FW_BYD4;
	}
	else if(vendor_id==0xA0)
	{
		fw_len= sizeof(CTPM_FW_SHENYUE);
		pbt_buf = CTPM_FW_SHENYUE;
	}
	if (fw_len < 8 || fw_len > 32 * 1024) {
		dev_err(&client->dev, "%s:FW length error\n", __func__);
		return -EIO;
	}
 	if ((pbt_buf[fw_len - 8] ^ pbt_buf[fw_len - 6]) == 0xFF
		&& (pbt_buf[fw_len - 7] ^ pbt_buf[fw_len - 5]) == 0xFF
		&& (pbt_buf[fw_len - 3] ^ pbt_buf[fw_len - 4]) == 0xFF) {
		/*FW upgrade */
		/*call the upgrade function */
		i_ret = fts_ctpm_fw_upgrade(client, pbt_buf, fw_len);
		if (i_ret != 0)
			dev_err(&client->dev, "%s:upgrade failed. err.\n",__func__);
#ifdef AUTO_CLB
		else
			fts_ctpm_auto_clb(client);	/*start auto CLB */
#endif
	} else {
		dev_err(&client->dev, "%s:FW format error\n", __func__);
		return -EBADFD;
	}
	return i_ret;
}

u8 fts_ctpm_get_i_file_ver_auto(u8 vendor_id)
{
	u16 ui_sz;		
	if(vendor_id==0x59)
		{
	ui_sz = sizeof(CTPM_FW_BYD4);
	if (ui_sz > 2)
		return CTPM_FW_BYD4[ui_sz - 2];
		}
	else if(vendor_id==0xA0)
		{
	ui_sz = sizeof(CTPM_FW_SHENYUE);
	if (ui_sz > 2)
		return CTPM_FW_SHENYUE[ui_sz - 2];
		}
	return 0x00;	/*default value */
}
int fts_ctpm_auto_upgrade(struct i2c_client * client)
{
	u8 uc_host_fm_ver =  FT5x0x_REG_FW_VER;
	u8 uc_tp_fm_ver;
	u8 uc_tp_vendor_id = 0;
        u8 vid = 0;
	int i_ret;
	int i = 0;
	for(i = 0; i <= 10; i++)
	{
		ft5x0x_read_reg(client, FT5x0x_REG_FW_VER, &uc_tp_fm_ver);
		if(uc_tp_fm_ver!=0)
			break;
	}
	for(i = 0; i <= 10; i++)
	{
		ft5x0x_read_reg(client, FT5x0x_REG_VENDOR_ID, &uc_tp_vendor_id);
		if((uc_tp_vendor_id == 0x59) || (uc_tp_vendor_id == 0xa0))
			break;
	}
	if ((0x59 != uc_tp_vendor_id) && (0xA0 != uc_tp_vendor_id))  //0x59 == byd4 ,0xA0 == shenyue
	{
//	  printk("[FST] reg_module_id do not known error !\n");
	  //get Vendor ID & FW Version from CTP module' BOOT
	  vid = fts_ctpm_VidFWid_get_from_boot( client);
  //         printk("[FST] VID= 0x%x!\n", vid);
	  if ( vid == 0xFF )
	  {// vid error or read fail
		  return -1;
	  }
	  else
	  {
		  uc_tp_vendor_id = vid;
		  uc_tp_fm_ver = 0;//flash damaged, force to upgrade the FW
	  }
  }		
	uc_host_fm_ver = fts_ctpm_get_i_file_ver_auto(uc_tp_vendor_id);
      printk("[FTS] YLL_uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x,uc_tp_vendor_id = 0x%x\n",uc_tp_fm_ver, uc_host_fm_ver,uc_tp_vendor_id);
      printk("[FTS] yll_uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x,uc_tp_vendor_id = 0x%x\n",uc_tp_fm_ver, uc_host_fm_ver,uc_tp_vendor_id);
      printk("[FTS] YLL_uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x,uc_tp_vendor_id = 0x%x\n",uc_tp_fm_ver, uc_host_fm_ver,uc_tp_vendor_id);
      printk("[FTS] yll_uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x,uc_tp_vendor_id = 0x%x\n",uc_tp_fm_ver, uc_host_fm_ver,uc_tp_vendor_id);
	if ( (uc_tp_fm_ver == FT5x0x_REG_FW_VER) || (uc_tp_fm_ver < uc_host_fm_ver))
	{
		msleep(100);
		dev_dbg(&client->dev, "[FTS] uc_tp_fm_ver = 0x%x, uc_host_fm_ver = 0x%x\n",
				uc_tp_fm_ver, uc_host_fm_ver);
		i_ret = fts_ctpm_fw_upgrade_with_i_file_auto(client,uc_tp_vendor_id);
		if (i_ret == 0)	{
			msleep(300);
			uc_host_fm_ver = fts_ctpm_get_i_file_ver_auto(uc_tp_vendor_id);
			dev_dbg(&client->dev, "[FTS] upgrade to new version 0x%x\n",uc_host_fm_ver);
		} else {
			pr_err("[FTS] upgrade failed ret=%d.\n", i_ret);
			return -EIO;
		}
	}
	return 0;
}

/*
*get upgrade information depend on the ic type
*/
static void fts_get_upgrade_info(struct Upgrade_Info * upgrade_info)
{
	switch(DEVICE_IC_TYPE)
	{
	case IC_FT5X06:
		upgrade_info->delay_55 = FT5X06_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X06_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X06_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X06_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X06_UPGRADE_READID_DELAY;
		break;
	case IC_FT5606:
		upgrade_info->delay_55 = FT5606_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5606_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5606_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5606_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5606_UPGRADE_READID_DELAY;
		break;
	case IC_FT5316:
		upgrade_info->delay_55 = FT5316_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5316_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5316_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5316_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5316_UPGRADE_READID_DELAY;
		break;
	case IC_FT5X36:
		upgrade_info->delay_55 = FT5X36_UPGRADE_55_DELAY;
		upgrade_info->delay_aa = FT5X36_UPGRADE_AA_DELAY;
		upgrade_info->upgrade_id_1 = FT5X36_UPGRADE_ID_1;
		upgrade_info->upgrade_id_2 = FT5X36_UPGRADE_ID_2;
		upgrade_info->delay_readid = FT5X36_UPGRADE_READID_DELAY;
		break;
	default:
		break;
	}
}
#define FTS_UPGRADE_LOOP	3

int ft5x36_read_project_code(struct i2c_client * client, char * pProjectCode) {
	u8 reg_val[2] = {0};
	u32 i = 0;
	u32  j;
	u32  temp;
	u8 	packet_buf[4];
	u8  	auc_i2c_write_buf[10];
	int      i_ret;
	struct Upgrade_Info upgradeinfo;

	fts_get_upgrade_info(&upgradeinfo);

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
    	/*********Step 1:Reset  CTPM *****/
    	/*write 0xaa to register 0xfc*/
	   	ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);
		
		 /*write 0x55 to register 0xfc*/
		ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);   
		msleep(upgradeinfo.delay_55);   

		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		
	    i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
	  
	    /*********Step 3:check READ-ID***********************/   
		msleep(upgradeinfo.delay_readid);
	   	auc_i2c_write_buf[0] = 0x90; 
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;

		ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		
		if (reg_val[0] == upgradeinfo.upgrade_id_1 
			&& reg_val[1] == upgradeinfo.upgrade_id_2)
		{
	    	dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	    	break;
		}
		else
		{
			dev_err(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	    	continue;
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;

	/*read project code*/

	packet_buf[0] = 0x03;
	packet_buf[1] = 0x00;
	for (j=0;j<33;j++)
	{
		temp = 0x07d0 + j;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;

		ft5x0x_i2c_Read(client, packet_buf, sizeof(packet_buf), 
			pProjectCode+j, 1);
		if (*(pProjectCode+j) == '\0')
			break;
	}
	pr_info("project code = %s \n", pProjectCode);
	
	/*********reset the new FW***********************/
	auc_i2c_write_buf[0] = 0x07;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);  /*make sure CTP startup normally*/
	
	return 0;
}

int  fts_ctpm_fw_upgrade(struct i2c_client * client, u8* pbt_buf, u32 dw_lenth)
{
	
	u8 reg_val[2] = {0};
	u32 i = 0;
	u8 is_5336_new_bootloader = 0;
	u32  packet_number;
	u32  j;
	u32  temp;
	u32  lenght;
	u8 	packet_buf[FTS_PACKET_LENGTH + 6];
	u8  	auc_i2c_write_buf[10];
	u8  	bt_ecc;
	int      i_ret;
	struct Upgrade_Info upgradeinfo;

	fts_get_upgrade_info(&upgradeinfo);

	for (i = 0; i < FTS_UPGRADE_LOOP; i++) {
    	/*********Step 1:Reset  CTPM *****/
    	/*write 0xaa to register 0xfc*/
	   	ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
		msleep(upgradeinfo.delay_aa);
		
		 /*write 0x55 to register 0xfc*/
		ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);   
		msleep(upgradeinfo.delay_55);   

		/*********Step 2:Enter upgrade mode *****/
		auc_i2c_write_buf[0] = FT_UPGRADE_55;
		auc_i2c_write_buf[1] = FT_UPGRADE_AA;
		
	    i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
	  
	    /*********Step 3:check READ-ID***********************/   
		msleep(upgradeinfo.delay_readid);
	   	auc_i2c_write_buf[0] = 0x90; 
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] = 0x00;

		ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		
		if (reg_val[0] == upgradeinfo.upgrade_id_1 
			&& reg_val[1] == upgradeinfo.upgrade_id_2)
		{
	    	dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	    	break;
		}
		else
		{
			dev_err(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0],reg_val[1]);
	    	continue;
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
		
	auc_i2c_write_buf[0] = 0xcd;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	if (reg_val[0] > 4)
		is_5336_new_bootloader = 1;

     /*********Step 4:erase app and panel paramenter area ********************/
	auc_i2c_write_buf[0] = 0x61;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1); /*erase app area*/	
    msleep(FT_UPGRADE_EARSE_DELAY); 

	/*********Step 5:write firmware(FW) to ctpm flash*********/
	bt_ecc = 0;

	dw_lenth = dw_lenth - 8;
	packet_number = (dw_lenth) / FTS_PACKET_LENGTH;
	packet_buf[0] = 0xbf;
	packet_buf[1] = 0x00;
	for (j=0;j<packet_number;j++)
	{
		temp = j * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;
		lenght = FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(lenght>>8);
		packet_buf[5] = (u8)lenght;

		for (i=0;i<FTS_PACKET_LENGTH;i++)
		{
		    packet_buf[6+i] = pbt_buf[j*FTS_PACKET_LENGTH + i]; 
		    bt_ecc ^= packet_buf[6+i];
		}

		ft5x0x_i2c_Write(client, packet_buf, FTS_PACKET_LENGTH+6);
		msleep(FTS_PACKET_LENGTH/6 + 1);
	}

	if ((dw_lenth) % FTS_PACKET_LENGTH > 0)
	{
		temp = packet_number * FTS_PACKET_LENGTH;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;

		temp = (dw_lenth) % FTS_PACKET_LENGTH;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;

		for (i=0;i<temp;i++)
		{
		    packet_buf[6+i] = pbt_buf[ packet_number*FTS_PACKET_LENGTH + i]; 
		    bt_ecc ^= packet_buf[6+i];
		}
  
		ft5x0x_i2c_Write(client, packet_buf, temp+6);
		msleep(20);
	}

	/*send the last six byte*/
	for (i = 0; i<6; i++)
	{
		if (is_5336_new_bootloader && DEVICE_IC_TYPE==IC_FT5X36) 
			temp = 0x7bfa + i;
		else
			temp = 0x6ffa + i;
		packet_buf[2] = (u8)(temp>>8);
		packet_buf[3] = (u8)temp;
		temp =1;
		packet_buf[4] = (u8)(temp>>8);
		packet_buf[5] = (u8)temp;
		packet_buf[6] = pbt_buf[ dw_lenth + i]; 
		bt_ecc ^= packet_buf[6];
  
		ft5x0x_i2c_Write(client, packet_buf, 7);
		msleep(20);
	}

	/*********Step 6: read out checksum***********************/
	/*send the opration head*/
	auc_i2c_write_buf[0] = 0xcc;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1); 

	if(reg_val[0] != bt_ecc)
	{
		dev_err(&client->dev, "[FTS]--ecc error! FW=%02x bt_ecc=%02x\n", reg_val[0], bt_ecc);
	    	return -EIO;
	}

	/*********Step 7: reset the new FW***********************/
	auc_i2c_write_buf[0] = 0x07;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);  /*make sure CTP startup normally*/

	return 0;
}

static unsigned char fts_ctpm_VidFWid_get_from_boot(struct i2c_client *client)
{
    unsigned char auc_i2c_write_buf[10];
    unsigned char reg_val[2] = {0};
    unsigned char i = 0;
    unsigned char vid = 0xFF;
	//  int i_ret;
        struct Upgrade_Info upgradeinfo;   
	fts_get_upgrade_info(&upgradeinfo);

	//focaltech_get_upgrade_array();
	
	for (i = 0; i < FTS_UPGRADE_LOOP; i++) 
	{
	        msleep(100);
		printk("[FTS] Step 1:Reset  CTPM\n");
		/*********Step 1:Reset  CTPM *****/
        	/*write 0xaa to register 0xfc*/
                ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_AA);
                msleep(upgradeinfo.delay_aa);

                 /*write 0x55 to register 0xfc*/
                ft5x0x_write_reg(client, 0xfc, FT_UPGRADE_55);
                msleep(upgradeinfo.delay_55);	
   
		/*********Step 2:Enter upgrade mode *****/
		printk("[FTS] Step 2:Enter upgrade mode \n");
		#if 0
			auc_i2c_write_buf[0] = FT_UPGRADE_55;
			auc_i2c_write_buf[1] = FT_UPGRADE_AA;
			do {
				j++;
				i_ret = ft5x0x_i2c_Write(client, auc_i2c_write_buf, 2);
				msleep(5);
			} while (i_ret <= 0 && j < 5);
		#else
			auc_i2c_write_buf[0] = FT_UPGRADE_55;
			ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
			msleep(5);
			auc_i2c_write_buf[0] = FT_UPGRADE_AA;
			ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
		#endif

		/*********Step 3:check READ-ID***********************/
		msleep(upgradeinfo.delay_readid);
		auc_i2c_write_buf[0] = 0x90;
		auc_i2c_write_buf[1] = auc_i2c_write_buf[2] = auc_i2c_write_buf[3] =0x00;
		ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
		printk("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",reg_val[0], reg_val[1]);
		if (reg_val[0] == upgradeinfo.upgrade_id_1
			&& reg_val[1] == upgradeinfo.upgrade_id_2) {
			//dev_dbg(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				//reg_val[0], reg_val[1]);
			dev_err("[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
			break;
		} else {
			dev_err(&client->dev, "[FTS] Step 3: CTPM ID,ID1 = 0x%x,ID2 = 0x%x\n",
				reg_val[0], reg_val[1]);
		}
	}
	if (i >= FTS_UPGRADE_LOOP)
		return -EIO;
	auc_i2c_write_buf[0] = 0xcd;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 1, reg_val, 1);
	printk("[FTS]bootloader version = 0x%x\n", reg_val[0]);
	/*get Vender ID info */
	auc_i2c_write_buf[0] = 0x03;
	auc_i2c_write_buf[1] = 0x00;
	auc_i2c_write_buf[2] = 0x78;
	auc_i2c_write_buf[3] = 0x04;
	ft5x0x_i2c_Read(client, auc_i2c_write_buf, 4, reg_val, 2);
	printk("%s: REG VAL ID1 = 0x%x,ID2 = 0x%x\n", __func__,reg_val[0],reg_val[1]);	
	printk("%s: REG VAL ID1 = 0x%x,ID2 = 0x%x\n", __func__,reg_val[0],(~reg_val[1]));
	if (reg_val[0]==((~reg_val[1])&0xff))
	{
		vid = reg_val[0];
	}else
	{
	//error Vid or read fail
	vid = 0xFF;
	}
    /*********Step 7: reset the new FW***********************/
	printk("Step 7: reset the new FW\n");
	auc_i2c_write_buf[0] = 0x07;
	ft5x0x_i2c_Write(client, auc_i2c_write_buf, 1);
	msleep(300);	/*make sure CTP startup normally */
	return vid;     
}


/* sysfs debug*/

/*
*get firmware size

@firmware_name:firmware name

note:the firmware default path is sdcard.
	if you want to change the dir, please modify by yourself.
*/
static int ft5x0x_GetFirmwareSize(char * firmware_name)
{
	struct file* pfile = NULL;
	struct inode *inode;
	unsigned long magic; 
	off_t fsize = 0; 
	char filepath[128];
	memset(filepath, 0, sizeof(filepath));

	sprintf(filepath, "%s", firmware_name);

	if(NULL == pfile){
		pfile = filp_open(filepath, O_RDONLY, 0);
		}
	if(IS_ERR(pfile)){
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
		}
	inode=pfile->f_dentry->d_inode; 
	magic=inode->i_sb->s_magic;
	fsize=inode->i_size; 
	filp_close(pfile, NULL);
	
	return fsize;
}

/*
*read firmware buf for .bin file.

@firmware_name: fireware name
@firmware_buf: data buf of fireware

note:the firmware default path is sdcard. 
	if you want to change the dir, please modify by yourself.
*/
static int ft5x0x_ReadFirmware(char * firmware_name, unsigned char * firmware_buf)
{
	struct file* pfile = NULL;
	struct inode *inode;
	unsigned long magic; 
	off_t fsize; 
	char filepath[128];
	loff_t pos;

	mm_segment_t old_fs;
	memset(filepath, 0, sizeof(filepath));
	sprintf(filepath, "%s", firmware_name);
	if(NULL == pfile){
		pfile = filp_open(filepath, O_RDONLY, 0);
		}
	if(IS_ERR(pfile)){
		pr_err("error occured while opening file %s.\n", filepath);
		return -EIO;
		}
	inode=pfile->f_dentry->d_inode; 
	magic=inode->i_sb->s_magic;
	fsize=inode->i_size; 
	old_fs = get_fs();
	set_fs(KERNEL_DS);
	pos = 0;

	vfs_read(pfile, firmware_buf, fsize, &pos);

	filp_close(pfile, NULL);
	set_fs(old_fs);
	return 0;
}

/*
upgrade with *.bin file
*/

int fts_ctpm_fw_upgrade_with_app_file(struct i2c_client * client, char * firmware_name)
{
  	u8*     pbt_buf = NULL;
   	int i_ret;
   	int fwsize = ft5x0x_GetFirmwareSize(firmware_name);
   	if(fwsize <= 0)
   	{
   		dev_err(&client->dev, "%s ERROR:Get firmware size failed\n", __FUNCTION__);
		return -EIO;
   	}
	if(fwsize<8 || fwsize>32*1024)
	{
		dev_err(&client->dev, "FW length error\n");
		return -EIO;
	}
	
    /*=========FW upgrade========================*/
  	 pbt_buf = (unsigned char *) kmalloc(fwsize+1,GFP_ATOMIC);
	if(ft5x0x_ReadFirmware(firmware_name, pbt_buf))
    	{
       	dev_err(&client->dev, "%s() - ERROR: request_firmware failed\n", __FUNCTION__);
        	kfree(pbt_buf);
		return -EIO;
    	}
	if((pbt_buf[fwsize-8]^pbt_buf[fwsize-6])==0xFF
		&& (pbt_buf[fwsize-7]^pbt_buf[fwsize-5])==0xFF
		&& (pbt_buf[fwsize-3]^pbt_buf[fwsize-4])==0xFF)
	{
		/*call the upgrade function*/
		i_ret =  fts_ctpm_fw_upgrade(client, pbt_buf, fwsize);
   		if (i_ret != 0)
   		{
       		dev_err(&client->dev, "%s() - ERROR:[FTS] upgrade failed i_ret = %d.\n",__FUNCTION__,  i_ret);
   		}
  	 	else
   		{
		
      		fts_ctpm_auto_clb(client);  //start auto CLB
   		}
		kfree(pbt_buf);
	}
	else
	{
		dev_dbg(&client->dev, "FW format error\n");
		kfree(pbt_buf);
		return -EIO;
	}
   	return i_ret;
}

static ssize_t ft5x0x_tpfwver_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	u8	   fwver = 0;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	
	mutex_lock(&g_device_mutex);
	if(ft5x0x_read_reg(client, FT5x0x_REG_FW_VER, &fwver) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get tp fw version fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "%02X\n", fwver);

	mutex_unlock(&g_device_mutex);
	return num_read_chars;
}

static ssize_t ft5x0x_tpfwver_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ft5x0x_tprwreg_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
	return -EPERM;
}

static ssize_t ft5x0x_tprwreg_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);	
	ssize_t num_read_chars = 0;
	int retval;
	//u32 wmreg=0;
	long unsigned int wmreg = 0;
	u8 regaddr=0xff,regvalue=0xff;
	u8 valbuf[5]={0};
	memset(valbuf, 0, sizeof(valbuf));
	
	mutex_lock(&g_device_mutex);
	num_read_chars = count - 1;

	if(num_read_chars!=2)
	{
		if(num_read_chars!=4)
		{
			dev_err(dev, "please input 2 or 4 character\n");
			goto error_return;
		}
	}
	
	memcpy(valbuf, buf, num_read_chars);
	retval = strict_strtoul(valbuf, 16, &wmreg);
	if (0 != retval)
	{
    	dev_err(dev, "%s() - ERROR: Could not convert the given input to a number. The given input was: \"%s\"\n", __FUNCTION__, buf);
    	goto error_return;
	}

	if(2 == num_read_chars)
	{
		//read register
		regaddr = wmreg;
		if(ft5x0x_read_reg(client, regaddr, &regvalue) < 0)
			dev_err(dev, "Could not read the register(0x%02x)\n", regaddr);
		else
			dev_dbg(dev, "the register(0x%02x) is 0x%02x\n", regaddr, regvalue);
	}
	else
	{
		regaddr = wmreg>>8;
		regvalue = wmreg;
		if(ft5x0x_write_reg(client, regaddr, regvalue)<0)
			dev_err(dev, "Could not write the register(0x%02x)\n", regaddr);
		else
			dev_dbg(dev, "Write 0x%02x into register(0x%02x) successful\n", regvalue, regaddr);
	}
error_return:
	mutex_unlock(&g_device_mutex);

	return count;
}


static ssize_t ft5x0x_fwupdate_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
    return -EPERM;
}

/*upgrade from *.i*/
static ssize_t ft5x0x_fwupdate_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	struct ft5x0x_ts_data *data = NULL;
	u8 uc_host_fm_ver;int i_ret;
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	data = (struct ft5x0x_ts_data *) i2c_get_clientdata( client );
	
	mutex_lock(&g_device_mutex);

	disable_irq(client->irq);
	i_ret = fts_ctpm_fw_upgrade_with_i_file(client);    
	if (i_ret == 0)
	{
	    msleep(300);
	    uc_host_fm_ver = fts_ctpm_get_i_file_ver(client);
	    dev_dbg(dev, "%s [FTS] upgrade to new version 0x%x\n", __FUNCTION__, uc_host_fm_ver);
	}
	else
	{
	    dev_err(dev, "%s ERROR:[FTS] upgrade failed ret=%d.\n", __FUNCTION__, i_ret);
	}
	enable_irq(client->irq);
	
	mutex_unlock(&g_device_mutex);

	return count;
}

static ssize_t ft5x0x_fwupgradeapp_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	/* place holder for future use */
    return -EPERM;
}
//upgrade from app.bin
static ssize_t ft5x0x_fwupgradeapp_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	char fwname[128];
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);
	memset(fwname, 0, sizeof(fwname));
	sprintf(fwname, "%s", buf);
	fwname[count-1] = '\0';

	mutex_lock(&g_device_mutex);
	disable_irq(client->irq);
	
	fts_ctpm_fw_upgrade_with_app_file(client, fwname);
	
	enable_irq(client->irq);

	mutex_unlock(&g_device_mutex);

	return count;
}

static ssize_t ft5x0x_ftsgetprojectcode_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	ssize_t num_read_chars = 0;
	char projectcode[32]; 
	struct i2c_client *client = container_of(dev, struct i2c_client, dev);

	memset(projectcode, 0, sizeof(projectcode));
	mutex_lock(&g_device_mutex);
	if(ft5x36_read_project_code(client, projectcode) < 0)
		num_read_chars = snprintf(buf, PAGE_SIZE, "get projcet code fail!\n");
	else
		num_read_chars = snprintf(buf, PAGE_SIZE, "projcet code = %s\n", projectcode);

	mutex_unlock(&g_device_mutex);
	return num_read_chars;

	
}
//upgrade from app.bin
static ssize_t ft5x0x_ftsgetprojectcode_store(struct device *dev,
					struct device_attribute *attr,
						const char *buf, size_t count)
{
	/* place holder for future use */
    return -EPERM;
}


/* sysfs */
/*get the fw version
*example:cat ftstpfwver
*/
static DEVICE_ATTR(ftstpfwver, S_IRUGO|S_IWUSR, ft5x0x_tpfwver_show, ft5x0x_tpfwver_store);

/*upgrade from *.i 
*example: echo 1 > ftsfwupdate
*/
static DEVICE_ATTR(ftsfwupdate, S_IRUGO|S_IWUSR, ft5x0x_fwupdate_show, ft5x0x_fwupdate_store);

/*read and write register
*read example: echo 88 > ftstprwreg ---read register 0x88
*write example:echo 8807 > ftstprwreg ---write 0x07 into register 0x88
*
*note:the number of input must be 2 or 4.if it not enough,please fill in the 0.
*/
static DEVICE_ATTR(ftstprwreg, S_IRUGO|S_IWUSR, ft5x0x_tprwreg_show, ft5x0x_tprwreg_store);

/*upgrade from app.bin 
*example:echo "*_app.bin" > ftsfwupgradeapp
*/
static DEVICE_ATTR(ftsfwupgradeapp, S_IRUGO|S_IWUSR, ft5x0x_fwupgradeapp_show, ft5x0x_fwupgradeapp_store);

static DEVICE_ATTR(ftsgetprojectcode, S_IRUGO|S_IWUSR, ft5x0x_ftsgetprojectcode_show, ft5x0x_ftsgetprojectcode_store);


/*add your attr in here*/
static struct attribute *ft5x0x_attributes[] = {
	&dev_attr_ftstpfwver.attr,
	&dev_attr_ftsfwupdate.attr,
	&dev_attr_ftstprwreg.attr,
	&dev_attr_ftsfwupgradeapp.attr,
	&dev_attr_ftsgetprojectcode.attr,
	NULL
};

static struct attribute_group ft5x0x_attribute_group = {
	.attrs = ft5x0x_attributes
};

/*create sysfs for debug*/
int ft5x0x_create_sysfs(struct i2c_client * client)
{
	int err;
	err = sysfs_create_group(&client->dev.kobj, &ft5x0x_attribute_group);
   	if (0 != err)
  	{
		dev_err(&client->dev, "%s() - ERROR: sysfs_create_group() failed.error code: %d\n", __FUNCTION__, err);
		sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
		return -EIO;
  	}
   	else
    	{		
		mutex_init(&g_device_mutex);
        	dev_dbg(&client->dev, "ft5x0x:%s() - sysfs_create_group() succeeded. \n", __FUNCTION__);
    	}
	return err;
}

int ft5x0x_remove_sysfs(struct i2c_client * client)
{
	sysfs_remove_group(&client->dev.kobj, &ft5x0x_attribute_group);
	mutex_destroy(&g_device_mutex);

	return 0;
}
/*create apk debug channel*/
#define PROC_UPGRADE                    0
#define PROC_READ_REGISTER              1
#define PROC_WRITE_REGISTER             2
#define PROC_AUTOCLB                    4

#define PROC_NAME       "ft5x0x-debug"
static unsigned char proc_operate_mode = PROC_UPGRADE;
static struct i2c_client *ft_client = NULL;
static struct proc_dir_entry *ft5x0x_proc_entry = NULL;
static int ft5x0x_debug_write(struct file *filp, const char __user *buff, unsigned long len, void *data);
static int ft5x0x_debug_read( char *page, char **start, off_t off, int count, int *eof, void *data );

static const struct file_operations debug_ops = {
      .owner = THIS_MODULE,
      .read = ft5x0x_debug_read,
      .write = ft5x0x_debug_write,
  };

/*interface of write proc*/
static int ft5x0x_debug_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
{

        struct i2c_client *client = ft_client;

        unsigned char writebuf[FTS_PACKET_LENGTH];
        int buflen = len;
        int writelen = 0;
        int ret = 0;

        if (copy_from_user(&writebuf, buff, buflen)) {
                dev_err(&client->dev, "%s:copy from user error\n", __func__);
                return -EFAULT;
        }
        proc_operate_mode = writebuf[0];

        switch (proc_operate_mode) {
        case PROC_UPGRADE:
                {
                        char upgrade_file_path[128];
                        memset(upgrade_file_path, 0, sizeof(upgrade_file_path));
                        sprintf(upgrade_file_path, "%s", writebuf + 1);
                        upgrade_file_path[buflen-1] = '\0';
                        disable_irq(client->irq);

                        ret = fts_ctpm_fw_upgrade_with_app_file(client, upgrade_file_path);

                        enable_irq(client->irq);
                        if (ret < 0) {
                                dev_err(&client->dev, "%s:upgrade failed.\n", __func__);
                                return ret;
                        }
                }
                break;
	case PROC_READ_REGISTER:
                writelen = 1;
                ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
                if (ret < 0) {
                        dev_err(&client->dev, "%s:write iic error\n", __func__);
                        return ret;
                }
                break;
        case PROC_WRITE_REGISTER:
                writelen = 2;
                ret = ft5x0x_i2c_Write(client, writebuf + 1, writelen);
                if (ret < 0) {
                        dev_err(&client->dev, "%s:write iic error\n", __func__);
                        return ret;
                }
                break;
        case PROC_AUTOCLB:
                fts_ctpm_auto_clb(client);
                break;
        default:
                break;
        }


        return len;
}
/*interface of read proc*/
static int ft5x0x_debug_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
        /*change*/
        struct i2c_client *client = ft_client;

        int ret = 0;
        /*int ret = 0, err = 0;
 *         u8 tx = 0, rx = 0;
 *                 int i, j;*/
        unsigned char buf[PAGE_SIZE];
        int num_read_chars = 0;
        int readlen = 0;
        u8 regvalue = 0x00, regaddr = 0x00;
        switch (proc_operate_mode) {
        case PROC_UPGRADE:
                /*after calling ft5x0x_debug_write to upgrade*/
                regaddr = 0xA6;
                ret = ft5x0x_read_reg(client, regaddr, &regvalue);
                if (ret < 0)
                        num_read_chars = sprintf(buf, "%s", "get fw version failed.\n");
                else
                        num_read_chars = sprintf(buf, "current fw version:0x%02x\n", regvalue);
                break;
        case PROC_READ_REGISTER:
                readlen = 1;
                ret = ft5x0x_i2c_Read(client, NULL, 0, buf, readlen);
                if (ret < 0) {
                        dev_err(&client->dev, "%s:read iic error\n", __func__);
                        return ret;
                } else
                num_read_chars = 1;
                break;
        default:
                break;
        }

        memcpy(page, buf, num_read_chars);

        return num_read_chars;
}
int ft5x0x_create_apk_debug_channel(struct i2c_client * client)
{
        /*change*/
        ft5x0x_proc_entry = proc_create(PROC_NAME, 0777, NULL, &debug_ops);
        if (NULL == ft5x0x_proc_entry) {
                dev_err(&client->dev, "Couldn't create proc entry!\n");
                return -ENOMEM;
        } else {
               // dev_info(&client->dev, "Create proc entry success!\n");
                /*change*/
                ft_client = client;
        }
        return 0;
}

void ft5x0x_release_apk_debug_channel(void)
{
        if (ft5x0x_proc_entry)
                remove_proc_entry(PROC_NAME, NULL);
}

