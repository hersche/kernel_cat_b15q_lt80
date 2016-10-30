

#include <linux/types.h>
#include <cust_alsps.h>
#ifdef MT6573
#include <mach/mt6573_pll.h>
#endif
#ifdef MT6575
#include <mach/mt6575_pm_ldo.h>
#endif
#ifdef MT6577
#include <mach/mt6577_pm_ldo.h>
#endif
#ifdef MT6589
#include <mach/mt_pm_ldo.h>
#endif
//LT8X porting for STK3210 start
#include <mach/mt_pm_ldo.h>
//LT8X porting for STK3210 end
#include <mach/upmu_common.h>
static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 2,		/* i2c bus number, for mt657x, default=0. For mt6589, default=3 */
	//.polling_mode =1,
	.polling_mode_ps =0,//0: interrupt mode
	.polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    .i2c_addr   = {0x90, 0x00, 0x00, 0x00},	/*STK3x1x*/
	//.als_level  = {5,  9, 36, 59, 82, 132, 205, 273, 500, 845, 1136, 1545, 2364, 4655, 6982},	/* als_code */
    //.als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},    /* lux */
	//STK add for CCI start 20130113
    .als_level  = {10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},	// als_code for CCI VV01 TP transmittance = 10%
	.als_value  = {0, 10, 40, 65, 90, 145, 225, 300, 550, 930, 1250, 1700, 2600, 5120, 7680, 10240},    /* lux */
	//STK add for CCI end 20130113
   	.state_val = 0x0,		/* disable all */
	.psctrl_val = 0x33, //0x73,//VV01, CCI=0x73		/* ps_persistance=4, ps_gain=64X, PS_IT=0.391ms */
	.alsctrl_val = 0x39,	/* als_persistance=1, als_gain=64X, ALS_IT=100ms */
	.ledctrl_val = 0xFF,	/* 50mA IRDR, 64/64 LED duty */
	.wait_val = 0x9,		/* 50 ms */
    .ps_high_thd_val = 1500,
    .ps_low_thd_val = 1300,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

int pmic_ldo_suspend_enable(int enable)
{
        //0 for disable suspend, 1 for enable suspend
        upmu_set_vio18_lp_sel(enable);
        upmu_set_vio28_lp_sel(enable);
        return 0;
}
