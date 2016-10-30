



/*****************************************************************************
 *
 * Filename:
 * ---------
 *   Sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <asm/io.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "hm03d5yuv_Sensor.h"
#include "hm03d5yuv_Camera_Sensor_para.h"
#include "hm03d5yuv_CameraCustomized.h"


#define HM03D5YUV_DEBUG
#ifdef HM03D5YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif


extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
void HM03D5_write_cmos_sensor(kal_uint16 addr, kal_uint8 para)
{
    char puSendCmd[3] = {(char)((addr & 0xFF00)>>8), (char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 3,HM03D5_WRITE_ID);

}
kal_uint8 HM03D5_read_cmos_sensor(kal_uint16 addr)
{
	kal_uint8 get_byte=0;
    char puSendCmd[2] = {(char)((addr & 0xFF00)>>8), (char)(addr & 0xFF)};
	iReadRegI2C(puSendCmd , 2, (u8*)&get_byte,1,HM03D5_WRITE_ID);
	
    return get_byte;
}


#define	HM03D5_LIMIT_EXPOSURE_LINES				(1253)
#define	HM03D5_VIDEO_NORMALMODE_30FRAME_RATE       (30)
#define	HM03D5_VIDEO_NORMALMODE_FRAME_RATE         (15)
#define	HM03D5_VIDEO_NIGHTMODE_FRAME_RATE          (7.5)
#define BANDING50_30HZ
/* Global Valuable */

static kal_uint32 zoom_factor = 0; 
static kal_uint8 HM03D5_exposure_line_h = 0, HM03D5_exposure_line_l = 0,HM03D5_extra_exposure_line_h = 0, HM03D5_extra_exposure_line_l = 0;

static kal_bool HM03D5_gPVmode = KAL_TRUE; //PV size or Full size
static kal_bool HM03D5_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool HM03D5_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint16 HM03D5_dummy_pixels=0, HM03D5_dummy_lines=0;

static kal_uint16 HM03D5_exposure_lines=0, HM03D5_extra_exposure_lines = 0;


static kal_int8 HM03D5_DELAY_AFTER_PREVIEW = -1;

static kal_uint8 HM03D5_Banding_setting = AE_FLICKER_MODE_50HZ;  //Wonder add

/****** OVT 6-18******/
static kal_uint16 HM03D5_Capture_Max_Gain16= 6*16;
static kal_uint16 HM03D5_Capture_Gain16=0 ;    
static kal_uint16 HM03D5_Capture_Shutter=0;
static kal_uint16 HM03D5_Capture_Extra_Lines=0;

static kal_uint16  HM03D5_PV_Dummy_Pixels =0, HM03D5_Capture_Dummy_Pixels =0, HM03D5_Capture_Dummy_Lines =0;
static kal_uint16  HM03D5_PV_Gain16 = 0;
static kal_uint16  HM03D5_PV_Shutter = 0;
static kal_uint16  HM03D5_PV_Extra_Lines = 0;

kal_uint16 HM03D5_sensor_gain_base=0,HM03D5_FAC_SENSOR_REG=0,HM03D5_iHM03D5_Mode=0,HM03D5_max_exposure_lines=0;
kal_uint32 HM03D5_capture_pclk_in_M=480,HM03D5_preview_pclk_in_M=480,HM03D5_PV_dummy_pixels=0,HM03D5_PV_dummy_lines=0,HM03D5_isp_master_clock=0;

static kal_uint32  HM03D5_sensor_pclk=480;	//390

kal_uint32 HM03D5_pv_HM03D5_exposure_lines = 0x0249f0,HM03D5_cp_HM03D5_exposure_lines=0;
kal_uint8 HM03D5_HV_Mirror;
kal_uint8 HM03D5_Sleep_Mode;

UINT8 HM03D5_PixelClockDivider=0;

//SENSOR_REG_STRUCT HM03D5SensorCCT[FACTORY_END_ADDR]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
//SENSOR_REG_STRUCT HM03D5SensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
//	camera_para.SENSOR.cct	SensorCCT	=> SensorCCT
//	camera_para.SENSOR.reg	SensorReg
MSDK_SENSOR_CONFIG_STRUCT HM03D5SensorConfigData;
 

void HM03D5_set_dummy(kal_uint16 pixels, kal_uint16 lines)
{
}    /* HM03D5_set_dummy */

kal_uint16 HM03D5_read_HM03D5_gain(void)
{
	return 0;
}  /* HM03D5_read_HM03D5_gain */


void HM03D5_write_HM03D5_gain(kal_uint16 gain)
{
}  /* HM03D5_write_HM03D5_gain */

void HM03D5_set_isp_driving_current(kal_uint8 current)
{
}



/*************************************************************************
* FUNCTION
*	HM03D5_night_mode
*
* DESCRIPTION
*	This function night mode of HM03D5.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void HM03D5_night_mode(kal_bool enable)
{
	if (enable) 		/* Night Mode */
	{
		/* HM03D5 night mode enable. */
		if (HM03D5_VEDIO_encode_mode == KAL_TRUE)	/* Video */
		{
			HM03D5_write_cmos_sensor(0x038F,0x0A);	//Max 5 FPS.
			HM03D5_write_cmos_sensor(0x0390,0x00);
		}
		else 										/* Camera */
		{
			HM03D5_write_cmos_sensor(0x038F,0x0A);	//Max 5 FPS.
			HM03D5_write_cmos_sensor(0x0390,0x00);
		}			
    
		HM03D5_write_cmos_sensor(0x02E0,0x00);	// 00 for Night Mode, By Brandon/20110208
		HM03D5_write_cmos_sensor(0x0481,0x06);	// 06 for Night Mode, By Brandon/20110208
		HM03D5_write_cmos_sensor(0x04B1,0x88);	// 88 for Night Mode, By Brandon/20110208
		HM03D5_write_cmos_sensor(0x04B4,0x20);	// 20 for Night Mode, By Brandon/20110208
		HM03D5_write_cmos_sensor(0x0000,0xFF);
		HM03D5_write_cmos_sensor(0x0100,0xFF);  
	}
	else  				/* Normal Mode */
	{
		/* HM03D5 night mode disable. */
		if (HM03D5_VEDIO_encode_mode == KAL_TRUE)	/* Video */
		{
			HM03D5_write_cmos_sensor(0x038F,0x05);	//Max 10 FPS.
			HM03D5_write_cmos_sensor(0x0390,0x00);
		}
		else										/* Camera */
		{
    		HM03D5_write_cmos_sensor(0x038F,0x05);	//Max 10 FPS.
			HM03D5_write_cmos_sensor(0x0390,0x00);
		}		
		
		HM03D5_write_cmos_sensor(0x02E0,0x02);	//06->02, By Brandon/20110129
		HM03D5_write_cmos_sensor(0x0481,0x08);	//06->08, By Brandon/20110129
		HM03D5_write_cmos_sensor(0x04B1,0x00);
		HM03D5_write_cmos_sensor(0x04B4,0x00);
		HM03D5_write_cmos_sensor(0x0000,0xFF);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
	}

}	/* HM03D5_night_mode */

