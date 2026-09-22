/*
 * 	@file INA237.c
 *  @date 04 Sep 2026
 *  @author: SRIVATHSJAYAN
 */
#include "INA237.h"

static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data);
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t reg,uint32_t *pData);
static HAL_StatusTypeDef INA237_ReadReg_Signed(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,int16_t *pData);
/**
 * @brief  Resets the INA237 device.
 * @details Sets the reset bit in the INA237 CONFIG register, causing the
 *          device to reset and restore its registers to their default values.
 *
 * @param[in] hfault INA237 handle structure.
 *
 * @retval HAL_OK    Reset command transmitted successfully.
 * @retval HAL_ERROR I2C communication failure.
 */
HAL_StatusTypeDef INA237_Reset(INA237_Handle_TypeDef_t *hfault)
{
	uint16_t reset_bit = 0x01 << __INA237_RST_POS;

	return INA237_WriteReg(hfault, INA237_REG_CONFIG,reset_bit);
}

HAL_StatusTypeDef INA237_Init(INA237_Handle_TypeDef_t *hfault)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	uint16_t reg=0;
	HAL_StatusTypeDef status;
	INA237_Reset(hfault);
	HAL_Delay(5);
	if(hfault->Init.adc_range > INA237_ADCRANGE_40_96_MV)
	{
		return HAL_ERROR;
	}
	reg = hfault->Init.adc_range << __INA237_ADCRANGE_POS;

	if(hfault->Init.conv_dly > __INA237_CONVDLY_MAX)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.conv_dly << __INA237_CONVDLY_POS;
	status = INA237_WriteReg(hfault,INA237_REG_CONFIG,reg);

	if(status != HAL_OK)
	{
		return status;
	}

	reg = 0;

	if(hfault->Init.op_mode > INA237_OP_MODE_SH_BUS_TEMP_CONT)
	{
		return HAL_ERROR;
	}
	if(hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_0 || hfault->Init.op_mode == INA237_OP_MODE_PWR_DWN_8)
	{
		return HAL_ERROR;
	}
	reg = hfault->Init.op_mode << __INA237_OP_MODE_POS;

	if(hfault->Init.vbusct > INA237_VBUSCT_4120US)
	{
		return HAL_ERROR;
	}

	reg |= hfault->Init.vbusct << __INA237_VBUSCT_POS;

	if(hfault->Init.vshct > INA237_VSHCT_4120US)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vshct << __INA237_VSHCT_POS;

	if(hfault->Init.vtct > INA237_VTCT_4120US)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.vtct << __INA237_VTCT_POS;

	if(hfault->Init.avg > INA237_AVG_1024)
	{
		return HAL_ERROR;
	}
	reg |= hfault->Init.avg << __INA237_AVG_POS;

	return INA237_WriteReg(hfault,INA237_REG_ADC_CONFIG,reg);
}

static HAL_StatusTypeDef INA237_WriteReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Data)
{
	uint8_t tx[2];
	tx[0] = (uint8_t)(Data>>8);
	tx[1] = (uint8_t)Data;

	return HAL_I2C_Mem_Write(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, tx, 2, HAL_MAX_DELAY);
}

HAL_StatusTypeDef INA237_ReadReg(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t rx_buff[2];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx_buff, 2,HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = (((uint16_t)rx_buff[0]<<8) |(uint16_t)rx_buff[1]);

	return status;
}
static HAL_StatusTypeDef INA237_ReadReg_Signed(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,int16_t *pData)
{
	HAL_StatusTypeDef status;
	uint8_t rx[2];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx, 2,HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = (((int16_t)rx[0]<<8) | (int16_t)rx[1]);

	return status;
}
static HAL_StatusTypeDef INA237_ReadReg_24(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint32_t *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;
	uint8_t rx[3];

	status = HAL_I2C_Mem_Read(hfault->hi2c, hfault->Init.dev_i2c_addr, Reg, 1, rx, 3, HAL_MAX_DELAY);
	if(status!=HAL_OK)
	{
		return status;
	}
	*pData = ((uint32_t)rx[2]<<16|(uint32_t)rx[1]<<8|(uint32_t)rx[0]);
	return status;
}

HAL_StatusTypeDef INA237_Set_Calib(INA237_Handle_TypeDef_t *hfault)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	if ((hfault->Init.max_cur_exp_A <= 0.0f) || (hfault->Init.shunt_res_Ohm <= 0.0f))
	{
		return HAL_ERROR;
	}
	float cal_f;
	HAL_StatusTypeDef status;
	uint16_t reg;
	status = INA237_ReadReg(hfault,INA237_REG_CONFIG,&reg);
	if(status != HAL_OK)
	{
		return status;
	}
	if(reg & __INA237_ADCRANGE_BIT_MASK)
	{
		cal_f = (float)(__INA237_CALIB_DIVIDEND_ADC1 / ((hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR) * hfault->Init.shunt_res_Ohm));
	}
	else
	{
		cal_f = (float)(__INA237_CALIB_DIVIDEND_ADC0 / ((hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR) * hfault->Init.shunt_res_Ohm));
	}
	if ((cal_f < 1.0f) || (cal_f > 32767.0f))
	{
		return HAL_ERROR;
	}
	uint16_t cal = ((uint16_t)(cal_f + 0.5f)>>1);

	status = INA237_WriteReg(hfault,INA237_REG_SHUNT_CAL,cal);
	if(status != HAL_OK)
	{
		return status;
	}

	hfault->_current_lsb = hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR;
	hfault->_power_lsb = __INA237_POWER_LSB_MULTIPLIER * (hfault->Init.max_cur_exp_A / __INA237_CURRENT_LSB_DIVISOR);

	return status;
}

