#ifdef BUILD_LK
    #ifndef bool
    typedef int bool;
    #endif
//#include <platform/cci_drv_common.h>
#include <platform/mt_gpio.h>
#include <string.h>
#else
//#include <cci_drv_common.h>
#include <linux/string.h>
#include <mach/mt_gpio.h>
#include <linux/proc_fs.h>

#ifndef BUILD_UBOOT
// Temporarily remove, Ken
//#include <linux/cci_drv_common.h>
#endif

#endif

#include "lcm_drv.h"


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

#define LCM_ID_NT35510 (5510)
#define REGFLAG_DELAY        0xFE
#define REGFLAG_END_OF_TABLE 0xFFF // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

/*************/
#define LSDA_GPIO_PIN       (GPIO_DISP_LSDA_PIN)

#define SET_GPIO_OUT(n, v)  (lcm_util.set_gpio_out((n), (v)))

#define SET_LSDA_LOW   SET_GPIO_OUT(LSDA_GPIO_PIN, 0)
#define SET_LSDA_HIGH  SET_GPIO_OUT(LSDA_GPIO_PIN, 1)

/***************/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)  lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)     lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                    lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                     lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)             lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
struct LCM_setting_table {
	unsigned char cmd;
	unsigned char count;
	unsigned char para_list[64];
};


#define LCM_DSI_CMD_MODE      1

#ifndef BUILD_LK
    static struct mutex g_device_mutex;
#endif

static struct LCM_setting_table lcm_initialization[] = {
	{0x35, 1,   {0x00}},
	//
	{0xff, 4,   {0xaa,0x55,0x25,0x01}},
	{0xf8, 8,   {0x01,0x02,0x00,0x20,0x33,0x13,0x00,0x48}},
	//
	{0xf0, 5,   {0x55,0xaa,0x52,0x08,0x01}},
	{0xb0, 3,   {0x10,0x10,0x10}},
	{0xb6, 3,   {0x44,0x44,0x44}},
	{0xb1, 3,   {0x10,0x10,0x10}},
	{0xb7, 3,   {0x34,0x34,0x34}},
	{0xb2, 3,   {0x00,0x00,0x00}},
	{0xb8, 3,   {0x34,0x34,0x34}},
	{0xbf, 4,   {0x01,0x0f,0x0f,0x0f}},

