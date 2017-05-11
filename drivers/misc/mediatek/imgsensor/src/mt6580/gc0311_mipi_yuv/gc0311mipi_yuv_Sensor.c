/*****************************************************************************
 *
 * Filename:
 * ---------
 *   gc0311yuv_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *   V1.2.3
 *
 * Author:
 * -------
 *   Leo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2012.02.29  kill bugs
 *   
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gc0311mipi_yuv_Sensor.h"
#include "gc0311mipi_yuv_Camera_Sensor_para.h"
#include "gc0311mipi_yuv_CameraCustomized.h"
#include "cust_gpio_usage.h"


static DEFINE_SPINLOCK(GC0311_drv_lock);


//#define GC0311YUV_DEBUG

#define MAIN_CAMERA_ID_STROBE_PIN GPIO_CAMERA_KEY2_PIN //hcz


#ifdef GC0311YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define GC0311_TEST_PATTERN_CHECKSUM (0xfd769299)

kal_bool GC0311_night_mode_enable = KAL_FALSE;
kal_uint16 GC0311_CurStatus_AWB = 0;


static void GC0311_awb_enable(kal_bool enalbe);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 GC0311_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 2, GC0311_WRITE_ID);

}
kal_uint16 GC0311_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, GC0311_READ_ID);
	
    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   GC0311_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 GC0311_dummy_pixels = 0, GC0311_dummy_lines = 0;
kal_bool   GC0311_MODE_CAPTURE = KAL_FALSE;
kal_bool   GC0311_NIGHT_MODE = KAL_FALSE;

static kal_uint32 MINFramerate = 0;
static kal_uint32 MAXFramerate = 0;

kal_uint32 GC0311_isp_master_clock;
static kal_uint32 GC0311_g_fPV_PCLK = 30 * 1000000;

kal_uint8 GC0311_sensor_write_I2C_address = GC0311_WRITE_ID;
kal_uint8 GC0311_sensor_read_I2C_address = GC0311_READ_ID;

UINT8 GC0311PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT GC0311SensorConfigData;

#define GC0311_SET_PAGE0 	GC0311_write_cmos_sensor(0xfe, 0x00)
#define GC0311_SET_PAGE1 	GC0311_write_cmos_sensor(0xfe, 0x01)
#define GC0311_SET_PAGE2 	GC0311_write_cmos_sensor(0xfe, 0x02)
#define GC0311_SET_PAGE3 	GC0311_write_cmos_sensor(0xfe, 0x03)

/*************************************************************************
 * FUNCTION
 *	GC0311_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of GC0311 to change exposure time.
 *
 * PARAMETERS
 *   iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0311_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_GC0311_Shutter */


/*************************************************************************
 * FUNCTION
 *	GC0311_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of GC0311 .
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0311_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = GC0311_read_cmos_sensor(0x04);
	temp_reg2 = GC0311_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* GC0311_read_shutter */

static void GC0311_Set_Mirrorflip(kal_uint8 image_mirror)
{
	SENSORDB("image_mirror = %d\n", image_mirror);
	
	switch (image_mirror)
	{
		case IMAGE_NORMAL://IMAGE_NORMAL:
			GC0311_write_cmos_sensor(0x17,0x14);//bit[1][0]
	//		write_cmos_sensor(0x92,0x03);
	//		write_cmos_sensor(0x94,0x0b);
			break;
		case IMAGE_H_MIRROR://IMAGE_H_MIRROR:
			GC0311_write_cmos_sensor(0x17,0x15);
	//		GC2355_write_cmos_sensor(0x92,0x03);
	//		GC2355_write_cmos_sensor(0x94,0x0b);
			break;
		case IMAGE_V_MIRROR://IMAGE_V_MIRROR:
			GC0311_write_cmos_sensor(0x17,0x16);
	//		GC2355_write_cmos_sensor(0x92,0x02);
	//		GC2355_write_cmos_sensor(0x94,0x0b);
			break;
		case IMAGE_HV_MIRROR://IMAGE_HV_MIRROR:
			GC0311_write_cmos_sensor(0x17,0x17);
	//		GC2355_write_cmos_sensor(0x92,0x02);
	//		GC2355_write_cmos_sensor(0x94,0x0b);
			break;
	}


}

static void GC0311_set_AE_mode(kal_bool AE_enable)
{

	SENSORDB("[GC0311]enter GC0311_set_AE_mode function, AE_enable = %d\n ", AE_enable);
    if (AE_enable == KAL_TRUE)
    {
        // turn on AEC/AGC
			SENSORDB("[GC0311]enter GC0311_set_AE_mode function 1\n ");
			GC0311_write_cmos_sensor(0xfe, 0x00);
			GC0311_write_cmos_sensor(0x4f, 0x01);
    }
    else
    {
        // turn off AEC/AGC
			SENSORDB("[GC0311]enter GC0311_set_AE_mode function 2\n ");        
			GC0311_write_cmos_sensor(0xfe, 0x00);
			GC0311_write_cmos_sensor(0x4f, 0x00);
    }   
}


void GC0311_set_contrast(UINT16 para)
{   
    SENSORDB("[GC0311]CONTROLFLOW enter GC0311_set_contrast function:\n ");
#if 0
    switch (para)
    {
        case ISP_CONTRAST_LOW:			 
			GC0311_write_cmos_sensor(0xfe, 0x00);	 
			GC0311_write_cmos_sensor(0xd3, 0x30);
			GC0311_write_cmos_sensor(0xfe, 0x00); 
			Sleep(200);
			break;
        case ISP_CONTRAST_HIGH:			 
			GC0311_write_cmos_sensor(0xfe, 0x00); 	
			GC0311_write_cmos_sensor(0xd3, 0x60);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			Sleep(200);			
			break;
        case ISP_CONTRAST_MIDDLE: 
        default:
			GC0311_write_cmos_sensor(0xfe, 0x00);	 
			GC0311_write_cmos_sensor(0xd3, 0x40);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			Sleep(200);			
			break;
        //default:
		//	break;
    }
	#endif  
    SENSORDB("[GC0311]exit GC0311_set_contrast function:\n ");
    return;
  
}

UINT32 GC0311_MIPI_SetMaxFramerateByScenario(
  MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 frameRate)
{
	SENSORDB("scenarioId = %d\n", scenarioId);
}


UINT32 GC0311SetTestPatternMode(kal_bool bEnable)
{
	SENSORDB("[GC0311SetTestPatternMode]test pattern bEnable:=%d\n",bEnable);

	if(bEnable)
	{
		GC0311_write_cmos_sensor(0xfe,0x00);
		GC0311_write_cmos_sensor(0x40,0x08); 
		GC0311_write_cmos_sensor(0x41,0x00); 
		GC0311_write_cmos_sensor(0x42,0x00); 
		GC0311_write_cmos_sensor(0x4f,0x00); 
		GC0311_write_cmos_sensor(0xfe,0x00); 
		GC0311_write_cmos_sensor(0x03,0x03);
		GC0311_write_cmos_sensor(0x04,0x9c); 
		GC0311_write_cmos_sensor(0x71,0x20); 
		GC0311_write_cmos_sensor(0x72,0x40); 
		GC0311_write_cmos_sensor(0x73,0x80); 
		GC0311_write_cmos_sensor(0x74,0x80); 
		GC0311_write_cmos_sensor(0x75,0x80);
		GC0311_write_cmos_sensor(0x76,0x80); 
		GC0311_write_cmos_sensor(0x7a,0x80);
		GC0311_write_cmos_sensor(0x7b,0x80);
		GC0311_write_cmos_sensor(0x7c,0x80); 
		GC0311_write_cmos_sensor(0x77,0x40); 
		GC0311_write_cmos_sensor(0x78,0x40); 
		GC0311_write_cmos_sensor(0x79,0x40); 
		GC0311_write_cmos_sensor(0xfe,0x00); 
		GC0311_write_cmos_sensor(0xfe,0x01); 
		GC0311_write_cmos_sensor(0x12,0x00); 
		GC0311_write_cmos_sensor(0x13,0x30); 
		GC0311_write_cmos_sensor(0x44,0x00); 
		GC0311_write_cmos_sensor(0x45,0x00); 
		GC0311_write_cmos_sensor(0xfe,0x00); 
		GC0311_write_cmos_sensor(0xd0,0x40); 
		GC0311_write_cmos_sensor(0xd1,0x20); 
		GC0311_write_cmos_sensor(0xd2,0x20);
		GC0311_write_cmos_sensor(0xd3,0x40); 
		GC0311_write_cmos_sensor(0xd5,0x00); 
		GC0311_write_cmos_sensor(0xd8,0x00); 
		GC0311_write_cmos_sensor(0xdd,0x00); 
		GC0311_write_cmos_sensor(0xde,0x00);
		GC0311_write_cmos_sensor(0xe4,0x00); 
		GC0311_write_cmos_sensor(0xeb,0x00); 
		GC0311_write_cmos_sensor(0xa4,0x00); 
		GC0311_write_cmos_sensor(0x4c,0x21); 
		
		GC0311_write_cmos_sensor(0xfe,0x00);


	}
	else
	{
		GC0311_write_cmos_sensor(0xfe, 0x00);	
		GC0311_write_cmos_sensor(0x4c, 0x20);
		GC0311_write_cmos_sensor(0xfe, 0x00);
	}
	
	return ERROR_NONE;
}


