#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include "camera_custom_nvram.h"
#include "camera_custom_sensor.h"
#include "image_sensor.h"
#include "kd_imgsensor_define.h"
#include "camera_AE_PLineTable_ov5648mipiraw.h"
#include "camera_info_ov5648mipiraw.h"
#include "camera_custom_AEPlinetable.h"
#include "camera_custom_tsf_tbl.h"
const NVRAM_CAMERA_ISP_PARAM_STRUCT CAMERA_ISP_DEFAULT_VALUE =
{{
    //Version
    Version: NVRAM_CAMERA_PARA_FILE_VERSION,

    //SensorId
    SensorId: SENSOR_ID,
    ISPComm:{
        {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    	}
    },
    ISPPca: {
        #include INCLUDE_FILENAME_ISP_PCA_PARAM
    },
    ISPRegs:{
        #include INCLUDE_FILENAME_ISP_REGS_PARAM
    },
    ISPMfbMixer:{{
        {//00: MFB mixer for ISO 100
            0x00000000, 0x00000000
        },
        {//01: MFB mixer for ISO 200
            0x00000000, 0x00000000
        },
        {//02: MFB mixer for ISO 400
            0x00000000, 0x00000000
        },
        {//03: MFB mixer for ISO 800
            0x00000000, 0x00000000
        },
        {//04: MFB mixer for ISO 1600
            0x00000000, 0x00000000
        },
        {//05: MFB mixer for ISO 2400
            0x00000000, 0x00000000
        },
        {//06: MFB mixer for ISO 3200
            0x00000000, 0x00000000
        }
    }},
    ISPCcmPoly22:{
        66225,    // i4R_AVG
        12121,    // i4R_STD
        87775,    // i4B_AVG
        23123,    // i4B_STD
        {  // i4P00[9]
            5115000, -1760000, -795000, -920000, 3662500, -185000, 17500, -2135000, 4680000
        },
        {  // i4P10[9]
            744016, -921855, 177838, -15868, -178606, 197151, 33075, 530272, -554129
        },
        {  // i4P01[9]
            241081, -390841, 149760, -148838, -238736, 389140, -18150, -199621, 226283
        },
        {  // i4P20[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P11[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        },
        {  // i4P02[9]
            0, 0, 0, 0, 0, 0, 0, 0, 0
        }
    }
}};

const NVRAM_CAMERA_3A_STRUCT CAMERA_3A_NVRAM_DEFAULT_VALUE =
{
    NVRAM_CAMERA_3A_FILE_VERSION, // u4Version
    SENSOR_ID, // SensorId

    // AE NVRAM
    {
        // rDevicesInfo
        {
            1136,    // u4MinGain, 1024 base = 1x
            10240,    // u4MaxGain, 16x
            86,    // u4MiniISOGain, ISOxx  
            256,    // u4GainStepUnit, 1x/8 
            13172,    // u4PreExpUnit 
            30,    // u4PreMaxFrameRate
            13172,    // u4VideoExpUnit  
            30,    // u4VideoMaxFrameRate 
            1024,    // u4Video2PreRatio, 1024 base = 1x 
            13172,    // u4CapExpUnit 
            30,    // u4CapMaxFrameRate
            1024,    // u4Cap2PreRatio, 1024 base = 1x
            24,    // u4LensFno, Fno = 2.8
            350    // u4FocusLength_100x
        },
        // rHistConfig
        {
            2,   // u4HistHighThres
            40,  // u4HistLowThres
            2,   // u4MostBrightRatio
            1,   // u4MostDarkRatio
            160, // u4CentralHighBound
            20,  // u4CentralLowBound
            {240, 230, 220, 210, 200}, // u4OverExpThres[AE_CCT_STRENGTH_NUM]
            {86, 108, 128, 148, 170},  // u4HistStretchThres[AE_CCT_STRENGTH_NUM]
            {18, 22, 26, 30, 34}       // u4BlackLightThres[AE_CCT_STRENGTH_NUM]
        },
        // rCCTConfig
        {
            TRUE,            // bEnableBlackLight
            TRUE,            // bEnableHistStretch
            FALSE,           // bEnableAntiOverExposure
            TRUE,            // bEnableTimeLPF
            TRUE,            // bEnableCaptureThres
            TRUE,            // bEnableVideoThres
            TRUE,            // bEnableStrobeThres
            47,                // u4AETarget
            0,                // u4StrobeAETarget
            50,                // u4InitIndex
            4,                 // u4BackLightWeight
            32,                // u4HistStretchWeight
            4,                 // u4AntiOverExpWeight
            2,                 // u4BlackLightStrengthIndex
            2,                 // u4HistStretchStrengthIndex
            2,                 // u4AntiOverExpStrengthIndex
            2,                 // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8}, // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM]
            90,                // u4InDoorEV = 9.0, 10 base
            -3,               // i4BVOffset delta BV = value/10 
            64,                 // u4PreviewFlareOffset
            64,                 // u4CaptureFlareOffset
            5,                 // u4CaptureFlareThres
            64,                 // u4VideoFlareOffset
            5,                 // u4VideoFlareThres
            64,                 // u4StrobeFlareOffset
            2,                 // u4StrobeFlareThres
            8,                 // u4PrvMaxFlareThres
            0,                 // u4PrvMinFlareThres
            8,                 // u4VideoMaxFlareThres
            0,                 // u4VideoMinFlareThres            
            18,                // u4FlatnessThres              // 10 base for flatness condition.
            50                 // u4FlatnessStrength
         }
    },

    // AWB NVRAM
    {
    	// AWB calibration data
    	{
    		// rUnitGain (unit gain: 1.0 = 512)
    		{
    			0,	// i4R
    			0,	// i4G
    			0	// i4B
    		},
    		// rGoldenGain (golden sample gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
    		// rTuningUnitGain (Tuning sample unit gain: 1.0 = 512)
    		{
	            0,	// i4R
	            0,	// i4G
	            0	// i4B
            },
            // rD65Gain (D65 WB gain: 1.0 = 512)
            {
                751,    // i4R
                512,    // i4G
                584    // i4B
    		}
    	},
    	// Original XY coordinate of AWB light source
    	{
           // Strobe
            {
                87,    // i4X
                -336    // i4Y
            },
            // Horizon
            {
                -423,    // i4X
                -279    // i4Y
            },
            // A
            {
                -308,    // i4X
                -269    // i4Y
            },
            // TL84
            {
                -173,    // i4X
                -294    // i4Y
            },
            // CWF
            {
                -104,    // i4X
                -370    // i4Y
            },
            // DNP
            {
                -6,    // i4X
                -236    // i4Y
            },
            // D65
            {
                93,    // i4X
                -190    // i4Y
            },
            // DF
            {
                57,    // i4X
                -294    // i4Y
            }
    	},
    	// Rotated XY coordinate of AWB light source
    	{
            // Strobe
            {
                15,    // i4X
                -346    // i4Y
            },
            // Horizon
            {
                -471,    // i4X
                -185    // i4Y
            },
            // A
            {
                -356,    // i4X
                -199    // i4Y
            },
            // TL84
            {
                -230,    // i4X
                -251    // i4Y
            },
            // CWF
            {
                -178,    // i4X
                -340    // i4Y
            },
            // DNP
            {
                -55,    // i4X
                -229    // i4Y
            },
            // D65
            {
                51,    // i4X
                -205    // i4Y
            },
		// DF
		{
			-5,	// i4X
			-299	// i4Y
    		}
    	},
	// AWB gain of AWB light source
	{
		// Strobe
            {
                907,    // i4R
                512,    // i4G
                717    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                622,    // i4G
                1610    // i4B
            },
            // A 
            {
                512,    // i4R
                540,    // i4G
                1180    // i4B
            },
            // TL84 
            {
                603,    // i4R
                512,    // i4G
                964    // i4B
            },
            // CWF 
            {
                733,    // i4R
                512,    // i4G
                973    // i4B
            },
            // DNP 
            {
                699,    // i4R
                512,    // i4G
                711    // i4B
            },
            // D65 
            {
                751,    // i4R
                512,    // i4G
                584    // i4B
            },
            // DF 
            {
                823,    // i4R
                512,    // i4G
                706    // i4B
		}
	},
    	// Rotation matrix parameter
    	{
            12,    // i4RotationAngle
            250,    // i4Cos
            53    // i4Sin
        },
        // Daylight locus parameter
        {
            -194,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            76,    // i4RightBound
            -105,    // i4LeftBound
            -390,    // i4UpperBound
            -450    // i4LowerBound
            },
            // Tungsten
            {
            -280,    // i4RightBound
            -930,    // i4LeftBound
            -142,    // i4UpperBound
            -242    // i4LowerBound
            },
            // Warm fluorescent
            {
            -280,    // i4RightBound
            -930,    // i4LeftBound
            -242,    // i4UpperBound
            -362    // i4LowerBound
            },
            // Fluorescent
            {
            -105,    // i4RightBound
            -280,    // i4LeftBound
            -133,    // i4UpperBound
            -295    // i4LowerBound
            },
            // CWF
            {
            -105,    // i4RightBound
            -280,    // i4LeftBound
            -295,    // i4UpperBound
            -390    // i4LowerBound
            },
            // Daylight
            {
            76,    // i4RightBound
            -105,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Shade
            {
            436,    // i4RightBound
            76,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            76,    // i4RightBound
            -105,    // i4LeftBound
            -285,    // i4UpperBound
            -390    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            436,    // i4RightBound
            -930,    // i4LeftBound
            -125,    // i4UpperBound
            -450    // i4LowerBound
            },
            // Daylight
            {
            101,    // i4RightBound
            -105,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Cloudy daylight
            {
            201,    // i4RightBound
            26,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Shade
            {
            301,    // i4RightBound
            26,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Twilight
            {
            -105,    // i4RightBound
            -265,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Fluorescent
            {
            101,    // i4RightBound
            -330,    // i4LeftBound
            -155,    // i4UpperBound
            -390    // i4LowerBound
            },
            // Warm fluorescent
            {
            -256,    // i4RightBound
            -456,    // i4LeftBound
            -155,    // i4UpperBound
            -390    // i4LowerBound
            },
            // Incandescent
            {
            -256,    // i4RightBound
            -456,    // i4LeftBound
            -125,    // i4UpperBound
            -285    // i4LowerBound
            },
            // Gray World
            {
            5000,    // i4RightBound
            -5000,    // i4LeftBound
            5000,    // i4UpperBound
            -5000    // i4LowerBound
            }
        },
        // PWB default gain	
        {
            // Daylight
            {
            710,    // i4R
            512,    // i4G
            636    // i4B
            },
            // Cloudy daylight
            {
            802,    // i4R
            512,    // i4G
            529    // i4B
            },
            // Shade
            {
            845,    // i4R
            512,    // i4G
            488    // i4B
            },
            // Twilight
            {
            587,    // i4R
            512,    // i4G
            854    // i4B
            },
            // Fluorescent
            {
            704,    // i4R
            512,    // i4G
            818    // i4B
            },
            // Warm fluorescent
            {
            547,    // i4R
            512,    // i4G
            1207    // i4B
            },
            // Incandescent
            {
            491,    // i4R
            512,    // i4G
            1124    // i4B
            },
            // Gray World
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        // AWB preference color	
        {
            // Tungsten
            {
            0,    // i4SliderValue
            6373    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5609    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1348    // i4OffsetThr
            },
            // Daylight WB gain
            {
            672,    // i4R
            512,    // i4G
            693    // i4B
            },
            // Preference gain: strobe
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: tungsten
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: warm fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: CWF
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: shade
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            },
            // Preference gain: daylight fluorescent
            {
            512,    // i4R
            512,    // i4G
            512    // i4B
            }
        },
        {// CCT estimation
            {// CCT
                2300,    // i4CCT[0]
                2850,    // i4CCT[1]
                4100,    // i4CCT[2]
                5100,    // i4CCT[3]
                6500    // i4CCT[4]
            },
            {// Rotated X coordinate
                -522,    // i4RotatedXCoordinate[0]
                -407,    // i4RotatedXCoordinate[1]
                -281,    // i4RotatedXCoordinate[2]
                -106,    // i4RotatedXCoordinate[3]
                0    // i4RotatedXCoordinate[4]
    		}
    	}
    },
	{0}
};
 
#include INCLUDE_FILENAME_ISP_LSC_PARAM
//};  //  namespace

const CAMERA_TSF_TBL_STRUCT CAMERA_TSF_DEFAULT_VALUE =
{
    #include INCLUDE_FILENAME_TSF_PARA
    #include INCLUDE_FILENAME_TSF_DATA
};


typedef NSFeature::RAWSensorInfo<SENSOR_ID> SensorInfoSingleton_T;


namespace NSFeature {
template <>
UINT32
SensorInfoSingleton_T::
impGetDefaultData(CAMERA_DATA_TYPE_ENUM const CameraDataType, VOID*const pDataBuf, UINT32 const size) const
{
    UINT32 dataSize[CAMERA_DATA_TYPE_NUM] = {sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT),
                                             sizeof(NVRAM_CAMERA_3A_STRUCT),
                                             sizeof(NVRAM_CAMERA_SHADING_STRUCT),
                                             sizeof(NVRAM_LENS_PARA_STRUCT),
                                             sizeof(AE_PLINETABLE_T),
                                             0,
                                             sizeof(CAMERA_TSF_TBL_STRUCT)};

    if (CameraDataType > CAMERA_DATA_TSF_TABLE || NULL == pDataBuf || (size < dataSize[CameraDataType]))
    {
        return 1;
    }

    switch(CameraDataType)
    {
        case CAMERA_NVRAM_DATA_ISP:
            memcpy(pDataBuf,&CAMERA_ISP_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_ISP_PARAM_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_3A:
            memcpy(pDataBuf,&CAMERA_3A_NVRAM_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_3A_STRUCT));
            break;
        case CAMERA_NVRAM_DATA_SHADING:
            memcpy(pDataBuf,&CAMERA_SHADING_DEFAULT_VALUE,sizeof(NVRAM_CAMERA_SHADING_STRUCT));
            break;
        case CAMERA_DATA_AE_PLINETABLE:
            memcpy(pDataBuf,&g_PlineTableMapping,sizeof(AE_PLINETABLE_T));
            break;
        case CAMERA_DATA_TSF_TABLE:
            memcpy(pDataBuf,&CAMERA_TSF_DEFAULT_VALUE,sizeof(CAMERA_TSF_TBL_STRUCT));
            break;
        default:
            break;
    }
    return 0;
}};  //  NSFeature


