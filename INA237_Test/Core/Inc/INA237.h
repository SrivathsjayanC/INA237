/*
 * INA237.h
 *
 *  Created on: Sep 4, 2026
 *      Author: SRIVATHSJAYAN
 */

#ifndef INC_INA237_H_
#define INC_INA237_H_

#include "main.h"

typedef enum
{
	INA237_REG_CONFIG           = 0x00,
	INA237_REG_ADC_CONFIG       = 0x01,
	INA237_REG_SHUNT_CAL        = 0x02,
	/* 0x03 reserved */
	INA237_REG_SHUNT_VLTG       = 0x04,
	INA237_REG_BUS_VLTG         = 0x05,
	INA237_REG_DIETEMP          = 0x06,
	INA237_REG_CURRENT          = 0x07,
	INA237_REG_POWER            = 0x08,
	/* 0x09 - 0x0A reserved */
	INA237_REG_DIAG_ALRT        = 0x0B,
	INA237_REG_SOVL             = 0x0C,
	INA237_REG_SUVL             = 0x0D,
	INA237_REG_BOVL             = 0x0E,
	INA237_REG_BUVL             = 0x0F,
	INA237_REG_TEMP_LIMIT       = 0x10,
	INA237_REG_PWR_LIMIT        = 0x11,
	/* 0x12 - 0x3D reserved */
	INA237_REG_MANUFACTURER_ID  = 0x3E
} INA237_Register_t;
typedef struct
{
	uint8_t adc_range;
	uint8_t conv_dly;
	uint8_t avg;
	uint8_t vbusct;
	uint8_t vshct;
	uint8_t vtct;
	uint8_t op_mode;
	uint16_t dev_i2c_addr;

	float max_cur_exp_A;
	float shunt_res_Ohm;

	INA237_Register_t reg;

} INA237_InitTypeDef_t;
typedef struct
{
	I2C_HandleTypeDef *hi2c;

	GPIO_TypeDef *FAULT_Port;
	uint16_t FAULT_Pin;

	INA237_InitTypeDef_t Init;

	float _current_lsb;
	float _power_lsb;

} INA237_Handle_TypeDef_t;

typedef enum
{
	_INA237_DIAG_ALRT_ALATCH      = 0x0FU,
	_INA237_DIAG_ALRT_CNVR        = 0x0EU,
	_INA237_DIAG_ALRT_SLOWALERT   = 0x0DU,
	_INA237_DIAG_ALRT_APOL        = 0x0CU
} INA237_Diag_Alert_Config_t;

typedef enum
{
	INA237_DIAG_ALRT_ALATCH      = 0x0FU,
	INA237_DIAG_ALRT_CNVR        = 0x0EU,
	INA237_DIAG_ALRT_SLOWALERT   = 0x0DU,
	INA237_DIAG_ALRT_APOL        = 0x0CU,
	INA237_DIAG_ALRT_MATHOF      = 0x09U,
	INA237_DIAG_ALRT_TMPOL       = 0x07U,
	INA237_DIAG_ALRT_SHNTOL      = 0x06U,
	INA237_DIAG_ALRT_SHNTUL      = 0x05U,
	INA237_DIAG_ALRT_BUSOL       = 0x04U,
	INA237_DIAG_ALRT_BUSUL       = 0x03U,
	INA237_DIAG_ALRT_POL         = 0x02U,
	INA237_DIAG_ALRT_CNVRF       = 0x01U,
	INA237_DIAG_ALRT_MEMSTAT     = 0x00U
} INA237_Diag_Alert_t;


//@adc_range
#define INA237_ADCRANGE_163_84_MV 0x00U
#define INA237_ADCRANGE_40_96_MV 0x01U

#define __INA237_CONVDLY_MAX          0xFFU

// @avg
#define INA237_AVG_1                0x00U
#define INA237_AVG_4                0x01U
#define INA237_AVG_16               0x02U
#define INA237_AVG_64               0x03U
#define INA237_AVG_128              0x04U
#define INA237_AVG_256              0x05U
#define INA237_AVG_512              0x06U
#define INA237_AVG_1024             0x07U

// @vbusct
#define INA237_VBUSCT_50US          0x00U
#define INA237_VBUSCT_84US          0x01U
#define INA237_VBUSCT_150US         0x02U
#define INA237_VBUSCT_280US         0x03U
#define INA237_VBUSCT_540US         0x04U
#define INA237_VBUSCT_1052US        0x05U
#define INA237_VBUSCT_2074US        0x06U
#define INA237_VBUSCT_4120US        0x07U