void GC0311_set_brightness(UINT16 para)
{

	SENSORDB("[GC0311]CONTROLFLOW enter GC0311_set_brightness function:\n ");
#if 1
	//return;
    switch (para)
    {
        case ISP_BRIGHT_LOW:
		//case AE_EV_COMP_n13:
			GC0311_write_cmos_sensor(0xd5, 0xc0);
			Sleep(200);
			
		//	GC0311_SET_PAGE1;
		//	GC0311_write_cmos_sensor(0x13, 0x30);
			GC0311_SET_PAGE0;
		break;
        case ISP_BRIGHT_HIGH:
		//case AE_EV_COMP_13:
			GC0311_write_cmos_sensor(0xd5, 0x40);
			Sleep(200);
		//	GC0311_SET_PAGE1;
		//	GC0311_write_cmos_sensor(0x13, 0x90);
		//	GC0311_SET_PAGE0;
			break;
        case ISP_BRIGHT_MIDDLE:
        default:
		//case AE_EV_COMP_00:
			GC0311_write_cmos_sensor(0xd5, 0x00);
			Sleep(200);
		//	GC0311_SET_PAGE1;
		//	GC0311_write_cmos_sensor(0x13, 0x60);
		//	GC0311_SET_PAGE0;
		break;
		//	return KAL_FALSE;
		//	break;
    }

    SENSORDB("[GC0311]exit GC0311_set_brightness function:\n ");
    return;
	#endif 
   
}
void GC0311_set_saturation(UINT16 para)
{
	SENSORDB("[GC0311]CONTROLFLOW enter GC0311_set_saturation function:\n ");

    switch (para)
    {
        case ISP_SAT_HIGH:
			GC0311_write_cmos_sensor(0xfe, 0x00); 	
			GC0311_write_cmos_sensor(0xd1, 0x45);
			GC0311_write_cmos_sensor(0xd2, 0x45);			
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
        case ISP_SAT_LOW:
			GC0311_write_cmos_sensor(0xfe, 0x00); 	
			GC0311_write_cmos_sensor(0xd1, 0x28);
			GC0311_write_cmos_sensor(0xd2, 0x28);	
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
        case ISP_SAT_MIDDLE:
      GC0311_write_cmos_sensor(0xfe, 0x00); 	
			GC0311_write_cmos_sensor(0xd1, 0x34);
			GC0311_write_cmos_sensor(0xd2, 0x34);	
			GC0311_write_cmos_sensor(0xfe, 0x00);
        default:
			GC0311_write_cmos_sensor(0xfe, 0x00); 	
			GC0311_write_cmos_sensor(0xd1, 0x34);//34
			GC0311_write_cmos_sensor(0xd2, 0x34);	//34
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
		//	return KAL_FALSE;
		//	break;
    }

	SENSORDB("[GC0311]exit GC0311_set_saturation function:\n ");
     return;
    
}

void GC0311_set_iso(UINT16 para)
{

	SENSORDB("[GC0311]CONTROLFLOW GC0311_set_iso:\n ");
    switch (para)
	{
        case AE_ISO_100:
             //ISO 100
			GC0311_write_cmos_sensor(0xfe, 0x01); 	
			GC0311_write_cmos_sensor(0x44, 0x00);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
        case AE_ISO_200:
             //ISO 200
			GC0311_write_cmos_sensor(0xfe, 0x01); 	
			GC0311_write_cmos_sensor(0x44, 0x01);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
        case AE_ISO_400:
             //ISO 400
			GC0311_write_cmos_sensor(0xfe, 0x01); 	
			GC0311_write_cmos_sensor(0x44, 0x02);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
		case AE_ISO_AUTO:
		default:
			GC0311_write_cmos_sensor(0xfe, 0x01); 	
			GC0311_write_cmos_sensor(0x44, 0x02);
			GC0311_write_cmos_sensor(0xfe, 0x00);
			break;
	}
    return;  
}



void GC0311StreamOn(void)
{
	//Sleep(150);
    GC0311_write_cmos_sensor(0xfe,0x03);
    GC0311_write_cmos_sensor(0x10,0x94); 
    GC0311_write_cmos_sensor(0xfe,0x00);    
    Sleep(50);	
}

void GC0311_MIPI_GetDelayInfo(UINT32 *delayAddr)
{
    SENSOR_DELAY_INFO_STRUCT* pDelayInfo = (SENSOR_DELAY_INFO_STRUCT*)delayAddr;
    pDelayInfo->InitDelay = 2;
    pDelayInfo->EffectDelay = 2;
    pDelayInfo->AwbDelay = 2;
    pDelayInfo->AFSwitchDelayFrame = 50;
}

UINT32 GC0311_MIPI_GetDefaultFramerateByScenario(
  MSDK_SCENARIO_ID_ENUM scenarioId, MUINT32 *pframeRate)
{
    switch (scenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
             *pframeRate = 300;
             break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        case MSDK_SCENARIO_ID_CAMERA_ZSD:
             *pframeRate = 300;
             break;
        case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
        case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
        case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added
             *pframeRate = 300;
             break;
        default:
             *pframeRate = 300;
          break;
    }

  return ERROR_NONE;
}

void GC0311_MIPI_SetMaxMinFps(UINT32 u2MinFrameRate, UINT32 u2MaxFrameRate)
{
	SENSORDB("GC0311_MIPI_SetMaxMinFps+ :FrameRate= %d %d\n",u2MinFrameRate,u2MaxFrameRate);
	spin_lock(&GC0311_drv_lock);
	MINFramerate = u2MinFrameRate;
	MAXFramerate = u2MaxFrameRate;
	spin_unlock(&GC0311_drv_lock);
	return;
}