/*************************************************************************
* FUNCTION
*	HM03D5_set_mirror
*
* DESCRIPTION
*	This function mirror, flip or mirror & flip the sensor output image.
*
*	IMPORTANT NOTICE: For some sensor, it need re-set the output order Y1CbY2Cr after
*	mirror or flip.
*
* PARAMETERS
*	1. kal_uint16 : horizontal mirror or vertical flip direction.
*
* RETURNS
*	None
*
*************************************************************************/
static void HM03D5_set_mirror(kal_uint16 image_mirror)
{
	
	switch (image_mirror)
	{
	case IMAGE_NORMAL:
		HM03D5_write_cmos_sensor(0x0006, 0x00);     //0x02
		HM03D5_write_cmos_sensor(0x0000, 0x01);   

		break;
	case IMAGE_H_MIRROR:
		HM03D5_write_cmos_sensor(0x0006, 0x02);     
		HM03D5_write_cmos_sensor(0x0000, 0x01);  

		break;
	case IMAGE_V_MIRROR:
		HM03D5_write_cmos_sensor(0x0006, 0x01);     
		HM03D5_write_cmos_sensor(0x0000, 0x01);  

		break;
	case IMAGE_HV_MIRROR:
		HM03D5_write_cmos_sensor(0x0006, 0x03);    //0x01    
		HM03D5_write_cmos_sensor(0x0000, 0x01); 

		break;
	default:
		ASSERT(0);
	}
}

/*************************************************************************
* FUNCTION
*	HM03D5_awb_enable
*
* DESCRIPTION
*	This function enable or disable the awb (Auto White Balance).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable awb, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/
static kal_bool HM03D5_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;
	
	temp_AWB_reg = HM03D5_read_cmos_sensor(0x0380);
	
	if (enalbe)
	{
		HM03D5_write_cmos_sensor(0x0380, (temp_AWB_reg | 0x02));
	}
	else
	{
		HM03D5_write_cmos_sensor(0x0380, (temp_AWB_reg& ~0x02));		
	}

	HM03D5_write_cmos_sensor(0x0000,0xFF);	
	HM03D5_write_cmos_sensor(0x0100,0xFF);
	HM03D5_write_cmos_sensor(0x0101,0xFF);
	
	return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*	HM03D5_ae_enable
*
* DESCRIPTION
*	This function enable or disable the ae (Auto Exposure).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable ae, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/
static kal_bool HM03D5_ae_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AE_reg = 0;


	temp_AE_reg = HM03D5_read_cmos_sensor(0x0380);
	
	if (enalbe)	  
	{
		HM03D5_write_cmos_sensor(0x0380, (temp_AE_reg| 0x01));	/* Turn ON AEC/AGC*/
	}
	else
	{
		HM03D5_write_cmos_sensor(0x0380, (temp_AE_reg&(~0x01))); /* Turn OFF AEC/AGC*/
	}

	HM03D5_write_cmos_sensor(0x0000,0xFF);	
	HM03D5_write_cmos_sensor(0x0100,0xFF);
	HM03D5_write_cmos_sensor(0x0101,0xFF);

	return KAL_TRUE;
}

