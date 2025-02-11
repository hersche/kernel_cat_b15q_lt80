/*
 * MD218A voice coil motor driver
 *
 *
 */

#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include "DW9714AAF.h"
#include "../camera/kd_camera_hw.h"

#define LENS_I2C_BUSNUM 1
static struct i2c_board_info __initdata kd_lens_dev={ I2C_BOARD_INFO("DW9714AAF", 0x18)};


#define DW9714AAF_DRVNAME "DW9714AAF"
#define DW9714AAF_VCM_WRITE_ID           0x18

#define DW9714AAF_DEBUG
#ifdef DW9714AAF_DEBUG
#define DW9714AAFDB printk
#else
#define DW9714AAFDB(x,...)
#endif

static spinlock_t g_DW9714AAF_SpinLock;

static struct i2c_client * g_pstDW9714AAF_I2Cclient = NULL;

static dev_t g_DW9714AAF_devno;
static struct cdev * g_pDW9714AAF_CharDrv = NULL;
static struct class *actuator_class = NULL;

static int  g_s4DW9714AAF_Opened = 0;
static long g_i4MotorStatus = 0;
static long g_i4Dir = 0;
static unsigned long g_u4DW9714AAF_INF = 0;
static unsigned long g_u4DW9714AAF_MACRO = 1023;
static unsigned long g_u4TargetPosition = 0;
static unsigned long g_u4CurrPosition   = 0;

static int g_sr = 3;

#if 0
extern s32 mt_set_gpio_mode(u32 u4Pin, u32 u4Mode);
extern s32 mt_set_gpio_out(u32 u4Pin, u32 u4PinOut);
extern s32 mt_set_gpio_dir(u32 u4Pin, u32 u4Dir);
#endif

static int s4DW9714AAF_ReadReg(unsigned short * a_pu2Result)
{
    int  i4RetValue = 0;
    char pBuff[2];

    i4RetValue = i2c_master_recv(g_pstDW9714AAF_I2Cclient, pBuff , 2);

    if (i4RetValue < 0)
    {
        DW9714AAFDB("[DW9714AAF] I2C read failed!! \n");
        return -1;
    }

    *a_pu2Result = (((u16)pBuff[0]) << 4) + (pBuff[1] >> 4);

    return 0;
}

static int s4DW9714AAF_WriteReg(u16 a_u2Data)
{
    int  i4RetValue = 0;

    char puSendCmd[2] = {(char)(a_u2Data >> 4) , (char)(((a_u2Data & 0xF) << 4)+g_sr)};

    //DW9714AAFDB("[DW9714AAF] g_sr %d, write %d \n", g_sr, a_u2Data);
    g_pstDW9714AAF_I2Cclient->ext_flag |= I2C_A_FILTER_MSG;
    i4RetValue = i2c_master_send(g_pstDW9714AAF_I2Cclient, puSendCmd, 2);

    if (i4RetValue < 0)
    {
        DW9714AAFDB("[DW9714AAF] I2C send failed!! \n");
        return -1;
    }

    return 0;
}

inline static int getDW9714AAFInfo(__user stDW9714AAF_MotorInfo * pstMotorInfo)
{
    stDW9714AAF_MotorInfo stMotorInfo;
    stMotorInfo.u4MacroPosition   = g_u4DW9714AAF_MACRO;
    stMotorInfo.u4InfPosition     = g_u4DW9714AAF_INF;
    stMotorInfo.u4CurrentPosition = g_u4CurrPosition;
    stMotorInfo.bIsSupportSR      = TRUE;

	if (g_i4MotorStatus == 1)	{stMotorInfo.bIsMotorMoving = 1;}
	else						{stMotorInfo.bIsMotorMoving = 0;}

    if (g_s4DW9714AAF_Opened >= 1) {stMotorInfo.bIsMotorOpen = 1;}
	else						{stMotorInfo.bIsMotorOpen = 0;}

    if(copy_to_user(pstMotorInfo , &stMotorInfo , sizeof(stDW9714AAF_MotorInfo)))
    {
        DW9714AAFDB("[DW9714AAF] copy to user failed when getting motor information \n");
    }

    return 0;
}