void GC0311_3ACtrl(ACDK_SENSOR_3A_LOCK_ENUM action)
{
	SENSORDB("[GC0329]enter ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
   switch (action)
   {
      case SENSOR_3A_AE_LOCK:
          GC0311_set_AE_mode(KAL_FALSE);
      break;
      case SENSOR_3A_AE_UNLOCK:
          GC0311_set_AE_mode(KAL_TRUE);
      break;

      case SENSOR_3A_AWB_LOCK:
          GC0311_awb_enable(KAL_FALSE);       
      break;

      case SENSOR_3A_AWB_UNLOCK:
		   if (((AWB_MODE_OFF == GC0311_CurStatus_AWB) ||
        		(AWB_MODE_AUTO == GC0311_CurStatus_AWB)))
        	{	
         			 GC0311_awb_enable(KAL_TRUE);
         	}
      break;
      default:
      	break;
   }
   SENSORDB("[GC0329]exit ACDK_SENSOR_3A_LOCK_ENUM function:action=%d\n",action);
   return;
}


void GC0311_MIPI_GetExifInfo(UINT32 *exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
//    pExifInfo->AEISOSpeed = GC0311_Driver.isoSpeed;
//    pExifInfo->AWBMode = S5K4ECGX_Driver.awbMode;
//    pExifInfo->CapExposureTime = S5K4ECGX_Driver.capExposureTime;
//    pExifInfo->FlashLightTimeus = 0;
//   pExifInfo->RealISOValue = (S5K4ECGX_MIPI_ReadGain()*57) >> 8;
        //S5K4ECGX_Driver.isoSpeed;
}


void GC0311_MIPI_get_AEAWB_lock(UINT32 *pAElockRet32,UINT32 *pAWBlockRet32)
{
    *pAElockRet32 = 1;
    *pAWBlockRet32 = 1;
    SENSORDB("[GC0311]GetAEAWBLock,AE=%d ,AWB=%d\n,",*pAElockRet32,*pAWBlockRet32);
}


/*************************************************************************
 * FUNCTION
 *	GC0311_write_reg
 *
 * DESCRIPTION
 *	This function set the register of GC0311.
 *
 * PARAMETERS
 *	addr : the register index of GC0311
 *  para : setting parameter of the specified register of GC0311
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0311_write_reg(kal_uint32 addr, kal_uint32 para)
{
	GC0311_write_cmos_sensor(addr, para);
} /* GC0311_write_reg() */


/*************************************************************************
 * FUNCTION
 *	GC0311_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from GC0311.
 *
 * PARAMETERS
 *	addr : the register index of GC0311
 *
 * RETURNS
 *	the data that read from GC0311
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 GC0311_read_reg(kal_uint32 addr)
{
	return GC0311_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	GC0311_awb_enable
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
static void GC0311_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;
	
	GC0311_write_cmos_sensor(0xfe, 0x00); 	
	temp_AWB_reg = GC0311_read_cmos_sensor(0x42);
	
	if (enalbe)
	{
		GC0311_write_cmos_sensor(0x42, (temp_AWB_reg |0x02));
	}
	else
	{
		GC0311_write_cmos_sensor(0x42, (temp_AWB_reg & (~0x02)));
	}
	GC0311_write_cmos_sensor(0xfe, 0x00); 	

}


/*************************************************************************
* FUNCTION
*	GC0311_GAMMA_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate GAMMA curve.
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

void GC0311GammaSelect(kal_uint32 GammaLvl)
{
	switch(GammaLvl)
	{
		case GC0311_RGB_Gamma_m1:						//smallest gamma curve
			GC0311_write_cmos_sensor(0xfe, 0x00);
			GC0311_write_cmos_sensor(0xbf, 0x06);
			GC0311_write_cmos_sensor(0xc0, 0x12);
			GC0311_write_cmos_sensor(0xc1, 0x22);
			GC0311_write_cmos_sensor(0xc2, 0x35);
			GC0311_write_cmos_sensor(0xc3, 0x4b);
			GC0311_write_cmos_sensor(0xc4, 0x5f);
			GC0311_write_cmos_sensor(0xc5, 0x72);
			GC0311_write_cmos_sensor(0xc6, 0x8d);
			GC0311_write_cmos_sensor(0xc7, 0xa4);
			GC0311_write_cmos_sensor(0xc8, 0xb8);
			GC0311_write_cmos_sensor(0xc9, 0xc8);
			GC0311_write_cmos_sensor(0xca, 0xd4);
			GC0311_write_cmos_sensor(0xcb, 0xde);
			GC0311_write_cmos_sensor(0xcc, 0xe6);
			GC0311_write_cmos_sensor(0xcd, 0xf1);
			GC0311_write_cmos_sensor(0xce, 0xf8);
			GC0311_write_cmos_sensor(0xcf, 0xfd);
			break;
		case GC0311_RGB_Gamma_m2:
			GC0311_write_cmos_sensor(0xBF, 0x08);
			GC0311_write_cmos_sensor(0xc0, 0x0F);
			GC0311_write_cmos_sensor(0xc1, 0x21);
			GC0311_write_cmos_sensor(0xc2, 0x32);
			GC0311_write_cmos_sensor(0xc3, 0x43);
			GC0311_write_cmos_sensor(0xc4, 0x50);
			GC0311_write_cmos_sensor(0xc5, 0x5E);
			GC0311_write_cmos_sensor(0xc6, 0x78);
			GC0311_write_cmos_sensor(0xc7, 0x90);
			GC0311_write_cmos_sensor(0xc8, 0xA6);
			GC0311_write_cmos_sensor(0xc9, 0xB9);
			GC0311_write_cmos_sensor(0xcA, 0xC9);
			GC0311_write_cmos_sensor(0xcB, 0xD6);
			GC0311_write_cmos_sensor(0xcC, 0xE0);
			GC0311_write_cmos_sensor(0xcD, 0xEE);
			GC0311_write_cmos_sensor(0xcE, 0xF8);
			GC0311_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0311_RGB_Gamma_m3:			
			GC0311_write_cmos_sensor(0xbf , 0x0b);
			GC0311_write_cmos_sensor(0xc0 , 0x17);
			GC0311_write_cmos_sensor(0xc1 , 0x2a);
			GC0311_write_cmos_sensor(0xc2 , 0x41);
			GC0311_write_cmos_sensor(0xc3 , 0x54);
			GC0311_write_cmos_sensor(0xc4 , 0x66);
			GC0311_write_cmos_sensor(0xc5 , 0x74);
			GC0311_write_cmos_sensor(0xc6 , 0x8c);
			GC0311_write_cmos_sensor(0xc7 , 0xa3);
			GC0311_write_cmos_sensor(0xc8 , 0xb5);
			GC0311_write_cmos_sensor(0xc9 , 0xc4);
			GC0311_write_cmos_sensor(0xca , 0xd0);
			GC0311_write_cmos_sensor(0xcb , 0xdb);
			GC0311_write_cmos_sensor(0xcc , 0xe5);
			GC0311_write_cmos_sensor(0xcd , 0xf0);
			GC0311_write_cmos_sensor(0xce , 0xf7);
			GC0311_write_cmos_sensor(0xcf , 0xff);
			break;
			
		case GC0311_RGB_Gamma_m4:
			GC0311_write_cmos_sensor(0xBF, 0x0E);
			GC0311_write_cmos_sensor(0xc0, 0x1C);
			GC0311_write_cmos_sensor(0xc1, 0x34);
			GC0311_write_cmos_sensor(0xc2, 0x48);
			GC0311_write_cmos_sensor(0xc3, 0x5A);
			GC0311_write_cmos_sensor(0xc4, 0x6B);
			GC0311_write_cmos_sensor(0xc5, 0x7B);
			GC0311_write_cmos_sensor(0xc6, 0x95);
			GC0311_write_cmos_sensor(0xc7, 0xAB);
			GC0311_write_cmos_sensor(0xc8, 0xBF);
			GC0311_write_cmos_sensor(0xc9, 0xCE);
			GC0311_write_cmos_sensor(0xcA, 0xD9);
			GC0311_write_cmos_sensor(0xcB, 0xE4);
			GC0311_write_cmos_sensor(0xcC, 0xEC);
			GC0311_write_cmos_sensor(0xcD, 0xF7);
			GC0311_write_cmos_sensor(0xcE, 0xFD);
			GC0311_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0311_RGB_Gamma_m5:
			GC0311_write_cmos_sensor(0xBF, 0x10);
			GC0311_write_cmos_sensor(0xc0, 0x20);
			GC0311_write_cmos_sensor(0xc1, 0x38);
			GC0311_write_cmos_sensor(0xc2, 0x4E);
			GC0311_write_cmos_sensor(0xc3, 0x63);
			GC0311_write_cmos_sensor(0xc4, 0x76);
			GC0311_write_cmos_sensor(0xc5, 0x87);
			GC0311_write_cmos_sensor(0xc6, 0xA2);
			GC0311_write_cmos_sensor(0xc7, 0xB8);
			GC0311_write_cmos_sensor(0xc8, 0xCA);
			GC0311_write_cmos_sensor(0xc9, 0xD8);
			GC0311_write_cmos_sensor(0xcA, 0xE3);
			GC0311_write_cmos_sensor(0xcB, 0xEB);
			GC0311_write_cmos_sensor(0xcC, 0xF0);
			GC0311_write_cmos_sensor(0xcD, 0xF8);
			GC0311_write_cmos_sensor(0xcE, 0xFD);
			GC0311_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0311_RGB_Gamma_m6:										// largest gamma curve
			GC0311_write_cmos_sensor(0xBF, 0x14);
			GC0311_write_cmos_sensor(0xc0, 0x28);
			GC0311_write_cmos_sensor(0xc1, 0x44);
			GC0311_write_cmos_sensor(0xc2, 0x5D);
			GC0311_write_cmos_sensor(0xc3, 0x72);
			GC0311_write_cmos_sensor(0xc4, 0x86);
			GC0311_write_cmos_sensor(0xc5, 0x95);
			GC0311_write_cmos_sensor(0xc6, 0xB1);
			GC0311_write_cmos_sensor(0xc7, 0xC6);
			GC0311_write_cmos_sensor(0xc8, 0xD5);
			GC0311_write_cmos_sensor(0xc9, 0xE1);
			GC0311_write_cmos_sensor(0xcA, 0xEA);
			GC0311_write_cmos_sensor(0xcB, 0xF1);
			GC0311_write_cmos_sensor(0xcC, 0xF5);
			GC0311_write_cmos_sensor(0xcD, 0xFB);
			GC0311_write_cmos_sensor(0xcE, 0xFE);
			GC0311_write_cmos_sensor(0xcF, 0xFF);
			break;
		case GC0311_RGB_Gamma_night:									//Gamma for night mode
			GC0311_write_cmos_sensor(0xBF, 0x0B);
			GC0311_write_cmos_sensor(0xc0, 0x16);
			GC0311_write_cmos_sensor(0xc1, 0x29);
			GC0311_write_cmos_sensor(0xc2, 0x3C);
			GC0311_write_cmos_sensor(0xc3, 0x4F);
			GC0311_write_cmos_sensor(0xc4, 0x5F);
			GC0311_write_cmos_sensor(0xc5, 0x6F);
			GC0311_write_cmos_sensor(0xc6, 0x8A);
			GC0311_write_cmos_sensor(0xc7, 0x9F);
			GC0311_write_cmos_sensor(0xc8, 0xB4);
			GC0311_write_cmos_sensor(0xc9, 0xC6);
			GC0311_write_cmos_sensor(0xcA, 0xD3);
			GC0311_write_cmos_sensor(0xcB, 0xDD);
			GC0311_write_cmos_sensor(0xcC, 0xE5);
			GC0311_write_cmos_sensor(0xcD, 0xF1);
			GC0311_write_cmos_sensor(0xcE, 0xFA);
			GC0311_write_cmos_sensor(0xcF, 0xFF);
			break;
		default:
			//GC0311_RGB_Gamma_m1
			GC0311_write_cmos_sensor(0xfe, 0x00);
			GC0311_write_cmos_sensor(0xbf , 0x0b);
			GC0311_write_cmos_sensor(0xc0 , 0x17);
			GC0311_write_cmos_sensor(0xc1 , 0x2a);
			GC0311_write_cmos_sensor(0xc2 , 0x41);
			GC0311_write_cmos_sensor(0xc3 , 0x54);
			GC0311_write_cmos_sensor(0xc4 , 0x66);
			GC0311_write_cmos_sensor(0xc5 , 0x74);
			GC0311_write_cmos_sensor(0xc6 , 0x8c);
			GC0311_write_cmos_sensor(0xc7 , 0xa3);
			GC0311_write_cmos_sensor(0xc8 , 0xb5);
			GC0311_write_cmos_sensor(0xc9 , 0xc4);
			GC0311_write_cmos_sensor(0xca , 0xd0);
			GC0311_write_cmos_sensor(0xcb , 0xdb);
			GC0311_write_cmos_sensor(0xcc , 0xe5);
			GC0311_write_cmos_sensor(0xcd , 0xf0);
			GC0311_write_cmos_sensor(0xce , 0xf7);
			GC0311_write_cmos_sensor(0xcf , 0xff);
			break;
	}
}


/*************************************************************************
 * FUNCTION
 *	GC0311_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of GC0311 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from GC0311
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0311_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* GC0311_config_window */


/*************************************************************************
 * FUNCTION
 *	GC0311_SetGain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *   iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0311_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
 * FUNCTION
 *	GC0311_NightMode
 *
 * DESCRIPTION
 *	This function night mode of GC0311.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0311NightMode(kal_bool bEnable)
{
	SENSORDB("Enter GC0311NightMode!, bEnable = %d, GC0311_MPEG4_encode_mode = %d\n", bEnable, GC0311_MPEG4_encode_mode);

	spin_lock(&GC0311_drv_lock);
	GC0311_night_mode_enable = bEnable;
	spin_unlock(&GC0311_drv_lock);	
	
	if (GC0311_night_mode_enable)	// night mode
	{	
		GC0311_write_cmos_sensor(0xfe, 0x00);
		if(MAXFramerate == 15)	// video
		{
			GC0311_write_cmos_sensor(0xf7, 0x33);		
			GC0311_write_cmos_sensor(0xfa, 0x32);
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x3c, 0x00);
		}	
		else
		{	
			GC0311_write_cmos_sensor(0xf7, 0x1b);				
			GC0311_write_cmos_sensor(0xfa, 0x11);
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x3c, 0x30);
//			Sleep(50);	
		}	
		GC0311_write_cmos_sensor(0xfe, 0x00);	
	}
	else 
	{
		GC0311_write_cmos_sensor(0xfe, 0x00);
		if(MINFramerate == 15)  // video
		{
			GC0311_write_cmos_sensor(0xf7, 0x1b);
			GC0311_write_cmos_sensor(0xfa, 0x11);			
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x3c, 0x00);
		}	
		else
		{
			GC0311_write_cmos_sensor(0xf7, 0x1b);
			GC0311_write_cmos_sensor(0xfa, 0x11);			
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x3c, 0x20);
		}
		GC0311_write_cmos_sensor(0xfe, 0x00);	
	}
	
} /* GC0311_NightMode */

/*************************************************************************
* FUNCTION
*	GC0311_Sensor_Init
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
void GC0311_Sensor_Init(void)
{
    GC0311_write_cmos_sensor(0xfe,0xf0);
    GC0311_write_cmos_sensor(0xfe,0xf0);
    GC0311_write_cmos_sensor(0xfe,0x00);
    GC0311_write_cmos_sensor(0xfc,0x0e);
    GC0311_write_cmos_sensor(0xfc,0x0e);
    GC0311_write_cmos_sensor(0xf2,0x80);
    GC0311_write_cmos_sensor(0xf3,0x00);
    GC0311_write_cmos_sensor(0xf7,0x1b);
    GC0311_write_cmos_sensor(0xf8,0x04);  // from 03 to 04
    GC0311_write_cmos_sensor(0xf9,0x8e);
    GC0311_write_cmos_sensor(0xfa,0x11);
     /////////////////////////////////////////////////      
	///////////////////   MIPI   ////////////////////      
	/////////////////////////////////////////////////      
	GC0311_write_cmos_sensor(0xfe,0x03);
	GC0311_write_cmos_sensor(0x40,0x08);
	GC0311_write_cmos_sensor(0x42,0x00);
	GC0311_write_cmos_sensor(0x43,0x00);
	GC0311_write_cmos_sensor(0x01,0x03);
	GC0311_write_cmos_sensor(0x10,0x84);
                                        
	GC0311_write_cmos_sensor(0x01,0x03);             
	GC0311_write_cmos_sensor(0x02,0x00);             
	GC0311_write_cmos_sensor(0x03,0x94);             
	GC0311_write_cmos_sensor(0x04,0x01);            
	GC0311_write_cmos_sensor(0x05,0x40);  // 40      20     
	GC0311_write_cmos_sensor(0x06,0x80);             
	GC0311_write_cmos_sensor(0x11,0x1e);             
	GC0311_write_cmos_sensor(0x12,0x00);      
	GC0311_write_cmos_sensor(0x13,0x05);             
	GC0311_write_cmos_sensor(0x15,0x10);                                                                    
	GC0311_write_cmos_sensor(0x21,0x10);             
	GC0311_write_cmos_sensor(0x22,0x01);             
	GC0311_write_cmos_sensor(0x23,0x10);                                             
	GC0311_write_cmos_sensor(0x24,0x02);                                             
	GC0311_write_cmos_sensor(0x25,0x10);                                             
	GC0311_write_cmos_sensor(0x26,0x03);                                             
	GC0311_write_cmos_sensor(0x29,0x02); //02                                            
	GC0311_write_cmos_sensor(0x2a,0x0a);   //0a                                          
	GC0311_write_cmos_sensor(0x2b,0x04);                                             
	GC0311_write_cmos_sensor(0xfe,0x00);
        /////////////////////////////////////////////////
        /////////////////   CISCTL reg  /////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x00,0x2f);
    GC0311_write_cmos_sensor(0x01,0x0f);
    GC0311_write_cmos_sensor(0x02,0x04);
    GC0311_write_cmos_sensor(0x03,0x04);
    GC0311_write_cmos_sensor(0x04,0xd0);
    GC0311_write_cmos_sensor(0x09,0x00);
    GC0311_write_cmos_sensor(0x0a,0x00);
    GC0311_write_cmos_sensor(0x0b,0x00);
    GC0311_write_cmos_sensor(0x0c,0x06);
    GC0311_write_cmos_sensor(0x0d,0x01);
    GC0311_write_cmos_sensor(0x0e,0xe8);
    GC0311_write_cmos_sensor(0x0f,0x02);
    GC0311_write_cmos_sensor(0x10,0x88);
    GC0311_write_cmos_sensor(0x16,0x00);
    GC0311_write_cmos_sensor(0x17,0x14);
    GC0311_write_cmos_sensor(0x18,0x1a);
    GC0311_write_cmos_sensor(0x19,0x14);
    GC0311_write_cmos_sensor(0x1b,0x48);
    GC0311_write_cmos_sensor(0x1e,0x6b);
    GC0311_write_cmos_sensor(0x1f,0x28);
    GC0311_write_cmos_sensor(0x20,0x8b);  // from 89 to 8b
    GC0311_write_cmos_sensor(0x21,0x49);
    GC0311_write_cmos_sensor(0x22,0xb0);
    GC0311_write_cmos_sensor(0x23,0x04);
    GC0311_write_cmos_sensor(0x24,0x16);
    GC0311_write_cmos_sensor(0x34,0x20);
        
        /////////////////////////////////////////////////
        ////////////////////   BLK   ////////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x26,0x23); 
    GC0311_write_cmos_sensor(0x28,0xff); 
    GC0311_write_cmos_sensor(0x29,0x00); 
    GC0311_write_cmos_sensor(0x33,0x10); 
    GC0311_write_cmos_sensor(0x37,0x20); 
	GC0311_write_cmos_sensor(0x38,0x10); 
    GC0311_write_cmos_sensor(0x47,0x80); 
    GC0311_write_cmos_sensor(0x4e,0x66); 
    GC0311_write_cmos_sensor(0xa8,0x02); 
    GC0311_write_cmos_sensor(0xa9,0x80);
        
        /////////////////////////////////////////////////
        //////////////////   ISP reg  ///////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x40,0xff); 
    GC0311_write_cmos_sensor(0x41,0x21); 
    GC0311_write_cmos_sensor(0x42,0xcf); 
    GC0311_write_cmos_sensor(0x44,0x01); // 02 yuv  00
    GC0311_write_cmos_sensor(0x45,0xa0); // from a8 - a4 a4-a0
    GC0311_write_cmos_sensor(0x46,0x03); 
    GC0311_write_cmos_sensor(0x4a,0x11);
    GC0311_write_cmos_sensor(0x4b,0x01);
    GC0311_write_cmos_sensor(0x4c,0x20); 
    GC0311_write_cmos_sensor(0x4d,0x05); 
    GC0311_write_cmos_sensor(0x4f,0x01);
    GC0311_write_cmos_sensor(0x50,0x01); 
    GC0311_write_cmos_sensor(0x55,0x01); 
    GC0311_write_cmos_sensor(0x56,0xe0);
    GC0311_write_cmos_sensor(0x57,0x02); 
    GC0311_write_cmos_sensor(0x58,0x80);
        
        /////////////////////////////////////////////////  
        ///////////////////   GAIN   ////////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x70,0x70); 
    GC0311_write_cmos_sensor(0x5a,0x84); 
    GC0311_write_cmos_sensor(0x5b,0xc9); 
    GC0311_write_cmos_sensor(0x5c,0xed); 
    GC0311_write_cmos_sensor(0x77,0x74); 
    GC0311_write_cmos_sensor(0x78,0x40); 
    GC0311_write_cmos_sensor(0x79,0x5f); 
        
        ///////////////////////////////////////////////// 
        ///////////////////   DNDD  /////////////////////
        ///////////////////////////////////////////////// 
    GC0311_write_cmos_sensor(0x82,0x0f); 
    GC0311_write_cmos_sensor(0x83,0x0b);
        
        
        ///////////////////////////////////////////////// 
        //////////////////   EEINTP  ////////////////////
        ///////////////////////////////////////////////// 
    GC0311_write_cmos_sensor(0x8f,0xaa); 
    GC0311_write_cmos_sensor(0x90,0x8c); 
    GC0311_write_cmos_sensor(0x91,0x90); 
    GC0311_write_cmos_sensor(0x92,0x03); 
    GC0311_write_cmos_sensor(0x93,0x03); 
    GC0311_write_cmos_sensor(0x94,0x05);
    GC0311_write_cmos_sensor(0x95,0x65); 
    GC0311_write_cmos_sensor(0x96,0xf0); 
        
        ///////////////////////////////////////////////// 
        /////////////////////  ASDE  ////////////////////
        ///////////////////////////////////////////////// 
    GC0311_write_cmos_sensor(0xfe,0x00);
    GC0311_write_cmos_sensor(0x9a,0x20);
    GC0311_write_cmos_sensor(0x9b,0x80);
    GC0311_write_cmos_sensor(0x9c,0x40);
    GC0311_write_cmos_sensor(0x9d,0x80);
    GC0311_write_cmos_sensor(0xa1,0x30);
    GC0311_write_cmos_sensor(0xa2,0x32);
    GC0311_write_cmos_sensor(0xa4,0x30);
    GC0311_write_cmos_sensor(0xa5,0x30);
    GC0311_write_cmos_sensor(0xaa,0x50);
    GC0311_write_cmos_sensor(0xac,0x22);
        
        /////////////////////////////////////////////////
        ///////////////////   GAMMA   ///////////////////
        /////////////////////////////////////////////////
        /*
    GC0311_write_cmos_sensor(0xbf,0x08); 
    GC0311_write_cmos_sensor(0xc0,0x16); 
    GC0311_write_cmos_sensor(0xc1,0x28); 
    GC0311_write_cmos_sensor(0xc2,0x41); 
    GC0311_write_cmos_sensor(0xc3,0x5a); 
    GC0311_write_cmos_sensor(0xc4,0x6c); 
    GC0311_write_cmos_sensor(0xc5,0x7a); 
    GC0311_write_cmos_sensor(0xc6,0x96); 
    GC0311_write_cmos_sensor(0xc7,0xac); 
    GC0311_write_cmos_sensor(0xc8,0xbc); 
    GC0311_write_cmos_sensor(0xc9,0xc9); 
    GC0311_write_cmos_sensor(0xca,0xd3); 
    GC0311_write_cmos_sensor(0xcb,0xdd); 
    GC0311_write_cmos_sensor(0xcc,0xe5); 
    GC0311_write_cmos_sensor(0xcd,0xf1); 
    GC0311_write_cmos_sensor(0xce,0xfa); 
    GC0311_write_cmos_sensor(0xcf,0xff);  */
    	GC0311_write_cmos_sensor(0xfe , 0x00);  //guodengxiang_melody
			GC0311_write_cmos_sensor(0xbf , 0x0d);
			GC0311_write_cmos_sensor(0xc0 , 0x12);
			GC0311_write_cmos_sensor(0xc1 , 0x29);
			GC0311_write_cmos_sensor(0xc2 , 0x42);
			GC0311_write_cmos_sensor(0xc3 , 0x59);
			GC0311_write_cmos_sensor(0xc4 , 0x6e);
			GC0311_write_cmos_sensor(0xc5 , 0x81);
			GC0311_write_cmos_sensor(0xc6 , 0x9f);
			GC0311_write_cmos_sensor(0xc7 , 0xb5);
			GC0311_write_cmos_sensor(0xc8 , 0xc8);
			GC0311_write_cmos_sensor(0xc9 , 0xd7);
			GC0311_write_cmos_sensor(0xca , 0xe1);
			GC0311_write_cmos_sensor(0xcb , 0xe9);
			GC0311_write_cmos_sensor(0xcc , 0xef);
			GC0311_write_cmos_sensor(0xcd , 0xf4);
			GC0311_write_cmos_sensor(0xce , 0xfa);
			GC0311_write_cmos_sensor(0xcf , 0xff);
                                          
        ///////////////////////////////////////////////////
        ///////////////////   YCP  ////////////////////////
        ///////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0xd0,0x40);   
    GC0311_write_cmos_sensor(0xd1,0x34);  //34
    GC0311_write_cmos_sensor(0xd2,0x34);  //34 melody
    GC0311_write_cmos_sensor(0xd3,0x40);   
    GC0311_write_cmos_sensor(0xd6,0xf2);   
    GC0311_write_cmos_sensor(0xd7,0x1b);   
    GC0311_write_cmos_sensor(0xd8,0x18);   
    GC0311_write_cmos_sensor(0xdd,0x03);   
        ///////////////////////////////////////////////////
        ////////////////////   AEC   //////////////////////
        ///////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0xfe,0x01);  
    GC0311_write_cmos_sensor(0x05,0x30);   
    GC0311_write_cmos_sensor(0x06,0x75);   
    GC0311_write_cmos_sensor(0x07,0x40);   
    GC0311_write_cmos_sensor(0x08,0xb0);   
    GC0311_write_cmos_sensor(0x0a,0xc5); 
    GC0311_write_cmos_sensor(0x0b,0x11);
    GC0311_write_cmos_sensor(0x0c,0x00); 
    GC0311_write_cmos_sensor(0x12,0x52);
    GC0311_write_cmos_sensor(0x13,0x38); 
    GC0311_write_cmos_sensor(0x18,0x95);
    GC0311_write_cmos_sensor(0x19,0x96);
    GC0311_write_cmos_sensor(0x1f,0x20);
    GC0311_write_cmos_sensor(0x20,0xc0); 
    GC0311_write_cmos_sensor(0x3e,0x40); 
    GC0311_write_cmos_sensor(0x3f,0x57); 
    GC0311_write_cmos_sensor(0x40,0x7d); 
    GC0311_write_cmos_sensor(0x03,0x60); 
    GC0311_write_cmos_sensor(0x44,0x02); 
    /////////////////////////////////////////////////
    ////////////////////   AWB   ////////////////////
    /////////////////////////////////////////////////
    /*
    GC0311_write_cmos_sensor(0x1c,0x91); 
    GC0311_write_cmos_sensor(0x21,0x15); 
    GC0311_write_cmos_sensor(0x50,0x80);
    GC0311_write_cmos_sensor(0x56,0x04);
    GC0311_write_cmos_sensor(0x58,0x08);    
    GC0311_write_cmos_sensor(0x59,0x08); 
    GC0311_write_cmos_sensor(0x5b,0x82);  // 02 to 82 to 02
    GC0311_write_cmos_sensor(0x61,0x8d); 
    GC0311_write_cmos_sensor(0x62,0xa7); 
    GC0311_write_cmos_sensor(0x63,0x00);   // d0 to  00
    GC0311_write_cmos_sensor(0x65,0x06);
    GC0311_write_cmos_sensor(0x66,0x06);   // 06 to 03
    GC0311_write_cmos_sensor(0x67,0x84); 
    GC0311_write_cmos_sensor(0x69,0x08);   // 08 to 20
    GC0311_write_cmos_sensor(0x6a,0x25); 
    GC0311_write_cmos_sensor(0x6b,0x01); 
    GC0311_write_cmos_sensor(0x6c,0x00);   // 00 to 0c
    GC0311_write_cmos_sensor(0x6d,0x02); 
    GC0311_write_cmos_sensor(0x6e,0x00);  // f0 to 00
    GC0311_write_cmos_sensor(0x6f,0x80); 
    GC0311_write_cmos_sensor(0x76,0x80); 
    GC0311_write_cmos_sensor(0x78,0xaf); 
    GC0311_write_cmos_sensor(0x79,0x75);
    GC0311_write_cmos_sensor(0x7a,0x40);
    GC0311_write_cmos_sensor(0x7b,0x50);
    GC0311_write_cmos_sensor(0x7c,0x08); //0c to 08 8.11
    GC0311_write_cmos_sensor(0xa4,0xb9); 
    GC0311_write_cmos_sensor(0xa5,0xa0);
    GC0311_write_cmos_sensor(0x90,0xc9); 
    GC0311_write_cmos_sensor(0x91,0xbe);
    GC0311_write_cmos_sensor(0xa6,0xb8); 
    GC0311_write_cmos_sensor(0xa7,0x95); 
    GC0311_write_cmos_sensor(0x92,0xe6); 
    GC0311_write_cmos_sensor(0x93,0xca); 
    GC0311_write_cmos_sensor(0xa9,0xb6); 
    GC0311_write_cmos_sensor(0xaa,0x89); 
    GC0311_write_cmos_sensor(0x95,0x23); 
    GC0311_write_cmos_sensor(0x96,0xe7); 
    GC0311_write_cmos_sensor(0xab,0x9d); 
    GC0311_write_cmos_sensor(0xac,0x80);
    GC0311_write_cmos_sensor(0x97,0x43); 
    GC0311_write_cmos_sensor(0x98,0x24); 
    GC0311_write_cmos_sensor(0xae,0xd0);   // b7 to d0
    GC0311_write_cmos_sensor(0xaf,0x9e); 
    GC0311_write_cmos_sensor(0x9a,0x43);
    GC0311_write_cmos_sensor(0x9b,0x24); 
        
    GC0311_write_cmos_sensor(0xb0,0xc0);  // c8 to c0
    GC0311_write_cmos_sensor(0xb1,0xa8);   // 97 to a8
    GC0311_write_cmos_sensor(0x9c,0xc4); 
    GC0311_write_cmos_sensor(0x9d,0x44); 
    GC0311_write_cmos_sensor(0xb3,0xb7); 
    GC0311_write_cmos_sensor(0xb4,0x7f);
    GC0311_write_cmos_sensor(0x9f,0xc7);
    GC0311_write_cmos_sensor(0xa0,0xc8); 
    GC0311_write_cmos_sensor(0xb5,0x00); 
    GC0311_write_cmos_sensor(0xb6,0x00);
    GC0311_write_cmos_sensor(0xa1,0x00);
    GC0311_write_cmos_sensor(0xa2,0x00);
    GC0311_write_cmos_sensor(0x86,0x60);
    GC0311_write_cmos_sensor(0x87,0x08);
    GC0311_write_cmos_sensor(0x88,0x00);
    GC0311_write_cmos_sensor(0x89,0x00);
    GC0311_write_cmos_sensor(0x8b,0xde);
    GC0311_write_cmos_sensor(0x8c,0x80);
    GC0311_write_cmos_sensor(0x8d,0x00);
    GC0311_write_cmos_sensor(0x8e,0x00);
    GC0311_write_cmos_sensor(0x94,0x55);
    GC0311_write_cmos_sensor(0x99,0xa6);
    GC0311_write_cmos_sensor(0x9e,0xaa);
    GC0311_write_cmos_sensor(0xa3,0x0a);
    GC0311_write_cmos_sensor(0x8a,0x0a);
    GC0311_write_cmos_sensor(0xa8,0x55);
    GC0311_write_cmos_sensor(0xad,0x55);
    GC0311_write_cmos_sensor(0xb2,0x55);
    GC0311_write_cmos_sensor(0xb7,0x05);
    GC0311_write_cmos_sensor(0x8f,0x05);
    GC0311_write_cmos_sensor(0xb8,0xcc);
    GC0311_write_cmos_sensor(0xb9,0x9a);  
    */
      GC0311_write_cmos_sensor(0xfe , 0x01);		//awb guodengxiang_melody
			GC0311_write_cmos_sensor(0x1c , 0x91);  
			GC0311_write_cmos_sensor(0x21 , 0x15);  
			GC0311_write_cmos_sensor(0x50 , 0x80);  
    GC0311_write_cmos_sensor(0x56,0x04);
		GC0311_write_cmos_sensor(0x58	,	0x08);    
		GC0311_write_cmos_sensor(0x59 , 0x08);  
		GC0311_write_cmos_sensor(0x5b	,	0x02);  // 
		GC0311_write_cmos_sensor(0x61 , 0x8d);  
		GC0311_write_cmos_sensor(0x62 , 0xa7);  
		GC0311_write_cmos_sensor(0x63	,	0x00);   // d0 to  00
		GC0311_write_cmos_sensor(0x65 , 0x06); 
		GC0311_write_cmos_sensor(0x66 , 0x06);  
		GC0311_write_cmos_sensor(0x67 , 0xa2);   //84 
		GC0311_write_cmos_sensor(0x69 , 0x08); 
		GC0311_write_cmos_sensor(0x6a , 0x50); //50
		GC0311_write_cmos_sensor(0x6b , 0x00);  //01  
		GC0311_write_cmos_sensor(0x6c , 0x00);  
		GC0311_write_cmos_sensor(0x6d , 0x02);  
		GC0311_write_cmos_sensor(0x6e , 0xf0);  //
		GC0311_write_cmos_sensor(0x6f , 0x00);   //80
		GC0311_write_cmos_sensor(0x76 , 0x80); 
		
		GC0311_write_cmos_sensor(0x78	,	0xaf);   
  
			/*GC0311_write_cmos_sensor(0xfe , 0x01); //xuan kuang////
			GC0311_write_cmos_sensor(0x90 , 0x00); 
			GC0311_write_cmos_sensor(0x91 , 0x00); 
			GC0311_write_cmos_sensor(0x92 , 0xff); 
			GC0311_write_cmos_sensor(0x93 , 0xeb); 
			GC0311_write_cmos_sensor(0x95 , 0x24); 
			GC0311_write_cmos_sensor(0x96 , 0x00); 
			GC0311_write_cmos_sensor(0x97 , 0x4d); 
			GC0311_write_cmos_sensor(0x98 , 0x23); 
			GC0311_write_cmos_sensor(0x9a , 0x4c); 
			GC0311_write_cmos_sensor(0x9b , 0x23); 
			GC0311_write_cmos_sensor(0x9c , 0x83); 
			GC0311_write_cmos_sensor(0x9d , 0x4c); 
			GC0311_write_cmos_sensor(0x9f , 0xa3); 
			GC0311_write_cmos_sensor(0xa0 , 0x84); 
			GC0311_write_cmos_sensor(0xa1 , 0x00); 
			GC0311_write_cmos_sensor(0xa2 , 0x00); 
			GC0311_write_cmos_sensor(0x86 , 0x00); 
			GC0311_write_cmos_sensor(0x87 , 0x00); 
			GC0311_write_cmos_sensor(0x88 , 0x00); 
			GC0311_write_cmos_sensor(0x89 , 0x00); 
			GC0311_write_cmos_sensor(0xa4 , 0x00); 
			GC0311_write_cmos_sensor(0xa5 , 0x00); 
			GC0311_write_cmos_sensor(0xa6 , 0xc1); 
			GC0311_write_cmos_sensor(0xa7 , 0x99); 
			GC0311_write_cmos_sensor(0xa9 , 0xc9); 
			GC0311_write_cmos_sensor(0xaa , 0x8e); 
			GC0311_write_cmos_sensor(0xab , 0xa8); 
			GC0311_write_cmos_sensor(0xac , 0x81); 
			GC0311_write_cmos_sensor(0xae , 0xc9); 
			GC0311_write_cmos_sensor(0xaf , 0xa9); 
			GC0311_write_cmos_sensor(0xb0 , 0xd3); 
			GC0311_write_cmos_sensor(0xb1 , 0xa2); 
			GC0311_write_cmos_sensor(0xb3 , 0xd4); 
			GC0311_write_cmos_sensor(0xb4 , 0xa0); 
			GC0311_write_cmos_sensor(0xb5 , 0x00); 
			GC0311_write_cmos_sensor(0xb6 , 0x00); 
			GC0311_write_cmos_sensor(0x8b , 0x00); 
			GC0311_write_cmos_sensor(0x8c , 0x00); 
			GC0311_write_cmos_sensor(0x8d , 0x00); 
			GC0311_write_cmos_sensor(0x8e , 0x00); 
			GC0311_write_cmos_sensor(0x94 , 0x50); 
			GC0311_write_cmos_sensor(0x99 , 0xaa); 
			GC0311_write_cmos_sensor(0x9e , 0xaa); 
			GC0311_write_cmos_sensor(0xa3 , 0x0a); 
			GC0311_write_cmos_sensor(0x8a , 0x00); 
			GC0311_write_cmos_sensor(0xa8 , 0x50); 
			GC0311_write_cmos_sensor(0xad , 0x55); 
			GC0311_write_cmos_sensor(0xb2 , 0x55); 
			GC0311_write_cmos_sensor(0xb7 , 0x05); 
			GC0311_write_cmos_sensor(0x8f , 0x00); 
			GC0311_write_cmos_sensor(0xb8 , 0xdd); 
			GC0311_write_cmos_sensor(0xb9 , 0x81);*/ 

      GC0311_write_cmos_sensor(0xfe , 0x01); //change  summer 0427////
			GC0311_write_cmos_sensor(0x90 , 0xc9); 
			GC0311_write_cmos_sensor(0x91 , 0xbe); 
			GC0311_write_cmos_sensor(0x92 , 0xe6); 
			GC0311_write_cmos_sensor(0x93 , 0xca); 
			GC0311_write_cmos_sensor(0x95 , 0x19); 
			GC0311_write_cmos_sensor(0x96 , 0xe7); 
			GC0311_write_cmos_sensor(0x97 , 0x47); 
			GC0311_write_cmos_sensor(0x98 , 0x1b); 
			GC0311_write_cmos_sensor(0x9a , 0x48); 
			GC0311_write_cmos_sensor(0x9b , 0x1b); 
			GC0311_write_cmos_sensor(0x9c , 0xc7); 
			GC0311_write_cmos_sensor(0x9d , 0x48); 
			GC0311_write_cmos_sensor(0x9f , 0xc7); 
			GC0311_write_cmos_sensor(0xa0 , 0xc8); 
			GC0311_write_cmos_sensor(0xa1 , 0x00); 
			GC0311_write_cmos_sensor(0xa2 , 0x00); 
			GC0311_write_cmos_sensor(0x86 , 0x00); 
			GC0311_write_cmos_sensor(0x87 , 0x00); 
			GC0311_write_cmos_sensor(0x88 , 0x00); 
			GC0311_write_cmos_sensor(0x89 , 0x00); 
			GC0311_write_cmos_sensor(0xa4 , 0xb9); 
			GC0311_write_cmos_sensor(0xa5 , 0xa0); 
			GC0311_write_cmos_sensor(0xa6 , 0xb8); 
			GC0311_write_cmos_sensor(0xa7 , 0x95); 
			GC0311_write_cmos_sensor(0xa9 , 0xb6); 
			GC0311_write_cmos_sensor(0xaa , 0x89); 
			GC0311_write_cmos_sensor(0xab , 0x9e); 
			GC0311_write_cmos_sensor(0xac , 0x7f); 
			GC0311_write_cmos_sensor(0xae , 0xc1); 
			GC0311_write_cmos_sensor(0xaf , 0x9e); 
			GC0311_write_cmos_sensor(0xb0 , 0xc8); 
			GC0311_write_cmos_sensor(0xb1 , 0x97); 
			GC0311_write_cmos_sensor(0xb3 , 0xb7); 
			GC0311_write_cmos_sensor(0xb4 , 0x7f); 
			GC0311_write_cmos_sensor(0xb5 , 0x00); 
			GC0311_write_cmos_sensor(0xb6 , 0x00); 
			GC0311_write_cmos_sensor(0x8b , 0x00); 
			GC0311_write_cmos_sensor(0x8c , 0x00); 
			GC0311_write_cmos_sensor(0x8d , 0x00); 
			GC0311_write_cmos_sensor(0x8e , 0x00); 
			GC0311_write_cmos_sensor(0x94 , 0x55); 
			GC0311_write_cmos_sensor(0x99 , 0xa6); 
			GC0311_write_cmos_sensor(0x9e , 0xaa); 
			GC0311_write_cmos_sensor(0xa3 , 0x0a); 
			GC0311_write_cmos_sensor(0x8a , 0x00); 
			GC0311_write_cmos_sensor(0xa8 , 0x55); 
			GC0311_write_cmos_sensor(0xad , 0x55); 
			GC0311_write_cmos_sensor(0xb2 , 0x55); 
			GC0311_write_cmos_sensor(0xb7 , 0x05); 
			GC0311_write_cmos_sensor(0x8f , 0x00); 
			GC0311_write_cmos_sensor(0xb8 , 0xcc); 
			GC0311_write_cmos_sensor(0xb9 , 0x9a);  
        /////////////////////////////////////
        ////////////////////  CC ////////////
        /////////////////////////////////////
 /*
    GC0311_write_cmos_sensor(0xfe,0x01);
    GC0311_write_cmos_sensor(0xd0,0x38);
    GC0311_write_cmos_sensor(0xd1,0xfd);
    GC0311_write_cmos_sensor(0xd2,0x06);
    GC0311_write_cmos_sensor(0xd3,0xf0);
    GC0311_write_cmos_sensor(0xd4,0x40);
    GC0311_write_cmos_sensor(0xd5,0x08);
    GC0311_write_cmos_sensor(0xd6,0x30);
    GC0311_write_cmos_sensor(0xd7,0x00);
    GC0311_write_cmos_sensor(0xd8,0x0a);
    GC0311_write_cmos_sensor(0xd9,0x16);
    GC0311_write_cmos_sensor(0xda,0x39);
    GC0311_write_cmos_sensor(0xdb,0xf8);
    */
    	GC0311_write_cmos_sensor(0xfe , 0x01);
	GC0311_write_cmos_sensor(0xd0 , 0x38);  //31
	GC0311_write_cmos_sensor(0xd1 , 0x02);
	GC0311_write_cmos_sensor(0xd2 , 0x0a); //08
	GC0311_write_cmos_sensor(0xd3 , 0x08);  //00
	GC0311_write_cmos_sensor(0xd4 , 0x46);
	GC0311_write_cmos_sensor(0xd5 , 0xfe);  //fd
	GC0311_write_cmos_sensor(0xd6 , 0x3f);
	GC0311_write_cmos_sensor(0xd7 , 0xf8);//00
	GC0311_write_cmos_sensor(0xd8 , 0x05);
	GC0311_write_cmos_sensor(0xd9 , 0x08);
	GC0311_write_cmos_sensor(0xda , 0x38);
	GC0311_write_cmos_sensor(0xdb , 0xe9);

        /////////////////////////////////////////////////
        ////////////////////   LSC   ////////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0xfe,0x01); 
    GC0311_write_cmos_sensor(0xc1,0x3c); 
    GC0311_write_cmos_sensor(0xc2,0x50); 
    GC0311_write_cmos_sensor(0xc3,0x00); 
    GC0311_write_cmos_sensor(0xc4,0x40); 
    GC0311_write_cmos_sensor(0xc5,0x2d); 
    GC0311_write_cmos_sensor(0xc6,0x34); 
    GC0311_write_cmos_sensor(0xc7,0x20); 
    GC0311_write_cmos_sensor(0xc8,0x00); 
    GC0311_write_cmos_sensor(0xc9,0x06); 
    GC0311_write_cmos_sensor(0xdc,0x20); 
    GC0311_write_cmos_sensor(0xdd,0x10); 
    GC0311_write_cmos_sensor(0xdf,0x00); 
    GC0311_write_cmos_sensor(0xde,0x00); 
        
        /////////////////////////////////////////////////
        ///////////////////  Histogram  /////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x01,0x10); 
    GC0311_write_cmos_sensor(0x0b,0x31); 
    GC0311_write_cmos_sensor(0x0e,0x50); 
    GC0311_write_cmos_sensor(0x0f,0x0f); 
    GC0311_write_cmos_sensor(0x10,0x6e); 
    GC0311_write_cmos_sensor(0x12,0xa0); 
    GC0311_write_cmos_sensor(0x15,0x60); 
    GC0311_write_cmos_sensor(0x16,0x60); 
    GC0311_write_cmos_sensor(0x17,0xe0); 
        
        /////////////////////////////////////////////////
        //////////////   Measure Window   ///////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0xcc,0x0c);  
    GC0311_write_cmos_sensor(0xcd,0x10); 
    GC0311_write_cmos_sensor(0xce,0xa0); 
    GC0311_write_cmos_sensor(0xcf,0xe6); 
        
        /////////////////////////////////////////////////
        /////////////////   dark sun   //////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0x45,0xf7);
    GC0311_write_cmos_sensor(0x46,0xff); 
    GC0311_write_cmos_sensor(0x47,0x15);
    GC0311_write_cmos_sensor(0x48,0x03); 
    GC0311_write_cmos_sensor(0x4f,0x60); 
        
        /////////////////////////////////////////////////
        ///////////////////  banding  ///////////////////
        /////////////////////////////////////////////////
    GC0311_write_cmos_sensor(0xfe,0x00);
    GC0311_write_cmos_sensor(0x05,0x01);
    GC0311_write_cmos_sensor(0x06,0x18); //HB
#if 1    
    GC0311_write_cmos_sensor(0x07,0x00);
    GC0311_write_cmos_sensor(0x08,0x10); //VB  from 10 to 50
#else
	GC0311_write_cmos_sensor(0x07,0x01);
	GC0311_write_cmos_sensor(0x08,0xe0); //VB
#endif
    GC0311_write_cmos_sensor(0xfe,0x01);
    GC0311_write_cmos_sensor(0x25,0x00); //step 
    GC0311_write_cmos_sensor(0x26,0x9a); 
    GC0311_write_cmos_sensor(0x27,0x01); //30fps
    GC0311_write_cmos_sensor(0x28,0xce);  
    GC0311_write_cmos_sensor(0x29,0x04); //12.5fps
	GC0311_write_cmos_sensor(0x2a,0x36); 
	GC0311_write_cmos_sensor(0x2b,0x06); //10fps
	GC0311_write_cmos_sensor(0x2c,0x04); 
	GC0311_write_cmos_sensor(0x2d,0x0c); //5fps
	GC0311_write_cmos_sensor(0x2e,0x08);
    GC0311_write_cmos_sensor(0x3c,0x20);
        
        /////////////////////////////////////////////////
        ///////////////////   MIPI   ////////////////////
        /////////////////////////////////////////////////
   GC0311_write_cmos_sensor(0xfe,0x03);
   GC0311_write_cmos_sensor(0x10,0x94);  
   GC0311_write_cmos_sensor(0xfe,0x00); 
}