/*************************************************************************
* FUNCTION
*	HM03D5_YUV_sensor_initial_setting
*
* DESCRIPTION
*	This function apply all of the initial setting to sensor.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
*************************************************************************/
static void HM03D5_YUV_sensor_initial_setting(void)
{
		/* HM03D5 initial setting date: 2010.12.28. 
			 2010.12.30 updated by Troy
		*/
		SENSORDB("HM03D5 tuning parameter version: 140505\n");

		HM03D5_write_cmos_sensor(0x0025,0x80);
		HM03D5_write_cmos_sensor(0x0024,0x40);
		HM03D5_write_cmos_sensor(0x0027,0x18);
		HM03D5_write_cmos_sensor(0x0040,0x0F);
		HM03D5_write_cmos_sensor(0x0053,0x0F);
		HM03D5_write_cmos_sensor(0x0044,0x06);
		HM03D5_write_cmos_sensor(0x0046,0xD8);
		HM03D5_write_cmos_sensor(0x0055,0x41);
		HM03D5_write_cmos_sensor(0x004A,0x04);
		HM03D5_write_cmos_sensor(0x0026,0x07);
		HM03D5_write_cmos_sensor(0x002a,0x1f);
		HM03D5_write_cmos_sensor(0x008e,0x80);
		HM03D5_write_cmos_sensor(0x0028,0x00);
		HM03D5_write_cmos_sensor(0x0088,0xCD);
		HM03D5_write_cmos_sensor(0x0090,0x00);
		HM03D5_write_cmos_sensor(0x0091,0x10);
		HM03D5_write_cmos_sensor(0x0092,0x11);
		HM03D5_write_cmos_sensor(0x0093,0x12);
		HM03D5_write_cmos_sensor(0x0094,0x13);
		HM03D5_write_cmos_sensor(0x0095,0x17);
		HM03D5_write_cmos_sensor(0x0086,0x84);
		HM03D5_write_cmos_sensor(0x0080,0x80);
		HM03D5_write_cmos_sensor(0x011F,0x54);
		HM03D5_write_cmos_sensor(0x0121,0x80);
		HM03D5_write_cmos_sensor(0x0122,0x6B);
		HM03D5_write_cmos_sensor(0x0123,0xAC);
		HM03D5_write_cmos_sensor(0x0124,0xD2);
		HM03D5_write_cmos_sensor(0x0125,0xDF);
		HM03D5_write_cmos_sensor(0x0126,0x71);
		HM03D5_write_cmos_sensor(0x0140,0x14);
		HM03D5_write_cmos_sensor(0x0141,0x0A);
		HM03D5_write_cmos_sensor(0x0142,0x14);
		HM03D5_write_cmos_sensor(0x0143,0x0A);
		HM03D5_write_cmos_sensor(0x0144,0x08);
		HM03D5_write_cmos_sensor(0x0145,0x00);
		HM03D5_write_cmos_sensor(0x0146,0xF3);
		HM03D5_write_cmos_sensor(0x0148,0x50);
		HM03D5_write_cmos_sensor(0x0149,0x0C);
		HM03D5_write_cmos_sensor(0x014A,0x60);
		HM03D5_write_cmos_sensor(0x014B,0x28);
		HM03D5_write_cmos_sensor(0x014C,0x20);
		HM03D5_write_cmos_sensor(0x014D,0x2E);
		HM03D5_write_cmos_sensor(0x014E,0x05);
		HM03D5_write_cmos_sensor(0x014F,0x0A);
		HM03D5_write_cmos_sensor(0x0150,0x0D);
		HM03D5_write_cmos_sensor(0x0155,0x00);
		HM03D5_write_cmos_sensor(0x0156,0x0A);
		HM03D5_write_cmos_sensor(0x0157,0x0A);
		HM03D5_write_cmos_sensor(0x0158,0x0A);
		HM03D5_write_cmos_sensor(0x0159,0x0A);
		HM03D5_write_cmos_sensor(0x015A,0x0C);
		HM03D5_write_cmos_sensor(0x015B,0x0C);
		HM03D5_write_cmos_sensor(0x0160,0x14);
		HM03D5_write_cmos_sensor(0x0161,0x14);
		HM03D5_write_cmos_sensor(0x0162,0x28);
		HM03D5_write_cmos_sensor(0x0163,0x28);
		HM03D5_write_cmos_sensor(0x0164,0x0A);
		HM03D5_write_cmos_sensor(0x0165,0x0C);
		HM03D5_write_cmos_sensor(0x0166,0x0A);
		HM03D5_write_cmos_sensor(0x0167,0x0C);
		HM03D5_write_cmos_sensor(0x0168,0x04);
		HM03D5_write_cmos_sensor(0x0169,0x08);
		HM03D5_write_cmos_sensor(0x016A,0x04);
		HM03D5_write_cmos_sensor(0x016B,0x08);
		HM03D5_write_cmos_sensor(0x01B0,0x33);
		HM03D5_write_cmos_sensor(0x01B1,0x10);
		HM03D5_write_cmos_sensor(0x01B2,0x10);
		HM03D5_write_cmos_sensor(0x01B3,0x0C);
		HM03D5_write_cmos_sensor(0x01B4,0x0A);
		HM03D5_write_cmos_sensor(0x01D8,0x0F); //Bayer Denoise
		HM03D5_write_cmos_sensor(0x01DE,0x0F);
        HM03D5_write_cmos_sensor(0x01E4,0x08);
		HM03D5_write_cmos_sensor(0x01E5,0x10);
		HM03D5_write_cmos_sensor(0x0220,0x10); //LSC
		HM03D5_write_cmos_sensor(0x0221,0xCC);
		HM03D5_write_cmos_sensor(0x0222,0x80);
		HM03D5_write_cmos_sensor(0x0223,0x88);
		HM03D5_write_cmos_sensor(0x0224,0x75);
		HM03D5_write_cmos_sensor(0x0225,0x00);
		HM03D5_write_cmos_sensor(0x0226,0x80);
		HM03D5_write_cmos_sensor(0x022A,0x6A);
		HM03D5_write_cmos_sensor(0x022B,0x00);
		HM03D5_write_cmos_sensor(0x022C,0x80);
		HM03D5_write_cmos_sensor(0x022D,0x10);
		HM03D5_write_cmos_sensor(0x022E,0x09);
		HM03D5_write_cmos_sensor(0x022F,0x10);
		HM03D5_write_cmos_sensor(0x0230,0x10);
		HM03D5_write_cmos_sensor(0x0233,0x10);
		HM03D5_write_cmos_sensor(0x0234,0x10);
		HM03D5_write_cmos_sensor(0x0235,0x40);
		HM03D5_write_cmos_sensor(0x0236,0x01);
		HM03D5_write_cmos_sensor(0x0237,0x40);
		HM03D5_write_cmos_sensor(0x0238,0x01);
		HM03D5_write_cmos_sensor(0x023B,0x40);
		HM03D5_write_cmos_sensor(0x023C,0x01);
		HM03D5_write_cmos_sensor(0x023D,0x12);
		HM03D5_write_cmos_sensor(0x023E,0x01);
		HM03D5_write_cmos_sensor(0x023F,0x0A);
		HM03D5_write_cmos_sensor(0x0240,0x01);
		HM03D5_write_cmos_sensor(0x0243,0x0A);
		HM03D5_write_cmos_sensor(0x0244,0x01);
		HM03D5_write_cmos_sensor(0x0251,0x0D);
		HM03D5_write_cmos_sensor(0x0252,0x08);
		HM03D5_write_cmos_sensor(0x0280,0x0A); //Gamma
		HM03D5_write_cmos_sensor(0x0282,0x14);
		HM03D5_write_cmos_sensor(0x0284,0x26);
		HM03D5_write_cmos_sensor(0x0286,0x4A);
		HM03D5_write_cmos_sensor(0x0288,0x5A);
		HM03D5_write_cmos_sensor(0x028A,0x67);
		HM03D5_write_cmos_sensor(0x028C,0x73);
		HM03D5_write_cmos_sensor(0x028E,0x7D);
		HM03D5_write_cmos_sensor(0x0290,0x86);
		HM03D5_write_cmos_sensor(0x0292,0x8E);
		HM03D5_write_cmos_sensor(0x0294,0x9A);
		HM03D5_write_cmos_sensor(0x0296,0xA6);
		HM03D5_write_cmos_sensor(0x0298,0xBB);
		HM03D5_write_cmos_sensor(0x029A,0xCF);
		HM03D5_write_cmos_sensor(0x029C,0xE2);
		HM03D5_write_cmos_sensor(0x029E,0x26);
		HM03D5_write_cmos_sensor(0x02A0,0x04);
		HM03D5_write_cmos_sensor(0x02C0,0xB1);
		HM03D5_write_cmos_sensor(0x02C1,0x01);
		HM03D5_write_cmos_sensor(0x02C2,0x7D);
		HM03D5_write_cmos_sensor(0x02C3,0x07);
		HM03D5_write_cmos_sensor(0x02C4,0xD2);
		HM03D5_write_cmos_sensor(0x02C5,0x07);
		HM03D5_write_cmos_sensor(0x02C6,0xC4);
		HM03D5_write_cmos_sensor(0x02C7,0x07);
		HM03D5_write_cmos_sensor(0x02C8,0x79);
		HM03D5_write_cmos_sensor(0x02C9,0x01);
		HM03D5_write_cmos_sensor(0x02CA,0xC4);
		HM03D5_write_cmos_sensor(0x02CB,0x07);
		HM03D5_write_cmos_sensor(0x02CC,0xF7);
		HM03D5_write_cmos_sensor(0x02CD,0x07);
		HM03D5_write_cmos_sensor(0x02CE,0x3B);
		HM03D5_write_cmos_sensor(0x02CF,0x07);
		HM03D5_write_cmos_sensor(0x02D0,0xCF);
		HM03D5_write_cmos_sensor(0x02D1,0x01);
		HM03D5_write_cmos_sensor(0x0302,0x00);
		HM03D5_write_cmos_sensor(0x0303,0x00);
		HM03D5_write_cmos_sensor(0x0304,0x00);
		HM03D5_write_cmos_sensor(0x02E0,0x04);
		HM03D5_write_cmos_sensor(0x02F0,0x97); //A CCM
		HM03D5_write_cmos_sensor(0x02F1,0x07);
		HM03D5_write_cmos_sensor(0x02F2,0xA9);
		HM03D5_write_cmos_sensor(0x02F3,0x00);
        HM03D5_write_cmos_sensor(0x02F4,0xC1);
		HM03D5_write_cmos_sensor(0x02F5,0x07);
        HM03D5_write_cmos_sensor(0x02F6,0xDF);
		HM03D5_write_cmos_sensor(0x02F7,0x07);
        HM03D5_write_cmos_sensor(0x02F8,0x3D);
		HM03D5_write_cmos_sensor(0x02F9,0x00);
        HM03D5_write_cmos_sensor(0x02FA,0xE4);
		HM03D5_write_cmos_sensor(0x02FB,0x07);
		HM03D5_write_cmos_sensor(0x02FC,0xB2);
		HM03D5_write_cmos_sensor(0x02FD,0x07);
        HM03D5_write_cmos_sensor(0x02FE,0x59);
		HM03D5_write_cmos_sensor(0x02FF,0x00);
        HM03D5_write_cmos_sensor(0x0300,0xF5);
        HM03D5_write_cmos_sensor(0x0301,0x07);
		HM03D5_write_cmos_sensor(0x0305,0x00);
		HM03D5_write_cmos_sensor(0x0306,0x00);
		HM03D5_write_cmos_sensor(0x0307,0x7A);
		HM03D5_write_cmos_sensor(0x0328,0x00);
		HM03D5_write_cmos_sensor(0x0329,0x04);
		HM03D5_write_cmos_sensor(0x032D,0x66);
		HM03D5_write_cmos_sensor(0x032E,0x01);
		HM03D5_write_cmos_sensor(0x032F,0x00);
		HM03D5_write_cmos_sensor(0x0330,0x01);
		HM03D5_write_cmos_sensor(0x0331,0x66);
		HM03D5_write_cmos_sensor(0x0332,0x01);
		HM03D5_write_cmos_sensor(0x0333,0x00);
		HM03D5_write_cmos_sensor(0x0334,0x00);
		HM03D5_write_cmos_sensor(0x0335,0x84);
		HM03D5_write_cmos_sensor(0x033E,0x00);
		HM03D5_write_cmos_sensor(0x033F,0x00);
		HM03D5_write_cmos_sensor(0x0340,0x38);  //AWB
		HM03D5_write_cmos_sensor(0x0341,0x88);
		HM03D5_write_cmos_sensor(0x0342,0x4A);
		HM03D5_write_cmos_sensor(0x0343,0x40);
		HM03D5_write_cmos_sensor(0x0344,0x6E);
		HM03D5_write_cmos_sensor(0x0345,0x50);
		HM03D5_write_cmos_sensor(0x0346,0x64);
		HM03D5_write_cmos_sensor(0x0347,0x58);
		HM03D5_write_cmos_sensor(0x0348,0x5A);
		HM03D5_write_cmos_sensor(0x0349,0x63);
		HM03D5_write_cmos_sensor(0x034A,0x54);
		HM03D5_write_cmos_sensor(0x034B,0x6D);
		HM03D5_write_cmos_sensor(0x034C,0x4C);
		HM03D5_write_cmos_sensor(0x0350,0x68);
		HM03D5_write_cmos_sensor(0x0351,0x90);
		HM03D5_write_cmos_sensor(0x0352,0x08);
		HM03D5_write_cmos_sensor(0x0353,0x18);
		HM03D5_write_cmos_sensor(0x0354,0x73);
		HM03D5_write_cmos_sensor(0x0355,0x45);
		HM03D5_write_cmos_sensor(0x0356,0x80); //B Max
		HM03D5_write_cmos_sensor(0x0357,0xD0);
		HM03D5_write_cmos_sensor(0x0358,0x05);
		HM03D5_write_cmos_sensor(0x035A,0x05);
		HM03D5_write_cmos_sensor(0x035B,0xA0);
		HM03D5_write_cmos_sensor(0x0381,0x54);
		HM03D5_write_cmos_sensor(0x0382,0x3C);
		HM03D5_write_cmos_sensor(0x0383,0x20);
		HM03D5_write_cmos_sensor(0x038A,0x80);
		HM03D5_write_cmos_sensor(0x038B,0x0C);
		HM03D5_write_cmos_sensor(0x038C,0xC1);
		HM03D5_write_cmos_sensor(0x038E,0x48);
		HM03D5_write_cmos_sensor(0x038F,0x05);
		HM03D5_write_cmos_sensor(0x0390,0xF4);
		HM03D5_write_cmos_sensor(0x0391,0x10);
		HM03D5_write_cmos_sensor(0x0393,0x80);
		HM03D5_write_cmos_sensor(0x0395,0x12);
		HM03D5_write_cmos_sensor(0x0398,0x01);
		HM03D5_write_cmos_sensor(0x0399,0xF0);
		HM03D5_write_cmos_sensor(0x039A,0x03);
		HM03D5_write_cmos_sensor(0x039B,0x00);
		HM03D5_write_cmos_sensor(0x039C,0x04);
		HM03D5_write_cmos_sensor(0x039D,0x00);
		HM03D5_write_cmos_sensor(0x039E,0x06);
		HM03D5_write_cmos_sensor(0x039F,0x00);
		HM03D5_write_cmos_sensor(0x03A0,0x09);
		HM03D5_write_cmos_sensor(0x03A1,0x50);
		HM03D5_write_cmos_sensor(0x03A6,0x10);
		HM03D5_write_cmos_sensor(0x03A7,0x10);
		HM03D5_write_cmos_sensor(0x03A8,0x36);
		HM03D5_write_cmos_sensor(0x03A9,0x40);
		HM03D5_write_cmos_sensor(0x03AE,0x35);
		HM03D5_write_cmos_sensor(0x03AF,0x2B);
		HM03D5_write_cmos_sensor(0x03B0,0x0C);
		HM03D5_write_cmos_sensor(0x03B1,0x0A);
		HM03D5_write_cmos_sensor(0x03B3,0x00);
		HM03D5_write_cmos_sensor(0x03B5,0x08);
		HM03D5_write_cmos_sensor(0x03B7,0xA0);
		HM03D5_write_cmos_sensor(0x03B9,0xD0);
		HM03D5_write_cmos_sensor(0x03BB,0xFF);
		HM03D5_write_cmos_sensor(0x03BC,0xFF);
		HM03D5_write_cmos_sensor(0x03BE,0x04);
		HM03D5_write_cmos_sensor(0x03BF,0x1D);
		HM03D5_write_cmos_sensor(0x03C0,0x2E);
		HM03D5_write_cmos_sensor(0x03C3,0x0F);
		HM03D5_write_cmos_sensor(0x03D0,0xE0);
		HM03D5_write_cmos_sensor(0x0420,0x86); //BLC Offset
		HM03D5_write_cmos_sensor(0x0421,0x00);
		HM03D5_write_cmos_sensor(0x0422,0x00);
        HM03D5_write_cmos_sensor(0x0423,0x82);
		HM03D5_write_cmos_sensor(0x0430,0x10); //ABLC
		HM03D5_write_cmos_sensor(0x0431,0x68);
		HM03D5_write_cmos_sensor(0x0432,0x38); //ABLC
		HM03D5_write_cmos_sensor(0x0433,0x30);
		HM03D5_write_cmos_sensor(0x0434,0x00);
		HM03D5_write_cmos_sensor(0x0435,0x40);
		HM03D5_write_cmos_sensor(0x0436,0x00);
		HM03D5_write_cmos_sensor(0x0450,0xFF);
		HM03D5_write_cmos_sensor(0x0451,0xFF);
		HM03D5_write_cmos_sensor(0x0452,0xC0);
		HM03D5_write_cmos_sensor(0x0453,0x70);
		HM03D5_write_cmos_sensor(0x0454,0x00);
		HM03D5_write_cmos_sensor(0x045A,0x00);
		HM03D5_write_cmos_sensor(0x045B,0x10);
		HM03D5_write_cmos_sensor(0x045C,0x00);
		HM03D5_write_cmos_sensor(0x045D,0xA0);
		HM03D5_write_cmos_sensor(0x0465,0x02);
		HM03D5_write_cmos_sensor(0x0466,0x34);
		HM03D5_write_cmos_sensor(0x047A,0x00);
		HM03D5_write_cmos_sensor(0x047B,0x00);
		HM03D5_write_cmos_sensor(0x0480,0x56);
		HM03D5_write_cmos_sensor(0x0481,0x06);
        HM03D5_write_cmos_sensor(0x04B0,0x56);
        HM03D5_write_cmos_sensor(0x04B6,0x30);
        HM03D5_write_cmos_sensor(0x04B9,0x0A);
		HM03D5_write_cmos_sensor(0x04B3,0x08);
		HM03D5_write_cmos_sensor(0x04B1,0x84);
		HM03D5_write_cmos_sensor(0x04B4,0x00);
		HM03D5_write_cmos_sensor(0x0540,0x00);
		HM03D5_write_cmos_sensor(0x0541,0x7F);
		HM03D5_write_cmos_sensor(0x0542,0x00);
		HM03D5_write_cmos_sensor(0x0543,0x98);
		HM03D5_write_cmos_sensor(0x0580,0x04);
		HM03D5_write_cmos_sensor(0x0581,0x08);
		HM03D5_write_cmos_sensor(0x0590,0x20);
		HM03D5_write_cmos_sensor(0x0591,0x30);
		HM03D5_write_cmos_sensor(0x0594,0x02);
		HM03D5_write_cmos_sensor(0x0595,0x08);
        HM03D5_write_cmos_sensor(0x05A0,0x05);
		HM03D5_write_cmos_sensor(0x05A1,0x14);
        HM03D5_write_cmos_sensor(0x05A2,0x35);
        HM03D5_write_cmos_sensor(0x05A3,0x32);
        HM03D5_write_cmos_sensor(0x05A5,0x25);
        HM03D5_write_cmos_sensor(0x05A6,0x22);
        HM03D5_write_cmos_sensor(0x05B0,0x36);
		HM03D5_write_cmos_sensor(0x05B1,0x04);
        HM03D5_write_cmos_sensor(0x05B2,0x1F);
		HM03D5_write_cmos_sensor(0x05B3,0x04);
		HM03D5_write_cmos_sensor(0x05D0,0x10);
		HM03D5_write_cmos_sensor(0x05D1,0x06);
		HM03D5_write_cmos_sensor(0x05E4,0x04);
		HM03D5_write_cmos_sensor(0x05E5,0x00);
		HM03D5_write_cmos_sensor(0x05E6,0x83);
		HM03D5_write_cmos_sensor(0x05E7,0x02);
		HM03D5_write_cmos_sensor(0x05E8,0x07);
		HM03D5_write_cmos_sensor(0x05E9,0x00);
		HM03D5_write_cmos_sensor(0x05EA,0xE6);
		HM03D5_write_cmos_sensor(0x05EB,0x01);
		/*
		HM03D5_write_cmos_sensor(0x0b20,0x9E);
		HM03D5_write_cmos_sensor(0x007C,0x02);
		HM03D5_write_cmos_sensor(0x007D,0x01);
		*/
		HM03D5_write_cmos_sensor(0x0b20,0xBE);
		HM03D5_write_cmos_sensor(0x007C,0x07);
		HM03D5_write_cmos_sensor(0x007D,0x3C);
		

		/* CMU */
		HM03D5_write_cmos_sensor(0x0000,0x01);
		HM03D5_write_cmos_sensor(0x0100,0x01);
		HM03D5_write_cmos_sensor(0x0101,0x01);
		HM03D5_write_cmos_sensor(0x0005,0x01);		

} /* HM03D5_YUV_sensor_initial_setting */

