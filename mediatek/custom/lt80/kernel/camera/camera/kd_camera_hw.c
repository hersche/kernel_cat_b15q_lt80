#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC(fmt, arg...)    xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg)

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

kal_bool searchMainSensor = KAL_TRUE;

extern void ISP_MCLK1_EN(bool En);
#define GPIO_122_FOR_CMMCLK       (GPIO122 | 0x80000000)
extern int get_cci_hw_id(void); 

int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
u32 pinSetIdx = 0;//default main sensor

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3


u32 pinSet[2][8] = {
                    //for main sensor
                    {GPIO_CAMERA_CMRST_PIN,
                        GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for sub sensor
                    {GPIO_CAMERA_CMRST1_PIN,
                     GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                     GPIO_CAMERA_CMPDN1_PIN,
                        GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                   };


    if(1 == get_cci_hw_id()) //EVT
    {
        PK_DBG("kdCISModulePowerOn - Change main camera CMPDN pin to STBYN!!!\n");
        pinSet[0][IDX_PS_CMPDN] = STBYN;
    }

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
		searchMainSensor = KAL_TRUE;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
		searchMainSensor = KAL_FALSE;
    }

   
    //power ON
    if (On) {


#if 0 //TODO: depends on HW layout. Should be notified by SA.

        PK_DBG("Set CAMERA_POWER_PULL_PIN for power \n");
        if (mt_set_gpio_pull_enable(GPIO_CAMERA_LDO_EN_PIN, GPIO_PULL_DISABLE)) {PK_DBG("[[CAMERA SENSOR] Set CAMERA_POWER_PULL_PIN DISABLE ! \n"); }
        if(mt_set_gpio_mode(GPIO_CAMERA_LDO_EN_PIN, GPIO_CAMERA_LDO_EN_PIN_M_GPIO)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_CAMERA_LDO_EN_PIN,GPIO_DIR_OUT)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_CAMERA_LDO_EN_PIN,GPIO_OUT_ONE)){PK_DBG("[[CAMERA SENSOR] set CAMERA_POWER_PULL_PIN failed!! \n");}