	{0xd1, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{0xd2, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{0xd3, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{0xd4, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{0xd5, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{0xd6, 52,  {0x00,0x33,0x00,0x34,0x00,0x3a,0x00,
	                0x4a,0x00,0x5c,0x00,
	                0x81,0x00,0xa6,0x00,
	                0xe5,0x01,0x13,0x01,
	                0x54,0x01,0x82,0x01,
	                0xca,0x02,0x00,0x02,
	                0x01,0x02,0x34,0x02,
	                0x67,0x02,0x84,0x02,
	                0xa4,0x02,0xb7,0x02,
	                0xcf,0x02,0xde,0x02,
	                0xf2,0x02,0xfe,0x03,
	                0x10,0x03,0x33,0x03,
	                0x6d}},
	{REGFLAG_DELAY, 10, {}},
	{0xb9, 3,  {0x34,0x34,0x34}},
	{0xb5, 3,  {0x08,0x08,0x08}},
	{0xba, 3,  {0x14,0x14,0x14}},
	{0xbc, 3,  {0x00,0x78,0x00}},
	{0xbd, 3,  {0x00,0x78,0x00}},
	{0xbe, 2,  {0x00,0x64}},
	{0xf0, 5,  {0x55,0xaa,0x52,0x08,0x00}},
	{0xb1, 2,  {0xfc,0x00}},
	{0xb6, 1,  {0x05}},
	{0xb8, 4,  {0x01,0x03,0x03,0x03}},
	{0xbc, 3,  {0x02,0x00,0x00}},

	{0x11, 1,  {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{0x2a, 4,  {0x00,0x00,0x01,0xdf}},
	{0x2b, 4,  {0x00,0x00,0x03,0x55}},
	{0x29, 1,  {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
	// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 60, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for(i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {
			case REGFLAG_DELAY:
				MDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE:
				break;

			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

static void init_lcm_registers(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00023902;
	data_array[1]=0x00000035; // open TE,only v output
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00063902;
	data_array[1]=0x52aa55f0; // enable manufacture command ,select page 1
	data_array[2]=0x00000108;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x101010b0;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x444444b6;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x101010b1;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x343434b7;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x000000b2;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x343434b8;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00053902;
	data_array[1]=0x0f0f01bf;
	data_array[2]=0x0000000f;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d1;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	//MDELAY(20);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d2;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	//MDELAY(20);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d3;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	//MDELAY(20);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d4;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	//MDELAY(20);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d5;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	//MDELAY(20);

	data_array[0]=0x00353902;
	data_array[1]=0x003300d6;
	data_array[2]=0x003a0034;
	data_array[3]=0x005c004a;
	data_array[4]=0x00a60081;
	data_array[5]=0x011301e5;
	data_array[6]=0x01820154;
	data_array[7]=0x020002ca;
	data_array[8]=0x02340201;
	data_array[9]=0x02840267;
	data_array[10]=0x02b702a4;
	data_array[11]=0x02de02cf;
	data_array[12]=0x03fe02f2;
	data_array[13]=0x03330310;
	data_array[14]=0x0000006d;
	dsi_set_cmdq(&data_array,15, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x343434b9;//timing for circuit 4
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x080808b5;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x141414ba;//timing for circuit 5
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x007800bc;//set output voltage of gamma divider P
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x007800bd;//set output voltage of gamma divider N
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00033902;
	data_array[1]=0x006400be;//adjust the DC VCOM offset voltage
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	data_array[0]=0x00063902;
	data_array[1]=0x52aa55f0;//enable manufacture command ,select page 0
	data_array[2]=0x00000008;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(10);

	data_array[0]=0x00033902;
	data_array[1]=0x0000fcb1;//VIDEO MODE off ; TE on; RGB order,content keep in sleep in
	dsi_set_cmdq(&data_array,2, 1);
	MDELAY(10);

	data_array[0]=0x00023902;
	data_array[1]=0x000005b6;
	dsi_set_cmdq(&data_array,2, 1);
	MDELAY(10);

	data_array[0]=0x00053902;
	data_array[1]=0x030301b8;//setting of inversion mode
	data_array[2]=0x00000003;
	dsi_set_cmdq(&data_array,3, 1);
	MDELAY(10);

	data_array[0]=0x00043902;
	data_array[1]=0x000002bc;//setting of inversion mode
	dsi_set_cmdq(&data_array,2, 1);
	MDELAY(10);

	//data_array[0]=0x00110500;
	//dsi_set_cmdq(&data_array,1, 1);
	//MDELAY(200);

	data_array[0]=0x00053902;
	data_array[1]=0x0100002a;
	data_array[2]=0x000000df;//column range set
	dsi_set_cmdq(&data_array,3, 1);
	MDELAY(200);

	data_array[0]=0x00053902;
	data_array[1]=0x0300002b;
	data_array[2]=0x00000055;//ROW range set
	dsi_set_cmdq(&data_array,3, 1);
	MDELAY(200);

	//cabc begin
	//data_array[0]=0x00023902;
	//data_array[1]=0x0000ff51;
	//dsi_set_cmdq(&data_array,2, 1);
	//MDELAY(10);

	//data_array[0]=0x00023902;
	//data_array[1]=0x00002c53;
	//dsi_set_cmdq(&data_array,2, 1);
	//MDELAY(10);

	//data_array[0]=0x00023902;
	//data_array[1]=0x00000255;
	//dsi_set_cmdq(&data_array,2, 1);
	//MDELAY(10);
	//cabc end
}
// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	// enable tearing-free
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	//params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;

	//params->dsi.lcm_ext_te_enable = TRUE;
#if defined(LCM_DSI_CMD_MODE)
	params->dsi.mode = CMD_MODE;
#else
	params->dsi.mode = SYNC_PULSE_VDO_MODE;
#endif

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_TWO_LANE;

	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573 => no use in MT6573
	params->dsi.packet_size = 256;

	// Video mode setting
	params->dsi.intermediat_buffer_num = 0;
	// Because DSI/DPI HW design change, this parameter should be 0
	// when video mode in MT658X, or it will memory leakage.

	// this parameter would be set to 1 if DriverIC is NTK's
	// and when force match DSI clock for NTK's
	params->dsi.compatibility_for_nvk = 1;

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count = 480*3;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	/*
	params->dsi.vertical_sync_active = 3;
	params->dsi.vertical_backporch   = 12;
	params->dsi.vertical_frontporch  = 2;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active  = 10;
	params->dsi.horizontal_backporch    = 50;
	params->dsi.horizontal_frontporch   = 50;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	*/

	// Bit rate calculation
	// 351 M
	//params->dsi.pll_div1 = 0x1a;
	//params->dsi.pll_div2 = 1;
	params->dsi.PLL_CLOCK = 201; //250;
	//params->dsi.pll_div1 = 1;
	//params->dsi.pll_div2 = 1;
	//params->dsi.fbk_div  = 30;
}
static void lcm_init(void)
{
#ifdef BUILD_LK
	printf("[LCM][%s] ENTER.\n",__func__);
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(67);
	push_table(lcm_initialization, sizeof(lcm_initialization) / sizeof(struct LCM_setting_table), 1);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif

#ifdef BUILD_LK
	printf("[LCM][%s]nt35510 EXIT.\n",__func__);
#else
	printk("[LCM][%s]nt35510 EXIT.\n",__func__);
#endif
}



static void lcm_suspend(void)
{
	#ifdef BUILD_LK
	printf("[LCM][%s] ENTER.\n",__func__);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif
	/*
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);
	*/
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

#ifdef BUILD_LK
	printf("[LCM][%s]nt35510 LEAVE.\n",__func__);
#else
	printk("[LCM][%s]nt35510 LEAVE.\n",__func__);
#endif
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("[LCM][%s] ENTER.\n",__func__);
#else
	printk("[LCM][%s] ENTER.\n",__func__);
#endif

	/*
	unsigned int data_array[16];

	data_array[0] = 0x00110500; // Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);
	data_array[0] = 0x00290500; // Display On
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);
	*/

	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

#ifdef BUILD_LK
	printf("[LCM][%s]nt35510 LEAVE.\n",__func__);
#else
	printk("[LCM][%s]nt35510 LEAVE.\n",__func__);
#endif
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(&data_array, 1, 0);
}


static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];

	//Do reset here
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(25);

	SET_RESET_PIN(1);
	MDELAY(50);

	array[0]=0x00063902;
	array[1]=0x52aa55f0;
	array[2]=0x00000108;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);

	array[0] = 0x00083700;
	dsi_set_cmdq(array, 1, 1);

	//read_reg_v2(0x04, buffer, 3);  //if read 0x04,should get 0x008000,that is both OK.
	read_reg_v2(0xc5, buffer,2);

	id = buffer[0]<<8 |buffer[1];

#ifdef BUILD_UBOOT
	printf("%s, id = 0x%x \n", __func__, id);
#endif

	if(id == LCM_ID_NT35510)
		return 1;
	else
		return 0;
}

/**** sysfs ***/
#ifndef BUILD_LK
static ssize_t nt35510b_lcmchipver_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t num_read_chars = 0;

	mutex_lock(&g_device_mutex);

	unsigned int id=0;
	unsigned char buffer[3];
	unsigned int array[16];

	array[0]=0x00063902;
	array[1]=0x52aa55f0;
	array[2]=0x00000108;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);

	array[0] = 0x00083700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xc5, buffer, 3);
	//id = buffer[0]<<8 |buffer[1];
	num_read_chars = snprintf(buf, PAGE_SIZE, "0x%02x\n",buffer[2]);
	mutex_unlock(&g_device_mutex);

	return num_read_chars;
}

static ssize_t nt35510b_lcmchipver_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	/*place holder for future use*/
	return -EPERM;
}
/*get the fw version
*example:cat /sys/class/graphics/fb0/device/lcmchipver
*/
static DEVICE_ATTR(lcmchipver, S_IRUGO | S_IWUSR, nt35510b_lcmchipver_show,
			nt35510b_lcmchipver_store);

/*add your attr in here*/
static struct attribute *nt35510b_attributes[] = {
	&dev_attr_lcmchipver.attr,
	NULL
};

static struct attribute_group nt35510b_attribute_group = {
	.attrs = nt35510b_attributes
};
#endif

static void lcm_sysfs_register(struct device *dev)
{
	int err = -1;
#ifndef BUILD_LK
	err = sysfs_create_group(&dev->kobj, &nt35510b_attribute_group);
	if (0 != err) {
		printk("%s() - ERROR: sysfs_create_group() failed.\n",
		        __func__);
		sysfs_remove_group(&dev->kobj, &nt35510b_attribute_group);
		return -EIO;
	} else {
		mutex_init(&g_device_mutex);
		printk("nt35510b:%s() - sysfs_create_group() succeeded.\n",
		        __func__);
	}
#endif
}

static void lcm_sysfs_unregister(struct device *dev)
{
#ifndef BUILD_LK
	sysfs_remove_group(&dev->kobj, &nt35510b_attribute_group);
	mutex_destroy(&g_device_mutex);
#endif
}
/**** sysfs ***/

LCM_DRIVER nt35510b_dsi_cmd_wvga_drv=
{
	.name           = "nt35510b_dsi_cmd_wvga",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	//.compare_id     = lcm_compare_id,
#if defined(LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
	.sysfs_register   = lcm_sysfs_register,
	.sysfs_unregister = lcm_sysfs_unregister,
};