/*************************************************************************
* FUNCTION
*	HM03D5_PV_setting
*
* DESCRIPTION
*	This function apply the preview mode setting, normal the preview size is 1/4 of full size.
*	Ex. 2M (1600 x 1200)
*	Preview: 800 x 600 (Use sub-sample or binning to acheive it)
*	Full Size: 1600 x 1200 (Output every effective pixels.)
*
* PARAMETERS
*	1. image_sensor_exposure_window_struct : Set the grab start x,y and width,height.
*	2. image_sensor_config_struct : Current operation mode.
*
* RETURNS
*	None
*
*************************************************************************/
static void HM03D5_PV_setting()
{
		/* Preview mode parameters */
	msleep(100);
	HM03D5_write_cmos_sensor(0x0000,0x01);
	HM03D5_write_cmos_sensor(0x0100,0x01);
	HM03D5_write_cmos_sensor(0x0101,0x01);	

	//Start
	HM03D5_write_cmos_sensor(0x0000,0x01); 
	HM03D5_write_cmos_sensor(0x0100,0x01); 
	HM03D5_write_cmos_sensor(0x0101,0x01); 
	HM03D5_write_cmos_sensor(0x0005,0x01); //Turn on rolling shutter
	
} /* HM03D5_PV_setting */

/*************************************************************************
* FUNCTION
*	HM03D5_Set_Video_Frame_Rate
*
* DESCRIPTION
*	This function set the sensor output frmae to target frame and fix the frame rate for 
*	video encode.
*
* PARAMETERS
*	1. kal_uint32 : Target frame rate to fixed.
*
* RETURNS
*	None
*
*************************************************************************/
static void HM03D5_Set_Video_Frame_Rate(kal_uint32 frame_rate)
{
	switch (frame_rate)
	{
		case 50:
			/* Fixed frame rate in 5 FPS */
			HM03D5_write_cmos_sensor(0x038F,0x0A);	//Max 5 FPS.
    		HM03D5_write_cmos_sensor(0x0390,0x00);           
			break;
		case 100:
			/* Fixed frame rate in 10 FPS */
			HM03D5_write_cmos_sensor(0x038F,0x05);	//Max 10 FPS.
    		HM03D5_write_cmos_sensor(0x0390,0x00);          
			break;
		case 120:
				/* Fixed frame rate in 15 FPS */
			HM03D5_write_cmos_sensor(0x038F,0x04);	//Max 12 FPS.
    		HM03D5_write_cmos_sensor(0x0390,0x00);           
			break;
		case 200:
				/* Fixed frame rate in 30 FPS */
			HM03D5_write_cmos_sensor(0x038F,0x03);	//Max 20 FPS.
    		HM03D5_write_cmos_sensor(0x0390,0x00);          
			break;
		default:
				/* Do nothing */
			break;
	}
	
	HM03D5_write_cmos_sensor(0x0000,0xFF);
	HM03D5_write_cmos_sensor(0x0100,0xFF);
}