#endif

	PK_DBG("kdCISModulePowerOn -on:currSensorName=%s\n",currSensorName);
	PK_DBG("kdCISModulePowerOn -on:pinSetIdx=%d\n",pinSetIdx);

	 if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW, currSensorName)) && (pinSetIdx == 0))
	 {
		PK_DBG("kdCISModulePowerOn get in---  SENSOR_DRVNAME_OV5648_MIPI_RAW \n");
		 //enable active sensor
		 if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			 //RST pin,active low
			 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
			 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
			 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[OV5648] set gpio failed!! \n");}
			 mdelay(10);
	 
			 //PDN pin,high
			 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
			 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
			 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[OV5648] set gpio failed!! \n");}
			 mdelay(10);
		 }
	 
		 //MCLK
		 PK_DBG("[Turn on MCLK]sensorIdx:%d \n",SensorIdx);
		 if(mt_set_gpio_mode(GPIO_122_FOR_CMMCLK, GPIO_MODE_01)){PK_DBG("[OV5648 CAMERA CMMCLK] set gpio mode failed!! \n");}
		 if(mt_set_gpio_dir(GPIO_122_FOR_CMMCLK,GPIO_DIR_OUT)){PK_DBG("[OV5648 CAMERA CMMCLK] set gpio dir failed!! \n");}
		 if(mt_set_gpio_out(GPIO_122_FOR_CMMCLK,GPIO_OUT_ZERO)){PK_DBG("[OV5648 CAMERA CMMCLK] set gpio failed!! \n");}
		 mdelay(10);
		 ISP_MCLK1_EN(TRUE); // Turn on MCLK.        
		 mdelay(20);

		 //DOVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
		 {
			 PK_DBG("[OV5648] Fail to enable CAMERA_POWER_VCAM_D2\n");
		 }
		 mdelay(10);
	 
		 //AVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
		 {
			 PK_DBG("[OV5648] Fail to enable CAMERA_POWER_VCAM_A\n");
		 }
		 mdelay(10);

		 //DVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
		 {
			  PK_DBG("[OV5648] Fail to enable CAMERA_POWER_VCAM_D\n");
		 }
		 mdelay(10);

		 //AF_VCC
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,mode_name))
		 {
			  PK_DBG("[OV5648] Fail to enable CAMERA_POWER_VCAM_A2\n");
		 }
		 mdelay(10);

		 //enable active sensor
		 if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			 //RST pin
			 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
			 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
			 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[OV5648] set gpio failed!! \n");}
			 mdelay(10);
             
			 //PDN pin
			 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
			 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
			 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[OV5648] set gpio failed!! \n");}
			 mdelay(10);
		 }
		 mdelay(20);  //delay 20ms to access I2C

		 //disable inactive sensor
		 if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
			 if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
				 PK_DBG("[OV5648] disable sub!! \n");
				 //if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
				 if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
				 //if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
				 if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
				 //if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[OV5648] set gpio failed!! \n");} //low == reset sensor
				 if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[OV5648] set gpio failed!! \n");} //high == power down lens module
			 }
		 }
		 else {
			 if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
				 PK_DBG("[OV5648] ERROR!! \n");
				 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
				 if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
				 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
				 if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
				 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[OV5648] set gpio failed!! \n");} //low == reset sensor
				 if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[OV5648] set gpio failed!! \n");} //high == power down lens module
			 }
		 }
	}
	else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_HM03D5_MIPI_YUV,currSensorName)) && (pinSetIdx == 1))
	{
		PK_DBG("kdCISModulePowerOn get in---  SENSOR_DRVNAME_HM03D5_MIPI_YUV \n");

		 //enable active sensor
		 if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
			 //PDN pin
			 if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[OV5648] set gpio mode failed!! \n");}
			 if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
			 if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[OV5648] set gpio failed!! \n");}
			 mdelay(10);
		 }

		 //DOVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D2, VOL_1800,mode_name))
		 {
			 PK_DBG("[HM03D5] Fail to enable CAMERA_POWER_VCAM_D2\n");
		 }
		 mdelay(10);
	 
		 //AVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A, VOL_2800,mode_name))
		 {
			 PK_DBG("[HM03D5] Fail to enable CAMERA_POWER_VCAM_A\n");
		 }
		 mdelay(10);

		 //DVDD
		 if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_D, VOL_1500,mode_name))
		 {
			  PK_DBG("[HM03D5] Fail to enable CAMERA_POWER_VCAM_D\n");
		 }
		 mdelay(10);

		// 3.Turn on CMMCLK
		if(mt_set_gpio_mode(GPIO_122_FOR_CMMCLK, GPIO_MODE_01)){PK_DBG("[HM03D5 CAMERA CMMCLK] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(GPIO_122_FOR_CMMCLK,GPIO_DIR_OUT)){PK_DBG("[HM03D5 CAMERA CMMCLK] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(GPIO_122_FOR_CMMCLK,GPIO_OUT_ZERO)){PK_DBG("[HM03D5 CAMERA CMMCLK] set gpio failed!! \n");}
		mdelay(10);
		ISP_MCLK1_EN(TRUE); // Turn on MCLK.
		mdelay(20);

		//PDN/STBY pin
		if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST])
		{
			PK_DBG("kdCISModulePowerOn get in---  SENSOR_DRVNAME_HM03D5_MIPI_YUV -init set pdn \n");
			
			if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
			if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
			if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[HM03D5] set gpio failed!! \n");}
			mdelay(5);
		}


        //disable inactive sensor
        if(pinSetIdx == 0 || pinSetIdx == 2) {//disable sub
	        if (GPIO_CAMERA_INVALID != pinSet[1][IDX_PS_CMRST]) {
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[1][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMRST],pinSet[1][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[HM03D5] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[1][IDX_PS_CMPDN],pinSet[1][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[HM03D5] set gpio failed!! \n");} //high == power down lens module
	        }
        }
		else {
	        if (GPIO_CAMERA_INVALID != pinSet[0][IDX_PS_CMRST]) {
				PK_DBG("kdCISModulePowerOn 8AA close index[0]\n");
				
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
	            if(mt_set_gpio_mode(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
	            if(mt_set_gpio_dir(pinSet[0][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMRST],pinSet[0][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[HM03D5] set gpio failed!! \n");} //low == reset sensor
	            if(mt_set_gpio_out(pinSet[0][IDX_PS_CMPDN],pinSet[0][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[HM03D5] set gpio failed!! \n");} //high == power down lens module
	        }
		}

		mdelay(10);
	}	
    else
    {
		PK_DBG("kdCISModulePowerOn get in---  other \n");

    }
  }
  else {//power OFF

    PK_DBG("kdCISModulePowerOn -off:currSensorName=%s\n",currSensorName);
    PK_DBG("kdCISModulePowerOn -off:pinSetIdx=%d\n",pinSetIdx);
    if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW, currSensorName)) && (pinSetIdx == 0))
    {
    	if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
    	//RST pin
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE]))
			{PK_DBG("[OV5648] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT))
			{PK_DBG("[OV5648] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF]))
			{PK_DBG("[OV5648] set gpio failed!! \n");}
		mdelay(5);

    	//PDN pin
		if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE]))
			{PK_DBG("[OV5648] set gpio mode failed!! \n");}
		if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT))
			{PK_DBG("[OV5648] set gpio dir failed!! \n");}
		if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF]))
			{PK_DBG("[OV5648] set gpio failed!! \n");}
		mdelay(5);
    	}
        // 2. turn off AVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))
        {
		PK_DBG("[OV5648] Fail to disable CAMERA_POWER_VCAM_A\n");
        }
        mdelay(10);

        // 3.Turn off DVDD 
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
        {
            PK_DBG("[OV5648] Fail to disable CAMERA_POWER_VCAM_D\n");
        }
        mdelay(10);

        // 4.Turn off DOVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
        {
            PK_DBG("[OV5648] Fail to disable CAMERA_POWER_VCAM_D2\n");
        }
        mdelay(10);
        
        // 5.Turn off AF_VCC
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,mode_name))
        {
            PK_DBG("[OV5648] Fail to OFF CAMERA_POWER_VCAM_A2 power\n");
        }
        mdelay(10);

        // 6.Turn off CMMCLK
        if(mt_set_gpio_mode(GPIO_122_FOR_CMMCLK, GPIO_MODE_00)){PK_DBG("[OV5648] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_122_FOR_CMMCLK,GPIO_DIR_OUT)){PK_DBG("[OV5648] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_122_FOR_CMMCLK,GPIO_OUT_ZERO)){PK_DBG("[OV5648] set gpio failed!! \n");}              
        ISP_MCLK1_EN(FALSE); // Turn off MCLK.
        mdelay(30);
	}
    else if (currSensorName && (0 == strcmp(SENSOR_DRVNAME_HM03D5_MIPI_YUV,currSensorName)) && (pinSetIdx == 1))
    {
    	//PDN pin
        if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE]))
            {PK_DBG("[HM03D5] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT))
            {PK_DBG("[HM03D5] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF]))
            {PK_DBG("[HM03D5] set gpio failed!! \n");}
        mdelay(5);

        // 2. turn off AVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A,mode_name))
        {
            PK_DBG("[HM03D5] Fail to disable CAMERA_POWER_VCAM_A\n");
        }
        mdelay(10);

        // 3.Turn off DVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D,mode_name))
        {
            PK_DBG("[HM03D5] Fail to disable CAMERA_POWER_VCAM_D\n");
        }
        mdelay(10);

        // 4.Turn off DOVDD
        if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_D2,mode_name))
        {
            PK_DBG("[HM03D5] Fail to disable CAMERA_POWER_VCAM_D2\n");
        }
        mdelay(10);
        
        // 6.Turn off CMMCLK
        if(mt_set_gpio_mode(GPIO_122_FOR_CMMCLK, GPIO_MODE_00)){PK_DBG("[HM03D5] set gpio mode failed!! \n");}
        if(mt_set_gpio_dir(GPIO_122_FOR_CMMCLK,GPIO_DIR_OUT)){PK_DBG("[HM03D5] set gpio dir failed!! \n");}
        if(mt_set_gpio_out(GPIO_122_FOR_CMMCLK,GPIO_OUT_ZERO)){PK_DBG("[HM03D5] set gpio failed!! \n");}              
        ISP_MCLK1_EN(FALSE); // Turn off MCLK.
        mdelay(30);
	}
    else
    {
        PK_DBG("kdCISModulePower--off get in---other \n");

    }
  }
	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

EXPORT_SYMBOL(kdCISModulePowerOn);


//!--
//




