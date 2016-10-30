

 ************************************************************************************************/
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
        69575,    // i4R_AVG
        14826,    // i4R_STD
        93450,    // i4B_AVG
        24335,    // i4B_STD
        {  // i4P00[9]
            5135000, -2007500, -567500, -720000, 3307500, -30000, 85000, -2002500, 4475000
        },
        {  // i4P10[9]
            1023177, -1112824, 77520, 35114, -212102, 181858, 73070, 550820, -619020
        },
        {  // i4P01[9]
            502676, -578116, 60273, -70138, -317612, 388306, 23909, -133761, 110408
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
            1195,   // u4MinGain, 1024 base =  1x
            16384,  // u4MaxGain, 16x
            55,     // u4MiniISOGain, ISOxx
            128,    // u4GainStepUnit, 1x/8
            33,     // u4PreExpUnit
            30,     // u4PreMaxFrameRate
            33,     // u4VideoExpUnit
            30,     // u4VideoMaxFrameRate
            1024,   // u4Video2PreRatio, 1024 base = 1x
            58,     // u4CapExpUnit
            15,     // u4CapMaxFrameRate
            1024,   // u4Cap2PreRatio, 1024 base = 1x
            28,      // u4LensFno, Fno = 2.8
            350     // u4FocusLength_100x
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
            TRUE,    // bEnableBlackLight
            TRUE,    // bEnableHistStretch
            FALSE,    // bEnableAntiOverExposure
            TRUE,    // bEnableTimeLPF
            FALSE,    // bEnableCaptureThres
            FALSE,    // bEnableVideoThres
            FALSE,    // bEnableStrobeThres
            47,    // u4AETarget
            0,    // u4StrobeAETarget
            50,    // u4InitIndex
            4,    // u4BackLightWeight
            32,    // u4HistStretchWeight
            4,    // u4AntiOverExpWeight
            2,    // u4BlackLightStrengthIndex
            2,    // u4HistStretchStrengthIndex
            2,    // u4AntiOverExpStrengthIndex
            2,    // u4TimeLPFStrengthIndex
            {1, 3, 5, 7, 8},    // u4LPFConvergeTable[AE_CCT_STRENGTH_NUM] 
            90,    // u4InDoorEV = 9.0, 10 base 
            -10,    // i4BVOffset delta BV = value/10 
            108, //64,    // u4PreviewFlareOffset
            108, //64,    // u4CaptureFlareOffset
            5,    // u4CaptureFlareThres
            108, //64,    // u4VideoFlareOffset
            5,    // u4VideoFlareThres
            108, // 2,    // u4StrobeFlareOffset
            2,    // u4StrobeFlareThres
            8,    // u4PrvMaxFlareThres
            0,    // u4PrvMinFlareThres
            8,    // u4VideoMaxFlareThres
            0,    // u4VideoMinFlareThres
            18,    // u4FlatnessThres    // 10 base for flatness condition.
            50    // u4FlatnessStrength
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
                827,    // i4R
                512,    // i4G
                607    // i4B
            }
        },
        // Original XY coordinate of AWB light source
        {
           // Strobe
            {
                114,    // i4X
                -240    // i4Y
            },
            // Horizon
            {
                -319,    // i4X
                -302    // i4Y
            },
            // A
            {
                -319,    // i4X
                -302    // i4Y
            },
            // TL84
            {
                -139,    // i4X
                -328    // i4Y
            },
            // CWF
            {
                -70,    // i4X
                -391    // i4Y
            },
            // DNP
            {
                32,    // i4X
                -264    // i4Y
            },
            // D65
            {
                114,    // i4X
                -240    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
    	},
    	// Rotated XY coordinate of AWB light source
    	{
            // Strobe
            {
                71,    // i4X
                -256    // i4Y
            },
            // Horizon
            {
                -366,    // i4X
                -242    // i4Y
            },
            // A
            {
                -366,    // i4X
                -242    // i4Y
            },
            // TL84
            {
                -193,    // i4X
                -299    // i4Y
            },
            // CWF
            {
                -136,    // i4X
                -373    // i4Y
            },
            // DNP
            {
                -14,    // i4X
                -265    // i4Y
            },
            // D65
            {
                71,    // i4X
                -256    // i4Y
            },
            // DF
            {
                0,    // i4X
                0    // i4Y
            }
        },
        // AWB gain of AWB light source
        {
            // Strobe 
            {
                827,    // i4R
                512,    // i4G
                607    // i4B
            },
            // Horizon 
            {
                512,    // i4R
                524,    // i4G
                1214    // i4B
            },
            // A 
            {
                512,    // i4R
                524,    // i4G
                1214    // i4B
            },
            // TL84 
            {
                661,    // i4R
                512,    // i4G
                964    // i4B
            },
            // CWF 
            {
                791,    // i4R
                512,    // i4G
                955    // i4B
            },
            // DNP 
            {
                764,    // i4R
                512,    // i4G
                701    // i4B
            },
            // D65 
            {
                827,    // i4R
                512,    // i4G
                607    // i4B
            },
            // DF 
            {
                512,    // i4R
                512,    // i4G
                512    // i4B
            }
        },
        // Rotation matrix parameter
        {
            10,    // i4RotationAngle
            252,    // i4Cos
            44    // i4Sin
        },
        // Daylight locus parameter
        {
            -179,    // i4SlopeNumerator
            128    // i4SlopeDenominator
        },
        // AWB light area
        {
            // Strobe:FIXME
            {
            0,    // i4RightBound
            0,    // i4LeftBound
            0,    // i4UpperBound
            0    // i4LowerBound
            },
            // Tungsten
            {
            -303, // -243,    // i4RightBound
            -893,    // i4LeftBound
            -142,    // i4UpperBound
            -292    // i4LowerBound
            },
            // Warm fluorescent
            {
            -303, // -243,    // i4RightBound
            -893,    // i4LeftBound
            -292,    // i4UpperBound
            -412    // i4LowerBound
            },
            // Fluorescent
            {
            -64,    // i4RightBound
            -303, // -243,    // i4LeftBound
            -184,    // i4UpperBound
            -336    // i4LowerBound
            },
            // CWF
            {
            -64,    // i4RightBound
            -303, // -243,    // i4LeftBound
            -336,    // i4UpperBound
            -423    // i4LowerBound
            },
            // Daylight
            {
            96,    // i4RightBound
            -64,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Shade
            {
            456,    // i4RightBound
            96,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Daylight Fluorescent
            {
            96,    // i4RightBound
            -64,    // i4LeftBound
            -336,    // i4UpperBound
            -423    // i4LowerBound
            }
        },
        // PWB light area
        {
            // Reference area
            {
            456,    // i4RightBound
            -893,    // i4LeftBound
            0,    // i4UpperBound
            -423    // i4LowerBound
            },
            // Daylight
            {
            121,    // i4RightBound
            -64,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Cloudy daylight
            {
            221,    // i4RightBound
            46,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Shade
            {
            321,    // i4RightBound
            146,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Twilight
            {
            -64,    // i4RightBound
            -224,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
            },
            // Fluorescent
            {
            121,    // i4RightBound
            -293,    // i4LeftBound
            -206,    // i4UpperBound
            -423    // i4LowerBound
            },
            // Warm fluorescent
            {
            -266,    // i4RightBound
            -466,    // i4LeftBound
            -206,    // i4UpperBound
            -423    // i4LowerBound
            },
            // Incandescent
            {
            -266,    // i4RightBound
            -466,    // i4LeftBound
            -176,    // i4UpperBound
            -336    // i4LowerBound
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
            789,    // i4R
            512,    // i4G
            649    // i4B
            },
            // Cloudy daylight
            {
            886,    // i4R
            512,    // i4G
            551    // i4B
            },
            // Shade
            {
            1036,//936,    // i4R
            512,    // i4G
            489//509    // i4B
            },
            // Twilight
            {
            653,    // i4R
            512,    // i4G
            851    // i4B
            },
            // Fluorescent
            {
            763,    // i4R
            512,    // i4G
            828    // i4B
            },
            // Warm fluorescent
            {
            560,    // i4R
            512,    // i4G
            1285    // i4B
            },
            // Incandescent
            {
            511,    // i4R
            512,    // i4G
            1205    // i4B
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
            7175    // i4OffsetThr
            },
            // Warm fluorescent	
            {
            0,    // i4SliderValue
            5341    // i4OffsetThr
            },
            // Shade
            {
            0,    // i4SliderValue
            1346    // i4OffsetThr
            },
            // Daylight WB gain
            {
            753,    // i4R
            512,    // i4G
            694    // i4B
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
                -437,    // i4RotatedXCoordinate[0]
                -437,    // i4RotatedXCoordinate[1]
                -264,    // i4RotatedXCoordinate[2]
                -85,    // i4RotatedXCoordinate[3]
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