/*************************************************************************
* FUNCTION
*	HM03D5_CAP_setting
*
* DESCRIPTION
*	This function apply the full size mode setting.
*	Ex. 2M (1600 x 1200)
*	Preview: 800 x 600 (Use sub-sample or binning to acheive it)
*	Full Size: 1600 x 1200 (Output every effective pixels.)
*
* PARAMETERS
*	1. image_sensor_exposure_window_struct : Set the grab start x,y and width,height.
*	2. image_sensor_config_struct : Current operation mode.
*
* RETURNS
*	None
*
*************************************************************************/
static void HM03D5_CAP_setting()
{
	kal_uint8 temp;
	
  kal_uint16 exp = 0;
  kal_uint8 exp_l = 0;
  kal_uint8 exp_h = 0;
  /*
	exp =(HM03D5_read_cmos_sensor(0x15)<<8 | HM03D5_read_cmos_sensor(0x16));//hm03d5
	exp = (exp/2);
	exp_h = (exp&0xff00)>>8;
	exp_l =  exp&0x00ff;
	HM03D5_write_cmos_sensor(0x0380,0xFE);
	HM03D5_write_cmos_sensor(0x0015,exp_h);
	HM03D5_write_cmos_sensor(0x0016,exp_l);
*/

	/* CMU */
  for (temp = 0; temp < 3; temp++)
  {		/* Troy add this loop to let capture more stable. */   			
			HM03D5_write_cmos_sensor(0x0000,0x01);
			HM03D5_write_cmos_sensor(0x0100,0x01);
			HM03D5_write_cmos_sensor(0x0101,0x01);			
}

	
} /* HM03D5_CAP_setting */

UINT32 HM03D5GetSensorID(UINT32 *sensorID)
{
	volatile signed char i;
	kal_uint16 sensor_id=0;

	HM03D5_write_cmos_sensor(0x0022, 0x01);
	mdelay(10);	

	sensor_id = (HM03D5_read_cmos_sensor(0x0001) << 8) | HM03D5_read_cmos_sensor(0x0002);


	if (HM03D5_SENSOR_ID != sensor_id)
	{
		*sensorID = 0xffffffff;
		SENSORDB("HM03D5 Sensor id read failed, ID = %x\n", sensor_id);
		return ERROR_SENSOR_CONNECT_FAIL;
	}

	*sensorID = sensor_id;

	printk("[HM03D5] sensor id = 0x%x\n", sensor_id);

	return ERROR_NONE;
}