UINT32 GC0311GetSensorID(UINT32 *sensorID)
{
    int  retry = 3; 
    // check if sensor ID correct
    do {
        *sensorID=((GC0311_read_cmos_sensor(0xf0)<< 8)|GC0311_read_cmos_sensor(0xf1));
		printk("%s:%d:Read Sensor ID sensorID = 0x%04x\n", __func__,__LINE__,*sensorID); 
        if (*sensorID == GC0310_SENSOR_ID)
        {
			                //add by hcz
				mt_set_gpio_dir(MAIN_CAMERA_ID_STROBE_PIN, GPIO_DIR_IN);
				mt_set_gpio_mode(MAIN_CAMERA_ID_STROBE_PIN, GPIO_MODE_00);	
				msleep(2);
				if(mt_get_gpio_in(MAIN_CAMERA_ID_STROBE_PIN)==0)  //bolixin module
				{		
					//printk("hcz bolixin ID\r\n");		     
					*sensorID = GC0311_SENSOR_ID;	
				
		            return ERROR_NONE;
				}
				else
				{
				 	*sensorID = 0xFFFFFFFF;
		        	return ERROR_SENSOR_CONNECT_FAIL;
				}
				//add end
        	}
            break; 
        SENSORDB("Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);

    if (*sensorID != GC0311_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}




/*************************************************************************
* FUNCTION
*	GC0311_Write_More_Registers
*
* DESCRIPTION
*	This function is served for FAE to modify the necessary Init Regs. Do not modify the regs
*     in init_GC0311() directly.
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
void GC0311_Write_More_Registers(void)
{

}


/*************************************************************************
 * FUNCTION
 *	GC0311Open
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
UINT32 GC0311Open(void)
{
	volatile signed char i;
	kal_uint16 sensor_id=0;

	SENSORDB("<Jet> Entry GC0311Open!!!\r\n");

	Sleep(10);

	//  Read sensor ID to adjust I2C is OK?
	for(i=0;i<3;i++)
	{
		sensor_id = ((GC0311_read_cmos_sensor(0xf0) << 8) | GC0311_read_cmos_sensor(0xf1));
		
		if(sensor_id == GC0310_SENSOR_ID)
		{
			//add by hcz
			  mt_set_gpio_dir(MAIN_CAMERA_ID_STROBE_PIN, GPIO_DIR_IN);
			  mt_set_gpio_mode(MAIN_CAMERA_ID_STROBE_PIN, GPIO_MODE_00);  
			  msleep(2);
			  if(mt_get_gpio_in(MAIN_CAMERA_ID_STROBE_PIN)==0)	//bolixin module
			  { 	
			  
					//printk("hcz bolixin ID\r\n");			
					sensor_id =GC0311_SENSOR_ID;
					break;
			  }
			  else
			  {			
				  return ERROR_SENSOR_CONNECT_FAIL;
			  }
				  //add end

		}
		else
		{
			SENSORDB("GC0311mipi Read Sensor ID Fail[open] = 0x%x\n", sensor_id); 
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}
	
	SENSORDB("GC0311mipi_ Sensor Read ID OK \r\n");
	GC0311_Sensor_Init();
	GC0311_Write_More_Registers();

	spin_lock(&GC0311_drv_lock);
	GC0311_night_mode_enable = KAL_FALSE;
	GC0311_MPEG4_encode_mode = KAL_FALSE;
	spin_unlock(&GC0311_drv_lock);	

	return ERROR_NONE;
} /* GC0311Open */



/*************************************************************************
 * FUNCTION
 *	GC0311Close
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
UINT32 GC0311Close(void)
{
    return ERROR_NONE;
} /* GC0311Close */


/*************************************************************************
 * FUNCTION
 * GC0311Preview
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
UINT32 GC0311Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

	SENSORDB("Enter GC0311Preview function!!!\r\n");
//	GC0311StreamOn();

    image_window->GrabStartX= IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    GC0311_Set_Mirrorflip(IMAGE_HV_MIRROR);
    

    // copy sensor_config_data
    memcpy(&GC0311SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
//    GC0311NightMode(GC0311_night_mode_enable);
    return ERROR_NONE;
} /* GC0311Preview */


/*************************************************************************
 * FUNCTION
 *	GC0311Capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0311Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
	spin_lock(&GC0311_drv_lock);
    GC0311_MODE_CAPTURE=KAL_TRUE;
	spin_unlock(&GC0311_drv_lock);


    image_window->GrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0311SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0311_Capture() */



UINT32 GC0311GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_PV_HEIGHT;
    
    pSensorResolution->SensorHighSpeedVideoWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorHighSpeedVideoHeight=IMAGE_SENSOR_PV_HEIGHT;
    
    pSensorResolution->SensorSlimVideoWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorSlimVideoHeight=IMAGE_SENSOR_PV_HEIGHT;    
    return ERROR_NONE;
} /* GC0311GetResolution() */


UINT32 GC0311GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_HEIGHT;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_VYUY;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;//MIPI setting
    pSensorInfo->CaptureDelayFrame = 2;
    pSensorInfo->PreviewDelayFrame = 2;
    pSensorInfo->VideoDelayFrame = 4;

    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;

    pSensorInfo->HighSpeedVideoDelayFrame = 4;
    pSensorInfo->SlimVideoDelayFrame = 4;
    pSensorInfo->SensorModeNum = 5;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    default:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
	//MIPI setting
	pSensorInfo->SensorMIPILaneNumber = SENSOR_MIPI_1_LANE; 	
	pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14;   
	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
	pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
	pSensorInfo->SensorHightSampling = 0;	// 0 is default 1x 
	pSensorInfo->SensorPacketECCOrder = 1;

        break;
    }
    GC0311PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &GC0311SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0311GetInfo() */


UINT32 GC0311Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	SENSORDB("Entry GC0311Control, ScenarioId = %d!!!\r\n", ScenarioId);
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:   
    	SENSORDB("GC0311 Camera Video Preview!\n");
		spin_lock(&GC0311_drv_lock);         
        GC0311_MPEG4_encode_mode = KAL_TRUE;
        spin_unlock(&GC0311_drv_lock); 
		GC0311Preview(pImageWindow, pSensorConfigData);        
        break;
	case MSDK_SCENARIO_ID_CAMERA_PREVIEW:       
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    default:
        spin_lock(&GC0311_drv_lock);  
        GC0311_MPEG4_encode_mode = KAL_FALSE;
        spin_unlock(&GC0311_drv_lock);
		GC0311Preview(pImageWindow, pSensorConfigData);        
        break;
    }
    
    return ERROR_NONE;
}	/* GC0311Control() */