#ifdef LensdrvCM3
inline static int getDW9714AAFMETA(__user stDW9714AAF_MotorMETAInfo * pstMotorMETAInfo)
{
    stDW9714AAF_MotorMETAInfo stMotorMETAInfo;
    stMotorMETAInfo.Aperture=2.8;      //fn
	stMotorMETAInfo.Facing=1;
	stMotorMETAInfo.FilterDensity=1;   //X
	stMotorMETAInfo.FocalDistance=1.0;  //diopters
	stMotorMETAInfo.FocalLength=34.0;  //mm
	stMotorMETAInfo.FocusRange=1.0;    //diopters
	stMotorMETAInfo.InfoAvalibleApertures=2.8;
	stMotorMETAInfo.InfoAvalibleFilterDensity=1;
	stMotorMETAInfo.InfoAvalibleFocalLength=34.0;
	stMotorMETAInfo.InfoAvalibleHypeDistance=1.0;
	stMotorMETAInfo.InfoAvalibleMinFocusDistance=1.0;
	stMotorMETAInfo.InfoAvalibleOptStabilization=0;
	stMotorMETAInfo.OpticalAxisAng[0]=0.0;
	stMotorMETAInfo.OpticalAxisAng[1]=0.0;
	stMotorMETAInfo.Position[0]=0.0;
	stMotorMETAInfo.Position[1]=0.0;
	stMotorMETAInfo.Position[2]=0.0;
	stMotorMETAInfo.State=0;
	stMotorMETAInfo.u4OIS_Mode=0;

    if(copy_to_user(pstMotorMETAInfo , &stMotorMETAInfo , sizeof(stDW9714AAF_MotorMETAInfo)))
	{
        DW9714AAFDB("[DW9714AAF] copy to user failed when getting motor information \n");
	}

    return 0;
}
#endif

inline static int moveDW9714AAF(unsigned long a_u4Position)
{
    int ret = 0;

    if((a_u4Position > g_u4DW9714AAF_MACRO) || (a_u4Position < g_u4DW9714AAF_INF))
    {
        DW9714AAFDB("[DW9714AAF] out of range \n");
        return -EINVAL;
    }

    if (g_s4DW9714AAF_Opened == 1)
    {
        unsigned short InitPos;
        ret = s4DW9714AAF_ReadReg(&InitPos);

        if(ret == 0)
        {
            DW9714AAFDB("[DW9714AAF] Init Pos %6d \n", InitPos);

            spin_lock(&g_DW9714AAF_SpinLock);
            g_u4CurrPosition = (unsigned long)InitPos;
            spin_unlock(&g_DW9714AAF_SpinLock);

        }
        else
        {
            spin_lock(&g_DW9714AAF_SpinLock);
            g_u4CurrPosition = 0;
            spin_unlock(&g_DW9714AAF_SpinLock);
        }

        spin_lock(&g_DW9714AAF_SpinLock);
        g_s4DW9714AAF_Opened = 2;
        spin_unlock(&g_DW9714AAF_SpinLock);
    }

    if (g_u4CurrPosition < a_u4Position)
    {
        spin_lock(&g_DW9714AAF_SpinLock);
        g_i4Dir = 1;
        spin_unlock(&g_DW9714AAF_SpinLock);
    }
    else if (g_u4CurrPosition > a_u4Position)
    {
        spin_lock(&g_DW9714AAF_SpinLock);
        g_i4Dir = -1;
        spin_unlock(&g_DW9714AAF_SpinLock);
    }
    else										{return 0;}

    spin_lock(&g_DW9714AAF_SpinLock);
    g_u4TargetPosition = a_u4Position;
    spin_unlock(&g_DW9714AAF_SpinLock);

    //DW9714AAFDB("[DW9714AAF] move [curr] %d [target] %d\n", g_u4CurrPosition, g_u4TargetPosition);

            spin_lock(&g_DW9714AAF_SpinLock);
            g_sr = 3;
            g_i4MotorStatus = 0;
            spin_unlock(&g_DW9714AAF_SpinLock);

            if(s4DW9714AAF_WriteReg((unsigned short)g_u4TargetPosition) == 0)
            {
                spin_lock(&g_DW9714AAF_SpinLock);
                g_u4CurrPosition = (unsigned long)g_u4TargetPosition;
                spin_unlock(&g_DW9714AAF_SpinLock);
            }
            else
            {
                DW9714AAFDB("[DW9714AAF] set I2C failed when moving the motor \n");
                spin_lock(&g_DW9714AAF_SpinLock);
                g_i4MotorStatus = -1;
                spin_unlock(&g_DW9714AAF_SpinLock);
            }

    return 0;
}