/*************************************************************************
* FUNCTION
*	HM03D5Open
*
* DESCRIPTION
*	This function initialize the registers of CMOS sensor
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 HM03D5Open(void)
{
	volatile signed char i;
	kal_uint16 sensor_id=0;
	zoom_factor = 0; 
	//HM03D5_write_cmos_sensor(0x0022, 0x01);
	mdelay(50);	 //20121025	
	
	sensor_id = (HM03D5_read_cmos_sensor(0x0001) << 8) | HM03D5_read_cmos_sensor(0x0002);
	printk("[HM03D5] sensor id = 0x%x\n", sensor_id);
	if (HM03D5_SENSOR_ID != sensor_id)
		{
		printk("[HM03D5]  ================(HM03D5_SENSOR_ID != sensor_id)==========================\n");
		return ERROR_SENSOR_CONNECT_FAIL;
		}
	HM03D5_YUV_sensor_initial_setting();
	printk("HM03D5 Initial setting \n");     
	mdelay(400);
	return ERROR_NONE;
}	/* HM03D5Open() *///by Michael.Xie  2010/11/30

/*************************************************************************
* FUNCTION
*	HM03D5Close
*
* DESCRIPTION
*	This function is to turn off sensor module power.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 HM03D5Close(void)
{
	HM03D5_write_cmos_sensor(0x04, 0x11);
	//HM03D5_write_cmos_sensor(0x03, 0x00); //enter	page 0
	//HM03D5_write_cmos_sensor(0x01, (HM03D5_read_cmos_sensor(0x01)|0x01));
//	CISModulePowerOn(FALSE);
	return ERROR_NONE;
}	/* HM03D5Close() */

/*************************************************************************
* FUNCTION
*	HM03D5Preview
*
* DESCRIPTION
*	This function start the sensor preview.
*
* PARAMETERS
*	*image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 HM03D5Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iDummyPixels = 0, iDummyLines = 0;

	kal_uint16 temp_reg;
     
    HM03D5_sensor_cap_state = KAL_FALSE;
  
    //4  <1> preview config sequence
	HM03D5_sensor_pclk=480;
    
    HM03D5_gPVmode = KAL_TRUE;
	HM03D5_PV_setting();
	mdelay(500);
	printk("HM03D5 Preview setting is done \n");     
	temp_reg=HM03D5_read_cmos_sensor(0x0006);
       SENSORDB("HM03D5 read register, 0x0006 = %x\n", temp_reg);

	HM03D5_ae_enable(KAL_TRUE);
	HM03D5_awb_enable(KAL_TRUE);
	if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
	{
		HM03D5_VEDIO_encode_mode = KAL_TRUE;
		HM03D5_Set_Video_Frame_Rate(200);

	}
	else
	{
        HM03D5_VEDIO_encode_mode = KAL_FALSE;
        HM03D5_iHM03D5_Mode = HM03D5_MODE_PREVIEW;
		HM03D5_Set_Video_Frame_Rate(100);
	}

    //4 <3> set mirror and flip
    sensor_config_data->SensorImageMirror = IMAGE_NORMAL;
    HM03D5_set_mirror(sensor_config_data->SensorImageMirror);

    //4 <7> set shutter
    image_window->GrabStartX = IMAGE_SENSOR_START_GRAB_X;
    image_window->GrabStartY = IMAGE_SENSOR_START_GRAB_X;
    image_window->ExposureWindowWidth = HM03D5_IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight = HM03D5_IMAGE_SENSOR_PV_HEIGHT;
    
    HM03D5_DELAY_AFTER_PREVIEW = 1;

	// copy sensor_config_data
	memcpy(&HM03D5SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
  	return ERROR_NONE;
}	/* HM03D5Preview() *///by Michael.Xie  2010/11/30

UINT32 HM03D5Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{

    HM03D5_sensor_cap_state = KAL_TRUE;

	  image_window->GrabStartX=IMAGE_SENSOR_START_GRAB_X;
	  image_window->GrabStartY=IMAGE_SENSOR_START_GRAB_X;
	  image_window->ExposureWindowWidth=HM03D5_IMAGE_SENSOR_FULL_WIDTH;
	  image_window->ExposureWindowHeight=HM03D5_IMAGE_SENSOR_FULL_HEIGHT;


    // AEC/AGC/AWB will be enable in preview and param_wb function
    /* total delay 4 frame for AE stable */
		HM03D5_gPVmode=KAL_TRUE;

		HM03D5_DELAY_AFTER_PREVIEW = 2;

	// copy sensor_config_data
	memcpy(&HM03D5SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* HM03D5Capture() */

UINT32 HM03D5GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth=HM03D5_IMAGE_SENSOR_FULL_WIDTH;  //modify by yanxu
	pSensorResolution->SensorFullHeight=HM03D5_IMAGE_SENSOR_FULL_HEIGHT;
	pSensorResolution->SensorPreviewWidth=HM03D5_IMAGE_SENSOR_PV_WIDTH ;
	pSensorResolution->SensorPreviewHeight=HM03D5_IMAGE_SENSOR_PV_HEIGHT;

	pSensorResolution->SensorVideoWidth=HM03D5_IMAGE_SENSOR_PV_WIDTH - 2*IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorVideoHeight=HM03D5_IMAGE_SENSOR_PV_HEIGHT-2*IMAGE_SENSOR_START_GRAB_Y;

	return ERROR_NONE;
}	/* HM03D5GetResolution() */

UINT32 HM03D5GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	pSensorInfo->SensorPreviewResolutionX=HM03D5_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=HM03D5_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=HM03D5_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=HM03D5_IMAGE_SENSOR_FULL_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=5;		// 1
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_VYUY;	
	//pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	/*??? */
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->CaptureDelayFrame = 2; 
	pSensorInfo->PreviewDelayFrame = 2; 
	pSensorInfo->VideoDelayFrame = 0; 
	pSensorInfo->SensorMasterClockSwitch = 0; 
	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;	
#if 0
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=TRUE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=TRUE;
	/*pSensorInfo->CaptureDelayFrame = 1; 
	pSensorInfo->PreviewDelayFrame = 0; 
	pSensorInfo->VideoDelayFrame = 4; 		
	pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   	*/	
#endif
   	pSensorInfo->YUVAwbDelayFrame = 2 ;
  	pSensorInfo->YUVEffectDelayFrame = 2 ;

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=24;//24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;	//4// 4
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_START_GRAB_X; 
			pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_START_GRAB_X;
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
      pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	    pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	    pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;             
			pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			pSensorInfo->SensorPacketECCOrder = 1;

		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			pSensorInfo->SensorClockFreq=24;//24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;	// 4
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_START_GRAB_X; 
			pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_START_GRAB_X;
			pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE;			
      pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	    pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
	    pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;             
			pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
			pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
			pSensorInfo->SensorPacketECCOrder = 1;
		break;
		default:
			pSensorInfo->SensorClockFreq=24;//24;
			pSensorInfo->SensorClockDividCount=	3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 4;
			pSensorInfo->SensorDataLatchCount= 2;
			pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_START_GRAB_X; 
			pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_START_GRAB_X;             
		break;
	}
	HM03D5_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
	memcpy(pSensorConfigData, &HM03D5SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* HM03D5GetInfo() */


UINT32 HM03D5Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
		//case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			HM03D5Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
		//case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			HM03D5Capture(pImageWindow, pSensorConfigData);
		break;
		default:
		    break; 
	}
	return TRUE;
}	/* HM03D5Control() */ 