BOOL GC0311_set_param_wb(UINT16 para)
{
	SENSORDB("GC0311_set_param_wb para = %d\n", para);
    spin_lock(&GC0311_drv_lock);
    GC0311_CurStatus_AWB = para;
    spin_unlock(&GC0311_drv_lock);

	GC0311_write_cmos_sensor(0xfe, 0x00); 	
	switch (para)
	{
		case AWB_MODE_OFF:
			GC0311_awb_enable(KAL_FALSE);
		    GC0311_write_cmos_sensor(0x77,0x74); 
		    GC0311_write_cmos_sensor(0x78,0x40); 
		    GC0311_write_cmos_sensor(0x79,0x5f); 			
		break;
		
		case AWB_MODE_AUTO:
		    GC0311_write_cmos_sensor(0x77,0x74); 
		    GC0311_write_cmos_sensor(0x78,0x40); 
		    GC0311_write_cmos_sensor(0x79,0x5f); 		
			GC0311_awb_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			GC0311_awb_enable(KAL_FALSE);
			GC0311_write_cmos_sensor(0x77, 0x40);
			GC0311_write_cmos_sensor(0x78, 0x54);
			GC0311_write_cmos_sensor(0x79, 0x70);
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			GC0311_awb_enable(KAL_FALSE);
			GC0311_write_cmos_sensor(0x77, 0x74); 
			GC0311_write_cmos_sensor(0x78, 0x52);
			GC0311_write_cmos_sensor(0x79, 0x40);			
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			GC0311_awb_enable(KAL_FALSE);
			GC0311_write_cmos_sensor(0x77, 0x48);
			GC0311_write_cmos_sensor(0x78, 0x40);
			GC0311_write_cmos_sensor(0x79, 0x5c);
		break;
		
		case AWB_MODE_TUNGSTEN: //home		
			GC0311_awb_enable(KAL_FALSE);
			GC0311_write_cmos_sensor(0x77, 0x8c); //WB_manual_gain 
			GC0311_write_cmos_sensor(0x78, 0x50);
			GC0311_write_cmos_sensor(0x79, 0x40);
		break;
		
		case AWB_MODE_FLUORESCENT:
			GC0311_awb_enable(KAL_FALSE);
			GC0311_write_cmos_sensor(0x77, 0x40);
			GC0311_write_cmos_sensor(0x78, 0x42);
			GC0311_write_cmos_sensor(0x79, 0x50);
		break;
		
		default:
		return FALSE;
	}
	GC0311_write_cmos_sensor(0xfe, 0x00); 

	return TRUE;
} /* GC0311_set_param_wb */