// @vshct
#define INA237_VSHCT_50US           0x00U
#define INA237_VSHCT_84US           0x01U
#define INA237_VSHCT_150US          0x02U
#define INA237_VSHCT_280US          0x03U
#define INA237_VSHCT_540US          0x04U
#define INA237_VSHCT_1052US         0x05U
#define INA237_VSHCT_2074US         0x06U
#define INA237_VSHCT_4120US         0x07U

// @vtct
#define INA237_VTCT_50US           0x00U
#define INA237_VTCT_84US           0x01U
#define INA237_VTCT_150US          0x02U
#define INA237_VTCT_280US          0x03U
#define INA237_VTCT_540US          0x04U
#define INA237_VTCT_1052US         0x05U
#define INA237_VTCT_2074US         0x06U
#define INA237_VTCT_4120US         0x07U

//@op_mode
// @op_mode
#define INA237_OP_MODE_PWR_DWN_0          0x00U
#define INA237_OP_MODE_BUS_VT_TRG         0x01U
#define INA237_OP_MODE_SH_VT_TRG          0x02U
#define INA237_OP_MODE_SH_BUS_VT_TRG      0x03U
#define INA237_OP_MODE_TEMP_TRG           0x04U
#define INA237_OP_MODE_BUS_TEMP_TRG       0x05U
#define INA237_OP_MODE_SH_TEMP_TRG        0x06U
#define INA237_OP_MODE_SH_BUS_TEMP_TRG    0x07U
#define INA237_OP_MODE_PWR_DWN_8          0x08U
#define INA237_OP_MODE_BUS_VT_CONT        0x09U
#define INA237_OP_MODE_SH_VT_CONT         0x0AU
#define INA237_OP_MODE_SH_BUS_VT_CONT     0x0BU
#define INA237_OP_MODE_TEMP_CONT          0x0CU
#define INA237_OP_MODE_BUS_TEMP_CONT      0x0DU
#define INA237_OP_MODE_SH_TEMP_CONT       0x0EU
#define INA237_OP_MODE_SH_BUS_TEMP_CONT   0x0FU

#define __INA237_ADCRANGE_POS 0x05U
#define __INA237_CONVDLY_POS  0x06U
#define __INA237_RST_POS      0x0FU

#define __INA237_AVG_POS       0x00U
#define __INA237_VTCT_POS      0x03U
#define __INA237_VSHCT_POS     0x06U
#define __INA237_VBUSCT_POS    0x09U
#define __INA237_OP_MODE_POS   0x0CU

#define __INA237_TOL_POS       0x04U
#define __INA237_DIETEMP_POS   0x04U

#define __INA237_DIAG_ALRT_ALATCH_POS       0x0FU
#define __INA237_DIAG_ALRT_CNVR_POS         0x0EU
#define __INA237_DIAG_ALRT_SLOWALERT_POS    0x0DU
#define __INA237_DIAG_ALRT_APOL_POS         0x0CU
#define __INA237_DIAG_ALRT_MATHOF_POS       0x09U
#define __INA237_DIAG_ALRT_TMPOL_POS        0x07U
#define __INA237_DIAG_ALRT_SHNTOL_POS       0x06U
#define __INA237_DIAG_ALRT_SHNTUL_POS       0x05U
#define __INA237_DIAG_ALRT_BUSOL_POS        0x04U
#define __INA237_DIAG_ALRT_BUSUL_POS        0x03U
#define __INA237_DIAG_ALRT_POL_POS          0x02U
#define __INA237_DIAG_ALRT_CNVRF_POS        0x01U
#define __INA237_DIAG_ALRT_MEMSTAT_POS      0x00U

#define DISABLE 0x00U
#define ENABLE  0x01U

HAL_StatusTypeDef INA237_Init(INA237_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA237_Reset(INA237_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA237_ReadReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t reg,uint8_t *pData);
HAL_StatusTypeDef INA237_Get_Shunt_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA237_Get_Bus_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA237_Get_Temp_C(INA237_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA237_Set_Calib(INA237_Handle_TypeDef_t *hfault);
HAL_StatusTypeDef INA237_Get_Power_W(INA237_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA237_Get_Current_A(INA237_Handle_TypeDef_t *hfault,float *pData);
HAL_StatusTypeDef INA237_Set_Alert_Val(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Val);
HAL_StatusTypeDef INA237_Set_Diag_Alert_Config(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_Config_t _Ina237_Diag_Alrt,uint8_t En_Di);
uint8_t INA237_Get_Diag_Alert_Flag(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_t Ina237_Diag_Alrt);
#endif /* INC_INA237_H_ */
