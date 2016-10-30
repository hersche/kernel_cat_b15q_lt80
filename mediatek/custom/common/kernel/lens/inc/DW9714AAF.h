#ifndef _DW9714AAF_H
#define _DW9714AAF_H

#include <linux/ioctl.h>
//#include "kd_imgsensor.h"

#define DW9714AAF_MAGIC 'A'
//IOCTRL(inode * ,file * ,cmd ,arg )


//Structures
typedef struct {
//current position
unsigned long u4CurrentPosition;
//macro position
unsigned long u4MacroPosition;
//Infiniti position
unsigned long u4InfPosition;
//Motor Status
bool          bIsMotorMoving;
//Motor Open?
bool          bIsMotorOpen;
//Support SR?
bool          bIsSupportSR;
} stDW9714AAF_MotorInfo;

//Control commnad
//S means "set through a ptr"
//T means "tell by a arg value"
//G means "get by a ptr"
//Q means "get by return a value"
//X means "switch G and S atomically"
//H means "switch T and Q atomically"
#define DW9714AAFIOC_G_MOTORINFO _IOR(DW9714AAF_MAGIC,0,stDW9714AAF_MotorInfo)

#define DW9714AAFIOC_T_MOVETO _IOW(DW9714AAF_MAGIC,1,unsigned long)

#define DW9714AAFIOC_T_SETINFPOS _IOW(DW9714AAF_MAGIC,2,unsigned long)

#define DW9714AAFIOC_T_SETMACROPOS _IOW(DW9714AAF_MAGIC,3,unsigned long)

#else
#endif