BOOL GC0311_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			GC0311_write_cmos_sensor(0x43 , 0x00);
		break;
		
		case MEFFECT_SEPIA:
			GC0311_write_cmos_sensor(0x43 , 0x02);
			GC0311_write_cmos_sensor(0xda , 0xd0);
			GC0311_write_cmos_sensor(0xdb , 0x28);
		break;
		
		case MEFFECT_NEGATIVE:
			GC0311_write_cmos_sensor(0x43 , 0x01);
		break;
		
		case MEFFECT_SEPIAGREEN:
			GC0311_write_cmos_sensor(0x43 , 0x02);
			GC0311_write_cmos_sensor(0xda , 0xc0);
			GC0311_write_cmos_sensor(0xdb , 0xc0);
		break;
		
		case MEFFECT_SEPIABLUE:
			GC0311_write_cmos_sensor(0x43 , 0x02);
			GC0311_write_cmos_sensor(0xda , 0x50);
			GC0311_write_cmos_sensor(0xdb , 0xe0);
		break;

		case MEFFECT_MONO:
			GC0311_write_cmos_sensor(0x43 , 0x02);
			GC0311_write_cmos_sensor(0xda , 0x00);
			GC0311_write_cmos_sensor(0xdb , 0x00);
		break;
		default:
			ret = FALSE;
	}

	return ret;

} /* GC0311_set_param_effect */