BOOL HM03D5_set_param_wb(UINT16 para)
{
printk("===========HM03D5_set_param_wb para=%d\n",para);
    switch (para)
    {
    case AWB_MODE_AUTO:
	HM03D5_write_cmos_sensor(0x0380, 0xFF);   // select Auto WB
	HM03D5_write_cmos_sensor(0x0101, 0xFF);  
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
    HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0380, 0xFD);  	//Disable AWB
	HM03D5_write_cmos_sensor(0x032D, 0x70);
	HM03D5_write_cmos_sensor(0x032E, 0x01); 	//Red
	HM03D5_write_cmos_sensor(0x032F, 0x00);
	HM03D5_write_cmos_sensor(0x0330, 0x01);		//Green
	HM03D5_write_cmos_sensor(0x0331, 0x08);
	HM03D5_write_cmos_sensor(0x0332, 0x01);		//Blue
	
	HM03D5_write_cmos_sensor(0x0000, 0xFF);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
        break;
    case AWB_MODE_DAYLIGHT: //sunny
     HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0380, 0xFD);  	//Disable AWB
	HM03D5_write_cmos_sensor(0x032D, 0x60);
	HM03D5_write_cmos_sensor(0x032E, 0x01); 	//Red
	HM03D5_write_cmos_sensor(0x032F, 0x00);
	HM03D5_write_cmos_sensor(0x0330, 0x01);		//Green
	HM03D5_write_cmos_sensor(0x0331, 0x20);
	HM03D5_write_cmos_sensor(0x0332, 0x01);		//Blue
	HM03D5_write_cmos_sensor(0x0000, 0xFF);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
        break;
    case AWB_MODE_INCANDESCENT: //office
     HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0380, 0xFD);  	//Disable AWB
	HM03D5_write_cmos_sensor(0x032D, 0x00);
	HM03D5_write_cmos_sensor(0x032E, 0x01); 	//Red
	HM03D5_write_cmos_sensor(0x032F, 0x14);
	HM03D5_write_cmos_sensor(0x0330, 0x01);		//Green
	HM03D5_write_cmos_sensor(0x0331, 0xD6);
	HM03D5_write_cmos_sensor(0x0332, 0x01);		//Blue
	HM03D5_write_cmos_sensor(0x0000, 0xFF);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
        break;
    case AWB_MODE_TUNGSTEN: //home
     HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0380, 0xFD);  	//Disable AWB
	HM03D5_write_cmos_sensor(0x032D, 0x10);
	HM03D5_write_cmos_sensor(0x032E, 0x01); 	//Red
	HM03D5_write_cmos_sensor(0x032F, 0x00);
	HM03D5_write_cmos_sensor(0x0330, 0x01);		//Green
	HM03D5_write_cmos_sensor(0x0331, 0xA0);
	HM03D5_write_cmos_sensor(0x0332, 0x01);		//Blue
	HM03D5_write_cmos_sensor(0x0000, 0xFF);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
        break;
    case AWB_MODE_FLUORESCENT:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0380, 0xFD);  	//Disable AWB
	HM03D5_write_cmos_sensor(0x032D, 0x34);
	HM03D5_write_cmos_sensor(0x032E, 0x01); 	//Red
	HM03D5_write_cmos_sensor(0x032F, 0x00);
	HM03D5_write_cmos_sensor(0x0330, 0x01);		//Green
	HM03D5_write_cmos_sensor(0x0331, 0x92);
	HM03D5_write_cmos_sensor(0x0332, 0x01);		//Blue
	HM03D5_write_cmos_sensor(0x0000, 0xFF);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
        break; 
    default:
        return FALSE;
    }

    return TRUE;
} /* HM03D5_set_param_wb */

BOOL HM03D5_set_param_effect(UINT16 para)
{
   BOOL  ret = TRUE;
   //UINT8  temp_reg;
   //temp_reg=HM03D5_read_cmos_sensor(0x3391);
    switch (para)
    {
    case MEFFECT_OFF:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0488, 0x10); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0486, 0x00); //Hue, sin                       
	HM03D5_write_cmos_sensor(0x0487, 0xFF); //Hue, cos
	HM03D5_write_cmos_sensor(0x0120, 0x37);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
        break;
    case MEFFECT_SEPIA:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0486, 0x40); //Hue, sin						 
	HM03D5_write_cmos_sensor(0x0487, 0xA0); //Hue, cos
	HM03D5_write_cmos_sensor(0x0488, 0x11); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0120, 0x27);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
        break;
    case MEFFECT_NEGATIVE:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0488, 0x12); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0486, 0x00); //Hue, sin						 
	HM03D5_write_cmos_sensor(0x0487, 0xFF); //Hue, cos
	HM03D5_write_cmos_sensor(0x0120, 0x37);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
        break;
    case MEFFECT_SEPIAGREEN:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0486, 0x60); //Hue, sin						 
	HM03D5_write_cmos_sensor(0x0487, 0x60); //Hue, cos
	HM03D5_write_cmos_sensor(0x0488, 0x11); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0120, 0x27);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
        break;
    case MEFFECT_SEPIABLUE:
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0486, 0xB0); //Hue, sin						 
	HM03D5_write_cmos_sensor(0x0487, 0x80); //Hue, cos
	HM03D5_write_cmos_sensor(0x0488, 0x11); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0120, 0x27);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
        break;
	case MEFFECT_MONO: //B&W
	HM03D5_write_cmos_sensor(0x0005, 0x00);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
	HM03D5_write_cmos_sensor(0x0486, 0x80); //Hue, sin						 
	HM03D5_write_cmos_sensor(0x0487, 0x80); //Hue, cos
	HM03D5_write_cmos_sensor(0x0488, 0x11); //[0]:Image scense, [1]:Image Xor
	HM03D5_write_cmos_sensor(0x0120, 0x27);
	HM03D5_write_cmos_sensor(0x0005, 0x01);
	HM03D5_write_cmos_sensor(0x0000, 0x01);
	HM03D5_write_cmos_sensor(0x0100, 0xFF);
	HM03D5_write_cmos_sensor(0x0101, 0xFF);
		break; 
    default:
        ret = FALSE;
    }

    return ret;

} /* HM03D5_set_param_effect */

BOOL HM03D5_set_param_banding(UINT16 para)
{

    kal_uint8 banding;

    //banding = HM03D5_read_cmos_sensor(0x3014);
    switch (para)
    {
	case AE_FLICKER_MODE_50HZ:
		HM03D5_Banding_setting = AE_FLICKER_MODE_50HZ;
		HM03D5_write_cmos_sensor(0x0120, 0x36);	//[0]:0=>50Hz
		HM03D5_write_cmos_sensor(0x0000, 0xFF);	//AE CMU   
		HM03D5_write_cmos_sensor(0x0100, 0xFF);	//AE CMU    
		HM03D5_write_cmos_sensor(0x0101, 0xFF);	//AE CMU    
		break;
    case AE_FLICKER_MODE_60HZ:			
        HM03D5_Banding_setting = AE_FLICKER_MODE_60HZ;
		HM03D5_write_cmos_sensor(0x0120, 0x37);	//[0]:1=>60Hz
		HM03D5_write_cmos_sensor(0x0000, 0xFF);	//AE CMU   
		HM03D5_write_cmos_sensor(0x0100, 0xFF);	//AE CMU    
		HM03D5_write_cmos_sensor(0x0101, 0xFF);	//AE CMU 		 
        break;
      default:
          return FALSE;
    }

    return TRUE;
} /* HM03D5_set_param_banding */

