


/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.h
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Header file of Sensor driver
 *
 *
 * Author:
 * -------
 *   PC Huang (MTK02204)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 09 10 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .alps dual sensor
 *
 * 09 02 2010 jackie.su
 * [ALPS00002279] [Need Patch] [Volunteer Patch] ALPS.Wxx.xx Volunteer patch for
 * .roll back dual sensor
 *
 * Mar 4 2010 mtk70508
 * [DUMA00154792] Sensor driver
 * 
 *
 * Feb 24 2010 mtk01118
 * [DUMA00025869] [Camera][YUV I/F & Query feature] check in camera code
 * 
 *
 * Aug 5 2009 mtk01051
 * [DUMA00009217] [Camera Driver] CCAP First Check In
 * 
 *
 * Apr 7 2009 mtk02204
 * [DUMA00004012] [Camera] Restructure and rename camera related custom folders and folder name of came
 * 
 *
 * Mar 26 2009 mtk02204
 * [DUMA00003515] [PC_Lint] Remove PC_Lint check warnings of camera related drivers.
 * 
 *
 * Mar 2 2009 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 * 
 *
 * Feb 24 2009 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 * 
 *
 * Dec 27 2008 MTK01813
 * DUMA_MBJ CheckIn Files
 * created by clearfsimport
 *
 * Dec 10 2008 mtk02204
 * [DUMA00001084] First Check in of MT6516 multimedia drivers
 * 
 *
 * Oct 27 2008 mtk01051
 * [DUMA00000851] Camera related drivers check in
 * Modify Copyright Header
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
/* SENSOR FULL SIZE */
#ifndef __SENSOR_H
#define __SENSOR_H



typedef enum _HM03D5_OP_TYPE_ {
        HM03D5_MODE_NONE,
        HM03D5_MODE_PREVIEW,
        HM03D5_MODE_CAPTURE,
        HM03D5_MODE_QCIF_VIDEO,
        HM03D5_MODE_CIF_VIDEO,
        HM03D5_MODE_QVGA_VIDEO
    } HM03D5_OP_TYPE;

extern HM03D5_OP_TYPE HM03D5_g_iHM03D5_Mode;

/* START GRAB PIXEL OFFSET */
#define IMAGE_SENSOR_START_GRAB_X		        2	// 0 or 1 recommended
#define IMAGE_SENSOR_START_GRAB_Y		        2	// 0 or 1 recommended

/* MAX/MIN FRAME RATE (FRAMES PER SEC.) */
#define MAX_FRAME_RATE							15		// Limitation for MPEG4 Encode Only
#define MIN_FRAME_RATE							12

/* SENSOR PIXEL/LINE NUMBERS IN ONE PERIOD */
    #define HM03D5_FULL_PERIOD_PIXEL_NUMS  (1940)  // default pixel#(w/o dummy pixels) in UXGA mode
    #define HM03D5_FULL_PERIOD_LINE_NUMS   (1238)  // default line#(w/o dummy lines) in UXGA mode
    #define HM03D5_PV_PERIOD_PIXEL_NUMS    (970)  // default pixel#(w/o dummy pixels) in SVGA mode
    #define HM03D5_PV_PERIOD_LINE_NUMS     (670)   // default line#(w/o dummy lines) in SVGA mode

    /* SENSOR EXPOSURE LINE LIMITATION */
    #define HM03D5_FULL_EXPOSURE_LIMITATION    (1236)
    #define HM03D5_PV_EXPOSURE_LIMITATION      (671)

/* SENSOR FULL SIZE */
   #define HM03D5_IMAGE_SENSOR_FULL_WIDTH	   (640 - 8)
   #define HM03D5_IMAGE_SENSOR_FULL_HEIGHT	  (480 - 6)


/* SENSOR PV SIZE */
#define HM03D5_IMAGE_SENSOR_PV_WIDTH   (640 - 8)
#define HM03D5_IMAGE_SENSOR_PV_HEIGHT  (480 - 6)

#define HM03D5_VIDEO_QCIF_WIDTH   (176)
#define HM03D5_VIDEO_QCIF_HEIGHT  (144)

#define HM03D5_VIDEO_30FPS_FRAME_LENGTH   (0x29E)
#define HM03D5_VIDEO_20FPS_FRAME_LENGTH   (0x3ED)
#define HM03D5_VIDEO_15FPS_FRAME_LENGTH   (0x53C)
#define HM03D5_VIDEO_10FPS_FRAME_LENGTH   (0x7DA)

// SETUP TIME NEED TO BE INSERTED
#define HM03D5_IMAGE_SENSOR_PV_INSERTED_PIXELS (390)
#define HM03D5_IMAGE_SENSOR_PV_INSERTED_LINES  (9 - 6)

#define HM03D5_IMAGE_SENSOR_FULL_INSERTED_PIXELS   (248)
#define HM03D5_IMAGE_SENSOR_FULL_INSERTED_LINES    (11 - 2)

#define HM03D5_PV_DUMMY_PIXELS			(600)
#define HM03D5_VIDEO__CIF_DUMMY_PIXELS  (100)
#define HM03D5_VIDEO__QCIF_DUMMY_PIXELS (0)

/* SENSOR SCALER FACTOR */
#define PV_SCALER_FACTOR					    3
#define FULL_SCALER_FACTOR					    1


/* DUMMY NEEDS TO BE INSERTED */
/* SETUP TIME NEED TO BE INSERTED */


/* SENSOR READ/WRITE ID */
	#define HM03D5_WRITE_ID							    0x60
	#define HM03D5_READ_ID								0x61


//s_add for porting
//s_add for porting
//s_add for porting

//export functions
UINT32 HM03D5Open(void);
UINT32 HM03D5GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution);
UINT32 HM03D5GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_INFO_STRUCT *pSensorInfo, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HM03D5Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow, MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData);
UINT32 HM03D5FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId, UINT8 *pFeaturePara,UINT32 *pFeatureParaLen);
UINT32 HM03D5Close(void);


//e_add for porting
//e_add for porting
//e_add for porting


#endif /* __SENSOR_H */
