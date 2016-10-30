#ifndef AP_EDTIOR_DATA_ITEM_SYSTEM_H
#define AP_EDTIOR_DATA_ITEM_SYSTEM_H

#include "NVRAM_Data_Type.h"
#include "cfg_module_file.h"
#include "CFG_file_lid.h"
//#include "CFG_File_LID.h"

// add by chipeng , temporary modify
//#include "../../custom/oppo/cgen/inc/Custom_NvRam_LID.h"
#include "Custom_NvRam_LID.h"

/***************************************************************************** 
* Include
*****************************************************************************/

/***************************************************************************** 
* Define
*****************************************************************************/
typedef struct
{
	char	cFileVer[4];
}File_Ver_Struct;



/***************************************************************************** 
* META Description
*****************************************************************************/
BEGIN_NVRAM_DATA


/***********************************************************************
  ***  This is a nvram data item bit level description for meta tools nvram editor
  ***
  ***  Logical Data Item ID : AP_CFG_FILE_AUXADC_LID
  ***
  ***  Module: 
  ***
  ***  Description:  
  ***
  ***  Maintainer: 
  ***
  ***********************************************************************/
LID_BIT VER_LID(AP_CFG_RDCL_FILE_AUXADC_LID)
     AUXADC_CFG_Struct *CFG_FILE_AUXADC_REC_TOTAL
     {
 
     };
     
LID_BIT VER_LID(AP_CFG_RDEB_FILE_BT_ADDR_LID)
     ap_nvram_btradio_mt6610_struct *CFG_FILE_BT_ADDR_REC_TOTAL
     {
 
     };      

LID_BIT VER_LID(AP_CFG_RDCL_FACTORY_LID)     
    FACTORY_CFG_Struct *CFG_FILE_FACTORY_REC_TOTAL     
    {

    };  

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_PARA_LID)     
    NVRAM_CAMERA_PARA_STRUCT *CFG_FILE_CAMERA_PARA_REC_TOTAL     
    {

    }; 

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_3A_LID)     
    NVRAM_CAMERA_3A_STRUCT *CFG_FILE_CAMERA_3A_REC_TOTAL     
    {

    }; 

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_SHADING_LID)     
    NVRAM_CAMERA_SHADING_STRUCT *CFG_FILE_CAMERA_SHADING_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_DEFECT_LID)     
    NVRAM_CAMERA_DEFECT_STRUCT *CFG_FILE_CAMERA_DEFECT_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_SENSOR_LID)     
    NVRAM_SENSOR_DATA_STRUCT *CFG_FILE_CAMERA_SENSOR_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_CAMERA_LENS_LID)     
    NVRAM_LENS_PARA_STRUCT *CFG_FILE_CAMERA_LENS_REC_TOTAL     
    {

    };

LID_BIT VER_LID(AP_CFG_RDCL_BWCS_LID)
    ap_nvram_bwcs_config_struct *CFG_FILE_BWCS_CONFIG_TOTAL
    {
        
    }; 
     
LID_BIT VER_LID(AP_CFG_RDCL_HWMON_ACC_LID)
    NVRAM_HWMON_ACC_STRUCT *CFG_FILE_HWMON_ACC_REC_TOTAL
    {
        
    }; 
      
/* BUG1039 - Add NVRAM */
LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_P_SENSOR_THRESHOLD_RESULT_LID)
   File_P_SENSOR_THRESHOLD_Struct *CFG_P_SENSOR_THRESHOLD_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_CUST_IN_SW_LID)
   File_CUST_IN_SW_Struct *CFG_FILE_CUST_IN_SW_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_CUST_COUNTRY_CODE_LID)
   File_CUST_COUNTRY_CODE_Struct *CFG_FILE_CUST_COUNTRY_CODE_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_CUST_PCB_SW_LID)
   File_CUST_PCB_SW_Struct *CFG_FILE_CUST_PCB_SW_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_CCI_SW_LID)
   File_CCI_SW_Struct *CFG_FILE_CCI_SW_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_CUST_EX_SW_LID)
   File_CUST_EX_SW_Struct *CFG_FILE_CUST_EX_SW_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_HW_LID)
   File_HW_Struct *CFG_FILE_HW_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_FTM_TEST_RESULT_LID)
   File_FTM_TEST_RESULT_Struct *CFG_FILE_FTM_TEST_RESULT_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_REEB_FILE_WARRANTY_INFO_LID)
   File_Warranty_Struct *CFG_FILE_WARRANTY_INFO_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_TWEAKER_LID)
   File_TWEAKER_Struct *CFG_FILE_TWEAKER_REC_TOTAL
   {

   };

LID_BIT VER_LID(AP_CFG_CUSTOM_FILE_OPERATOR_LID)
   File_OPERATOR_Struct *CFG_FILE_OPERATOR_REC_TOTAL
   {

   };
/* BUG1039 - Add NVRAM */

END_NVRAM_DATA

#endif /* AP_EDTIOR_DATA_ITEM_SYSTEM_H */ 