inline static int setDW9714AAFInf(unsigned long a_u4Position)
{
    spin_lock(&g_DW9714AAF_SpinLock);
    g_u4DW9714AAF_INF = a_u4Position;
    spin_unlock(&g_DW9714AAF_SpinLock);
    return 0;
}

inline static int setDW9714AAFMacro(unsigned long a_u4Position)
{
    spin_lock(&g_DW9714AAF_SpinLock);
    g_u4DW9714AAF_MACRO = a_u4Position;
    spin_unlock(&g_DW9714AAF_SpinLock);
    return 0;
}

////////////////////////////////////////////////////////////////
static long DW9714AAF_Ioctl(
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
{
    long i4RetValue = 0;

    switch(a_u4Command)
    {
        case DW9714AAFIOC_G_MOTORINFO :
            i4RetValue = getDW9714AAFInfo((__user stDW9714AAF_MotorInfo *)(a_u4Param));
        break;
		#ifdef LensdrvCM3
        case DW9714AAFIOC_G_MOTORMETAINFO :
            i4RetValue = getDW9714AAFMETA((__user stDW9714AAF_MotorMETAInfo *)(a_u4Param));
        break;
		#endif
        case DW9714AAFIOC_T_MOVETO :
            i4RetValue = moveDW9714AAF(a_u4Param);
        break;

        case DW9714AAFIOC_T_SETINFPOS :
            i4RetValue = setDW9714AAFInf(a_u4Param);
        break;

        case DW9714AAFIOC_T_SETMACROPOS :
            i4RetValue = setDW9714AAFMacro(a_u4Param);
        break;

        default :
            DW9714AAFDB("[DW9714AAF] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    return i4RetValue;
}

//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
// 3.Update f_op pointer.
// 4.Fill data structures into private_data
//CAM_RESET
static int DW9714AAF_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    DW9714AAFDB("[DW9714AAF] DW9714AAF_Open - Start\n");


    if(g_s4DW9714AAF_Opened)
    {
        DW9714AAFDB("[DW9714AAF] the device is opened \n");
        return -EBUSY;
    }

    spin_lock(&g_DW9714AAF_SpinLock);
    g_s4DW9714AAF_Opened = 1;
    spin_unlock(&g_DW9714AAF_SpinLock);

    DW9714AAFDB("[DW9714AAF] DW9714AAF_Open - End\n");

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int DW9714AAF_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    DW9714AAFDB("[DW9714AAF] DW9714AAF_Release - Start\n");

    if (g_s4DW9714AAF_Opened)
    {
        DW9714AAFDB("[DW9714AAF] feee \n");
        g_sr = 5;
        s4DW9714AAF_WriteReg(200);
        msleep(10);
        s4DW9714AAF_WriteReg(100);
        msleep(10);

        spin_lock(&g_DW9714AAF_SpinLock);
        g_s4DW9714AAF_Opened = 0;
        spin_unlock(&g_DW9714AAF_SpinLock);

    }

    DW9714AAFDB("[DW9714AAF] DW9714AAF_Release - End\n");

    return 0;
}

static const struct file_operations g_stDW9714AAF_fops =
{
    .owner = THIS_MODULE,
    .open = DW9714AAF_Open,
    .release = DW9714AAF_Release,
    .unlocked_ioctl = DW9714AAF_Ioctl
};

inline static int Register_DW9714AAF_CharDrv(void)
{
    struct device* vcm_device = NULL;

    DW9714AAFDB("[DW9714AAF] Register_DW9714AAF_CharDrv - Start\n");

    //Allocate char driver no.
    if( alloc_chrdev_region(&g_DW9714AAF_devno, 0, 1,DW9714AAF_DRVNAME) )
    {
        DW9714AAFDB("[DW9714AAF] Allocate device no failed\n");

        return -EAGAIN;
    }

    //Allocate driver
    g_pDW9714AAF_CharDrv = cdev_alloc();

    if(NULL == g_pDW9714AAF_CharDrv)
    {
        unregister_chrdev_region(g_DW9714AAF_devno, 1);

        DW9714AAFDB("[DW9714AAF] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pDW9714AAF_CharDrv, &g_stDW9714AAF_fops);

    g_pDW9714AAF_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pDW9714AAF_CharDrv, g_DW9714AAF_devno, 1))
    {
        DW9714AAFDB("[DW9714AAF] Attatch file operation failed\n");

        unregister_chrdev_region(g_DW9714AAF_devno, 1);

        return -EAGAIN;
    }

    actuator_class = class_create(THIS_MODULE, "actuatordrv");
    if (IS_ERR(actuator_class)) {
        int ret = PTR_ERR(actuator_class);
        DW9714AAFDB("Unable to create class, err = %d\n", ret);
        return ret;
    }

    vcm_device = device_create(actuator_class, NULL, g_DW9714AAF_devno, NULL, DW9714AAF_DRVNAME);

    if(NULL == vcm_device)
    {
        return -EIO;
    }

    DW9714AAFDB("[DW9714AAF] Register_DW9714AAF_CharDrv - End\n");
    return 0;
}

inline static void Unregister_DW9714AAF_CharDrv(void)
{
    DW9714AAFDB("[DW9714AAF] Unregister_DW9714AAF_CharDrv - Start\n");

    //Release char driver
    cdev_del(g_pDW9714AAF_CharDrv);

    unregister_chrdev_region(g_DW9714AAF_devno, 1);

    device_destroy(actuator_class, g_DW9714AAF_devno);

    class_destroy(actuator_class);

    DW9714AAFDB("[DW9714AAF] Unregister_DW9714AAF_CharDrv - End\n");
}

//////////////////////////////////////////////////////////////////////

static int DW9714AAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int DW9714AAF_i2c_remove(struct i2c_client *client);
static const struct i2c_device_id DW9714AAF_i2c_id[] = {{DW9714AAF_DRVNAME,0},{}};
struct i2c_driver DW9714AAF_i2c_driver = {
    .probe = DW9714AAF_i2c_probe,
    .remove = DW9714AAF_i2c_remove,
    .driver.name = DW9714AAF_DRVNAME,
    .id_table = DW9714AAF_i2c_id,
};

#if 0
static int DW9714AAF_i2c_detect(struct i2c_client *client, int kind, struct i2c_board_info *info) {
    strcpy(info->type, DW9714AAF_DRVNAME);
    return 0;
}
#endif
static int DW9714AAF_i2c_remove(struct i2c_client *client) {
    return 0;
}

/* Kirby: add new-style driver {*/
static int DW9714AAF_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int i4RetValue = 0;

    DW9714AAFDB("[DW9714AAF] DW9714AAF_i2c_probe\n");

    /* Kirby: add new-style driver { */
    g_pstDW9714AAF_I2Cclient = client;

    g_pstDW9714AAF_I2Cclient->addr = g_pstDW9714AAF_I2Cclient->addr >> 1;

    //Register char driver
    i4RetValue = Register_DW9714AAF_CharDrv();

    if(i4RetValue){

        DW9714AAFDB("[DW9714AAF] register char device failed!\n");

        return i4RetValue;
    }

    spin_lock_init(&g_DW9714AAF_SpinLock);

    DW9714AAFDB("[DW9714AAF] Attached!! \n");

    return 0;
}

static int DW9714AAF_probe(struct platform_device *pdev)
{
    return i2c_add_driver(&DW9714AAF_i2c_driver);
}

static int DW9714AAF_remove(struct platform_device *pdev)
{
    i2c_del_driver(&DW9714AAF_i2c_driver);
    return 0;
}

static int DW9714AAF_suspend(struct platform_device *pdev, pm_message_t mesg)
{
    return 0;
}

static int DW9714AAF_resume(struct platform_device *pdev)
{
    return 0;
}

// platform structure
static struct platform_driver g_stDW9714AAF_Driver = {
    .probe      = DW9714AAF_probe,
    .remove = DW9714AAF_remove,
    .suspend    = DW9714AAF_suspend,
    .resume = DW9714AAF_resume,
    .driver		= {
        .name	= "lens_actuator",
        .owner	= THIS_MODULE,
    }
};

static int __init DW9714AAF_i2C_init(void)
{
    i2c_register_board_info(LENS_I2C_BUSNUM, &kd_lens_dev, 1);

    if(platform_driver_register(&g_stDW9714AAF_Driver)){
        DW9714AAFDB("failed to register DW9714AAF driver\n");
        return -ENODEV;
    }

    return 0;
}

static void __exit DW9714AAF_i2C_exit(void)
{
    platform_driver_unregister(&g_stDW9714AAF_Driver);
}

module_init(DW9714AAF_i2C_init);
module_exit(DW9714AAF_i2C_exit);

MODULE_DESCRIPTION("DW9714AAF lens module driver");
MODULE_AUTHOR("KY Chen <ky.chen@Mediatek.com>");
MODULE_LICENSE("GPL");