BOOL GC0311_set_param_banding(UINT16 para)
{

	SENSORDB("Enter GC0311_set_param_banding!, para = %d\r\n", para);	
	switch (para)
	{
		case AE_FLICKER_MODE_AUTO:
		case AE_FLICKER_MODE_OFF:
		case AE_FLICKER_MODE_50HZ:
			GC0311_write_cmos_sensor(0xfe, 0x00); 
			GC0311_write_cmos_sensor(0x05, 0x01); 	
			GC0311_write_cmos_sensor(0x06, 0x18); 
			GC0311_write_cmos_sensor(0x07, 0x00);
			GC0311_write_cmos_sensor(0x08, 0x10);
			
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x25,0x00); //step 
			GC0311_write_cmos_sensor(0x26,0x9a); 
		
			GC0311_write_cmos_sensor(0x27,0x01); //30fps
			GC0311_write_cmos_sensor(0x28,0xce);  
			GC0311_write_cmos_sensor(0x29,0x04); //15fps
			GC0311_write_cmos_sensor(0x2a,0x36); 
			GC0311_write_cmos_sensor(0x2b,0x06); //10fps
			GC0311_write_cmos_sensor(0x2c,0x04); 
			GC0311_write_cmos_sensor(0x2d,0x09); //8fps  
			GC0311_write_cmos_sensor(0x2e,0xa0);  // 
			//GC0311_write_cmos_sensor(0x3c,0x20);
			GC0311_write_cmos_sensor(0xfe, 0x00); 			
			break;

		case AE_FLICKER_MODE_60HZ:
			GC0311_write_cmos_sensor(0xfe, 0x00); 
			GC0311_write_cmos_sensor(0x05, 0x01); 	
			GC0311_write_cmos_sensor(0x06, 0x13); 
			GC0311_write_cmos_sensor(0x07, 0x00);
			GC0311_write_cmos_sensor(0x08, 0x10);
			
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x25, 0x00);   //anti-flicker step [11:8]
			GC0311_write_cmos_sensor(0x26, 0x81);	//anti-flicker step [7:0]
			
			GC0311_write_cmos_sensor(0x27, 0x01);	//exp level 0  30fps
			GC0311_write_cmos_sensor(0x28, 0x83); 
			GC0311_write_cmos_sensor(0x29, 0x04);	//exp level 1  15fps
			GC0311_write_cmos_sensor(0x2a, 0x08); 
			GC0311_write_cmos_sensor(0x2b, 0x06);	//exp level 2  10fps
			GC0311_write_cmos_sensor(0x2c, 0x0c); 
			GC0311_write_cmos_sensor(0x2d, 0x09);	//exp level 3 8fps
			GC0311_write_cmos_sensor(0x2e, 0x93);	
			GC0311_write_cmos_sensor(0xfe, 0x00);

		break;
		default:
		return FALSE;
	}
	GC0311_write_cmos_sensor(0xfe, 0x00); 

	return TRUE;
} /* GC0311_set_param_banding */