BOOL HM03D5_set_param_exposure(UINT16 para)
{
    HM03D5_write_cmos_sensor(0x03,0x10);
	HM03D5_write_cmos_sensor(0x12,(HM03D5_read_cmos_sensor(0x12)|0x10));//make sure the Yoffset control is opened.

    switch (para)
    {
    case AE_EV_COMP_n13:
		HM03D5_write_cmos_sensor(0x04C0,0xC0);
		HM03D5_write_cmos_sensor(0x038E,0x28);	// 20-28-30
		HM03D5_write_cmos_sensor(0x0381,0x30);
		HM03D5_write_cmos_sensor(0x0382,0x20);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    case AE_EV_COMP_n10:
		HM03D5_write_cmos_sensor(0x04C0,0xB0);
		HM03D5_write_cmos_sensor(0x038E,0x30);	// 28-30-38
		HM03D5_write_cmos_sensor(0x0381,0x38);
		HM03D5_write_cmos_sensor(0x0382,0x28);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    case AE_EV_COMP_n07:
		HM03D5_write_cmos_sensor(0x04C0,0xA0);
		HM03D5_write_cmos_sensor(0x038E,0x38);  // 30-38-40
		HM03D5_write_cmos_sensor(0x0381,0x40);
		HM03D5_write_cmos_sensor(0x0382,0x30);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    case AE_EV_COMP_n03:
		HM03D5_write_cmos_sensor(0x04C0,0x90);
		HM03D5_write_cmos_sensor(0x038E,0x40);	// 38-40-48
		HM03D5_write_cmos_sensor(0x0381,0x48);
		HM03D5_write_cmos_sensor(0x0382,0x38);
		HM03D5_write_cmos_sensor(0x0100,0xFF); 
        break;
    case AE_EV_COMP_00:
		HM03D5_write_cmos_sensor(0x04C0,0x00);
		HM03D5_write_cmos_sensor(0x038E,0x48);	// 40-48-50
		HM03D5_write_cmos_sensor(0x0381,0x50);
		HM03D5_write_cmos_sensor(0x0382,0x40);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    case AE_EV_COMP_03:
		HM03D5_write_cmos_sensor(0x04C0,0x10);	
		HM03D5_write_cmos_sensor(0x038E,0x50);	// 48-50-58
		HM03D5_write_cmos_sensor(0x0381,0x58);
		HM03D5_write_cmos_sensor(0x0382,0x48);
		HM03D5_write_cmos_sensor(0x0100,0xFF); 
        break;
    case AE_EV_COMP_07:
		HM03D5_write_cmos_sensor(0x04C0,0x20);	
		HM03D5_write_cmos_sensor(0x038E,0x58);	// 50-58-60
		HM03D5_write_cmos_sensor(0x0381,0x60);
		HM03D5_write_cmos_sensor(0x0382,0x50);
		HM03D5_write_cmos_sensor(0x0100,0xFF); 
        break;
    case AE_EV_COMP_10:
		HM03D5_write_cmos_sensor(0x04C0,0x30);	
		HM03D5_write_cmos_sensor(0x038E,0x60);	// 58-60-68
		HM03D5_write_cmos_sensor(0x0381,0x68);
		HM03D5_write_cmos_sensor(0x0382,0x58);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    case AE_EV_COMP_13:
		HM03D5_write_cmos_sensor(0x04C0,0x40);
		HM03D5_write_cmos_sensor(0x038E,0x68);	// 60-68-70
		HM03D5_write_cmos_sensor(0x0381,0x70);
		HM03D5_write_cmos_sensor(0x0382,0x60);
		HM03D5_write_cmos_sensor(0x0100,0xFF);
        break;
    default:
        return FALSE;
    }

    return TRUE;
} /* HM03D5_set_param_exposure */



UINT32 HM03D5YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
//   if( HM03D5_sensor_cap_state == KAL_TRUE)
//	   return TRUE;
	
	switch (iCmd) {
	case FID_SCENE_MODE:	    
//	    printk("Set Scene Mode:%d\n", iPara); 
	    if (iPara == SCENE_MODE_OFF)
	    {
	        HM03D5_night_mode(0); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
               HM03D5_night_mode(1); 
	    }	    
	    break; 	    
	case FID_AWB_MODE:
//	    printk("Set AWB Mode:%d\n", iPara); 	    
           HM03D5_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT:
//	    printk("Set Color Effect:%d\n", iPara); 	    	    
           HM03D5_set_param_effect(iPara);
	break;
	case FID_AE_EV: 	    
//           printk("Set EV:%d\n", iPara); 	    	    
           HM03D5_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:
//           printk("Set Flicker:%d\n", iPara); 	    	    	    
           HM03D5_set_param_banding(iPara);
	break;
	case FID_ZOOM_FACTOR:
	    zoom_factor = iPara; 
	default:
	break;
	}
	return TRUE;
}   /* MT9P012YUVSensorSetting */

UINT32 HM03D5YUVSetVideoMode(UINT16 u2FrameRate)
{
    kal_uint8 iTemp;
    /* to fix VSYNC, to fix frame rate */
    //printk("Set YUV Video Mode \n");     
   // iTemp = HM03D5_read_cmos_sensor(0x3014);
    //HM03D5_write_cmos_sensor(0x3014, iTemp & 0xf7); //Disable night mode

    if (u2FrameRate == 30)
    {
        //HM03D5_write_cmos_sensor(0x302d, 0x00);
        //HM03D5_write_cmos_sensor(0x302e, 0x00);
    }
    else if (u2FrameRate == 15)       
    {
        //HM03D5_write_cmos_sensor(0x300e, 0x34);
        //HM03D5_write_cmos_sensor(0x302A, HM03D5_VIDEO_15FPS_FRAME_LENGTH>>8);  /*  15fps*/
        //HM03D5_write_cmos_sensor(0x302B, HM03D5_VIDEO_15FPS_FRAME_LENGTH&0xFF);
                
        // clear extra exposure line
   // HM03D5_write_cmos_sensor(0x302d, 0x00);
    //HM03D5_write_cmos_sensor(0x302e, 0x00);
    }
    else 
    {
        printk("Wrong frame rate setting \n");
    }
    HM03D5_VEDIO_encode_mode = KAL_TRUE; 
        
    return TRUE;
}

UINT32 HM03D5FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara; 
	
	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=HM03D5_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=HM03D5_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:
			*pFeatureReturnPara16++=HM03D5_PV_PERIOD_PIXEL_NUMS+HM03D5_PV_dummy_pixels;
			*pFeatureReturnPara16=HM03D5_PV_PERIOD_LINE_NUMS+HM03D5_PV_dummy_lines;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			*pFeatureReturnPara32 = HM03D5_sensor_pclk/10;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			HM03D5_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			HM03D5_isp_master_clock=*pFeatureData32;
		break;
		case SENSOR_FEATURE_SET_REGISTER:
			HM03D5_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = HM03D5_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &HM03D5SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		//case SENSOR_FEATURE_GET_GROUP_COUNT:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
		break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
                        *pFeatureReturnPara32++=0;
                        *pFeatureParaLen=4;	    
		    break; 
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_YUV_CMD:
//		       printk("HM03D5 YUV sensor Setting:%d, %d \n", *pFeatureData32,  *(pFeatureData32+1));
			HM03D5YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
		break; 	
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			HM03D5GetSensorID(pFeatureData32);
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       HM03D5YUVSetVideoMode(*pFeatureData16);
		       break; 
		default:
			break;			
	}
	return ERROR_NONE;
}	/* HM03D5FeatureControl() *///by Michael.Xie  2010/11/30
SENSOR_FUNCTION_STRUCT	SensorFuncHM03D5=
{
	HM03D5Open,
	HM03D5GetInfo,
	HM03D5GetResolution,
	HM03D5FeatureControl,
	HM03D5Control,
	HM03D5Close
};

UINT32 HM03D5_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncHM03D5;

	return ERROR_NONE;
}	/* SensorInit() */