HAL_StatusTypeDef INA237_Get_Shunt_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint16_t reg;
	int16_t shnt_vlt_raw;
	HAL_StatusTypeDef status;
	status =  INA237_ReadReg(hfault,INA237_REG_CONFIG,&reg);
	if(status != HAL_OK)
	{
		return status;
	}
	status =  INA237_ReadReg_Signed(hfault,INA237_REG_SHUNT_VLTG,&shnt_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}
	if(reg & __INA237_ADCRANGE_BIT_MASK)
	{
		*pData = (float)(shnt_vlt_raw * __INA237_SHUNTVLTG_LSB_ADC_1);
	}
	else
	{
		*pData = (float)(shnt_vlt_raw * __INA237_SHUNTVLTG_LSB_ADC_0);
	}
	return status;
}

HAL_StatusTypeDef INA237_Get_Bus_Vltg_V(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t bus_vlt_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_BUS_VLTG,&bus_vlt_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(bus_vlt_raw * __INA237_BUSVLTG_LSB);

	return status;
}

HAL_StatusTypeDef INA237_Get_Temp_C(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t die_temp_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_DIETEMP,&die_temp_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)((die_temp_raw>>__INA237_DIETEMP_SHIFT) * __INA237_TEMP_LSB);

	return status;
}

HAL_StatusTypeDef INA237_Get_Current_A(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	int16_t cur_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_Signed(hfault,INA237_REG_CURRENT,&cur_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(cur_raw * hfault->_current_lsb);

	return status;
}

HAL_StatusTypeDef INA237_Get_Power_W(INA237_Handle_TypeDef_t *hfault,float *pData)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 || pData == NULL)
	{
		return HAL_ERROR;
	}
	uint32_t power_raw;
	HAL_StatusTypeDef status;
	status = INA237_ReadReg_24(hfault,INA237_REG_POWER,&power_raw);
	if(status != HAL_OK)
	{
		return status;
	}

	*pData = (float)(power_raw * hfault->_power_lsb);

	return status;
}

HAL_StatusTypeDef INA237_Set_Alert_Val(INA237_Handle_TypeDef_t *hfault,INA237_Register_t Reg,uint16_t Val)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0)
	{
		return HAL_ERROR;
	}
	switch(Reg)
	{
	case INA237_REG_SOVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_SOVL, Val);
		break;
	case INA237_REG_SUVL:
		if(Val > 0x8000U)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_SUVL, Val);
		break;
	case INA237_REG_BOVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_BOVL, Val);
		break;
	case INA237_REG_BUVL:
		if(Val > 0x7FFFU)
		{
			return HAL_ERROR;
		}
		return INA237_WriteReg(hfault, INA237_REG_BUVL, Val);
		break;
	case INA237_REG_TEMP_LIMIT:
		if(Val > 0x7FFU)
		{
			return HAL_ERROR;
		}
		uint16_t tx = Val << __INA237_TOL_POS;
		return INA237_WriteReg(hfault, INA237_REG_TEMP_LIMIT, tx);
		break;
	case INA237_REG_PWR_LIMIT:
		return INA237_WriteReg(hfault, INA237_REG_PWR_LIMIT, Val);
		break;
	default:
		return HAL_ERROR; // Wrong Register was used to set Limit Value
		/* Use Only the Limit Setting Registers*/
	}
}

HAL_StatusTypeDef INA237_Set_Diag_Alert_Config(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_Config_t _Ina237_Diag_Alrt,uint8_t En_Di)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			_Ina237_Diag_Alrt > _INA237_DIAG_ALRT_ALATCH || _Ina237_Diag_Alrt < _INA237_DIAG_ALRT_APOL)
	{
		return HAL_ERROR;
	}
	uint16_t tx;
	HAL_StatusTypeDef status;
	if(En_Di > ENABLE)
	{
		return HAL_ERROR;
	}
	status = INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT,&tx);
	if(status != HAL_OK)
	{
		return status;
	}
	if(En_Di == DISABLE)
	{
		tx &= ~(1U << _Ina237_Diag_Alrt);
		status = INA237_WriteReg(hfault, INA237_REG_DIAG_ALRT, tx);
		if(status != HAL_OK)
		{
			return status;
		}
	}
	else
	{
		tx |= (1U << _Ina237_Diag_Alrt);
		status = INA237_WriteReg(hfault, INA237_REG_DIAG_ALRT, tx);
		if(status != HAL_OK)
		{
			return status;
		}
	}
	return status;
}

uint8_t INA237_Get_Diag_Alert_Flag(INA237_Handle_TypeDef_t *hfault,INA237_Diag_Alert_t Ina237_Diag_Alrt)
{
	if(hfault == NULL ||hfault->hi2c == NULL || hfault->Init.dev_i2c_addr == 0 ||
			Ina237_Diag_Alrt > INA237_DIAG_ALRT_ALATCH)
	{
		return HAL_ERROR;
	}
	uint16_t diag_alert;
	if(INA237_ReadReg(hfault, INA237_REG_DIAG_ALRT,&diag_alert) != HAL_OK)
	{
		return 0x99U;
	}
	return ((diag_alert & (1U << Ina237_Diag_Alrt)));
}