BOOL GC0311_set_param_exposure(UINT16 para)
{

	switch (para)
	{
	
		case AE_EV_COMP_n20:
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x13, 0x20);
			GC0311_write_cmos_sensor(0xfe, 0x00);
		break;

		case AE_EV_COMP_n10:
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x13, 0x28);  // 28 to 10
			GC0311_write_cmos_sensor(0xfe, 0x00);
		break;			
		
		case AE_EV_COMP_00:	
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x13, 0x38);//35
			GC0311_write_cmos_sensor(0xfe, 0x00);
		break;

		case AE_EV_COMP_15:    	
		case AE_EV_COMP_10:
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x13, 0x48);  // 48 to 60
			GC0311_write_cmos_sensor(0xfe, 0x00);
			Sleep(200);
		break;
		
		case AE_EV_COMP_20:
			GC0311_write_cmos_sensor(0xfe, 0x01);
			GC0311_write_cmos_sensor(0x13, 0x50);
			GC0311_write_cmos_sensor(0xfe, 0x00);
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0311_set_param_exposure */


UINT32 GC0311YUVSetVideoMode(UINT16 u2FrameRate)    // lanking add
{

	SENSORDB("Enter GC0311YUVSetVideoMode, u2FrameRate = %d\n", u2FrameRate);
//	spin_lock(&GC0311_drv_lock);  
//    GC0311_MPEG4_encode_mode = KAL_TRUE;
//	spin_unlock(&GC0311_drv_lock);

    
     if (u2FrameRate == 30)
   	{
   	
   	    /*********video frame ************/
		
   	}
    else if (u2FrameRate == 15)       
    	{
    	
   	    /*********video frame ************/
		
    	}
    else
   	{
   	
            SENSORDB("Wrong Frame Rate"); 
			
   	}
   	
	GC0311NightMode(GC0311_night_mode_enable);

    return TRUE;

}


UINT32 GC0311YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
    case FID_AWB_MODE:
        GC0311_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        GC0311_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        GC0311_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        GC0311_set_param_banding(iPara);
		break;
    case FID_SCENE_MODE:
	 	GC0311NightMode(iPara);
        break;
        
	case FID_ISP_CONTRAST:
		GC0311_set_contrast(iPara);
		break;
	case FID_ISP_BRIGHT:
		GC0311_set_brightness(iPara);
		break;
	case FID_ISP_SAT:
		GC0311_set_saturation(iPara);
		break; 
	case FID_AE_ISO:
		GC0311_set_iso(iPara);
		break;	
	case FID_AE_SCENE_MODE: 
		GC0311_set_AE_mode(iPara);
		break; 
		
    default:
        break;
    }
    return TRUE;
} /* GC0311YUVSensorSetting */


UINT32 GC0311FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 **ppFeatureData=(UINT32 **) pFeaturePara;
    UINT32 GC0311SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    unsigned long long *pFeatureData=(unsigned long long *) pFeaturePara;
    unsigned long long *pFeatureReturnPara=(unsigned long long *) pFeaturePara;
    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(VGA_PERIOD_PIXEL_NUMS)+GC0311_dummy_pixels;
        *pFeatureReturnPara16=(VGA_PERIOD_LINE_NUMS)+GC0311_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = GC0311_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        GC0311NightMode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        GC0311_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        GC0311_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = GC0311_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &GC0311SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
        *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
        break;
    case SENSOR_FEATURE_SET_CCT_REGISTER:
    case SENSOR_FEATURE_GET_CCT_REGISTER:
    case SENSOR_FEATURE_SET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
    case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
    case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
    case SENSOR_FEATURE_GET_GROUP_COUNT:
    case SENSOR_FEATURE_GET_GROUP_INFO:
    case SENSOR_FEATURE_GET_ITEM_INFO:
    case SENSOR_FEATURE_SET_ITEM_INFO:
    case SENSOR_FEATURE_GET_ENG_INFO:
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        GC0311YUVSensorSetting((FEATURE_ID)*pFeatureData, *(pFeatureData+1));
        
        break;
    case SENSOR_FEATURE_SET_VIDEO_MODE:    //  lanking
	 GC0311YUVSetVideoMode(*pFeatureData16);
	 break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	GC0311GetSensorID(pFeatureData32);
	break;
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
		*pFeatureReturnPara32=GC0311_TEST_PATTERN_CHECKSUM;
		*pFeatureParaLen=4;
		break;

	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
		 SENSORDB("[GC0311] F_SET_MAX_FRAME_RATE_BY_SCENARIO.\n");
	//	 GC0311_MIPI_SetMaxFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData32, *(pFeatureData32+1));

		 break;

	//case SENSOR_CMD_SET_VIDEO_FRAME_RATE:
//		SENSORDB("[GC0311] Enter SENSOR_CMD_SET_VIDEO_FRAME_RATE\n");
		//GC0311_MIPI_SetVideoFrameRate(*pFeatureData32);
		//break;

	case SENSOR_FEATURE_SET_TEST_PATTERN:			 
		GC0311SetTestPatternMode((BOOL)*pFeatureData16);			
		break;

    case SENSOR_FEATURE_GET_DELAY_INFO:
        SENSORDB("[GC0311] F_GET_DELAY_INFO\n");
    //    GC0311_MIPI_GetDelayInfo(*ppFeatureData);
	GC0311_MIPI_GetDelayInfo(*pFeatureData);
    break;

    case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
         SENSORDB("[GC0311] F_GET_DEFAULT_FRAME_RATE_BY_SCENARIO\n");
         GC0311_MIPI_GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData, (MUINT32 *)(uintptr_t)(*(pFeatureData+1)));
		 //S5K4ECGX_MIPI_GetDefaultFramerateByScenario((MSDK_SCENARIO_ID_ENUM)*pFeatureData, (MUINT32 *)(uintptr_t)(*(pFeatureData+1)));
    break;   
    
	case SENSOR_FEATURE_SET_YUV_3A_CMD:
		 SENSORDB("[GC0311] SENSOR_FEATURE_SET_YUV_3A_CMD ID:%d\n", *pFeatureData32);
		 GC0311_3ACtrl((ACDK_SENSOR_3A_LOCK_ENUM)*pFeatureData);
	//	 GC0311_3ACtrl((ACDK_SENSOR_3A_LOCK_ENUM)*pFeatureData32);
		 break;


	case SENSOR_FEATURE_GET_EXIF_INFO:
		 //SENSORDB("[4EC] F_GET_EXIF_INFO\n");
		 //GC0311_MIPI_GetExifInfo(*ppFeatureData);/////////////////fhjf
		 break;


	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
		 SENSORDB("[GC0311] F_GET_AE_AWB_LOCK_INFO\n");
		  GC0311_MIPI_get_AEAWB_lock((*pFeatureData),*(pFeatureData+1));
//		 GC0311_MIPI_get_AEAWB_lock((*pFeatureData32),*(pFeatureData32+1));
	break;    

	case SENSOR_FEATURE_SET_MIN_MAX_FPS:
		SENSORDB("SENSOR_FEATURE_SET_MIN_MAX_FPS:[%d,%d]\n",*pFeatureData32,*(pFeatureData32+1)); 
	//	GC0311_MIPI_SetMaxMinFps((UINT32)*pFeatureData32, (UINT32)*(pFeatureData32+1));
	break; 
	
    default:
        break;
	}
return ERROR_NONE;
}	/* GC0311FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC0311YUV=
{
	GC0311Open,
	GC0311GetInfo,
	GC0311GetResolution,
	GC0311FeatureControl,
	GC0311Control,
	GC0311Close
};


UINT32 GC0311_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGC0311YUV;
	return ERROR_NONE;
} /* SensorInit() */

